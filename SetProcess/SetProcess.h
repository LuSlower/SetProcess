#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <tlhelp32.h>
#include <processthreadsapi.h>

#define PROCESS_AFFINITY_ENABLE_AUTO_UPDATE __MSABI_LONG(0x1U)

extern "C" {

    typedef enum _PROC_INF_CLASS {
        ProcessIoPriority = 0x21
    } PROC_INF_CLASS;

    NTSYSAPI LONG NTAPI NtSetInformationProcess(
        HANDLE ProcessHandle,
        PROC_INF_CLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength
    );

    NTSYSAPI LONG NTAPI NtSuspendProcess(
        HANDLE ProcessHandle
    );

    NTSYSAPI LONG NTAPI NtResumeProcess(
        HANDLE ProcessHandle
    );

    typedef struct _IO_PRIORITY_INFORMATION {
        ULONG IoPriority;
    } IO_PRIORITY_INFORMATION, *PIO_PRIORITY_INFORMATION;

    typedef struct _CLIENT_ID {
         PVOID UniqueProcess;
         PVOID UniqueThread;
    } CLIENT_ID, *PCLIENT_ID;

    WINBASEAPI WINBOOL WINAPI SetProcessAffinityUpdateMode(
        HANDLE hProcess,
        DWORD dwFlags
    );

    WINBASEAPI WINBOOL WINAPI SetProcessDefaultCpuSets(
        HANDLE Process,
        const ULONG* CpuSetIds,
        ULONG CpuSetIdCount
    );

    WINBASEAPI WINBOOL WINAPI SetProcessInformation(
        HANDLE hProcess,
        PROCESS_INFORMATION_CLASS ProcessInformationClass,
        LPVOID ProcessInformation,
        DWORD ProcessInformationSize
    );

}

bool EnablePrivilege(DWORD processId, LPCSTR privilegeName, HANDLE hProcess = NULL) {

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!LookupPrivilegeValue(NULL, privilegeName, &tp.Privileges[0].Luid)) {
        printf("Error finding privilege value.\n");
        return false;
    }

    if (!hProcess) {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
        // comprobar por ultima vez
        if (!hProcess){
        printf("Error opening process token.\n");
        return false;
        }
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        CloseHandle(hProcess);
        return false;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("Error adjusting token privileges.\n");
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hToken);
    CloseHandle(hProcess);
    return true;
}

DWORD GetChildProcesses(DWORD ParentPID, DWORD* ChildPIDs) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printf("Error creating a process snapshot.\n");
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    DWORD NumProcesses = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ParentProcessID == ParentPID) {
                if (NumProcesses < 64) {
                    ChildPIDs[NumProcesses++] = pe32.th32ProcessID;
                } else {
                    printf("The maximum limit of child processes has been reached.\n");
                    break;
                }
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return NumProcesses;
}


DWORD GetPID(const char* processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("Error creating a process snapshot.\n");
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        printf("Error getting first process entry.\n");
        return 0;
    }

    DWORD processId = 0;
    do {
        if (strcmp(entry.szExeFile, processName) == 0) {
            processId = entry.th32ProcessID;
            break;
        }
    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return processId;
}

// SID de Local System
static SID LocalSystem = { SID_REVISION, 1, SECURITY_NT_AUTHORITY, { SECURITY_LOCAL_SYSTEM_RID } };

BOOL ImpersonateSystem() {
    // Crear un snapshot de todos los hilos en el sistema
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        printf("Error creating snapshot of threads.\n");
        return FALSE;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // Recuperar información del primer hilo
    if (!Thread32First(hThreadSnap, &te32)) {
        printf("Error retrieving information about the first thread.\n");
        CloseHandle(hThreadSnap);
        return FALSE;
    }

    BOOL impersonated = FALSE;

    // Iterar sobre todos los hilos para encontrar uno que pertenezca a un proceso del sistema
    do {
        // Verificar si el hilo pertenece a un proceso del sistema
        HANDLE hThread = OpenThread(THREAD_DIRECT_IMPERSONATION, FALSE, te32.th32ThreadID);
        if (hThread) {
            // Impersonar el hilo
            if (SetThreadToken(NULL, hThread)) {
                // Obtener el token del hilo
                HANDLE hToken;
                if (OpenThreadToken(hThread, TOKEN_QUERY, FALSE, &hToken)) {
                    // Verificar si el token pertenece al sistema
                    TOKEN_USER tokenUser;
                    DWORD cb;
                    if (GetTokenInformation(hToken, TokenUser, &tokenUser, sizeof(TOKEN_USER), &cb)) {
                        if (EqualSid(tokenUser.User.Sid, &LocalSystem)) {
                            CloseHandle(hToken);
                            impersonated = TRUE;
                            break;
                        }
                    }
                    CloseHandle(hToken);
                }
                RevertToSelf(); // Revertir la suplantación antes de cerrar el hilo
            }
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);

    return impersonated;
}

DWORD ParseArg(char* argumento) {
    // Eliminar la coma del argumento
    char* p = argumento;
    char* q = argumento;
    while (*q) {
        if (*q != ',') {
            *p++ = *q;
        }
        q++;
    }
    *p = '\0';

    // Convertir la cadena resultante en un número entero
    DWORD numero = atoi(argumento);

    return numero;
}

DWORD CountSetBits(DWORD mask) {
    DWORD counts = 0;
    while (mask) {
        counts += mask & 1;
        mask >>= 1;
    }
    return counts;
}

DWORD ConvertToBitMask(const char* Str, DWORD counts = NULL) {
    DWORD Mask = 0;
    char* endptr;
    char* ptr = (char*)Str;

    while (*ptr != '\0') {
        int Num = strtol(ptr, &endptr, 10);
        if (ptr == endptr) {
            break; // No se pudo convertir a número
        }
        Mask |= (1 << Num);
        ptr = endptr;
        if (*ptr == ',') {
            ++ptr; // Saltar la coma
        }
    }

    if (counts){
        CountSetBits(Mask);
    }
    return Mask;
}

BOOL SetProcessDefaultCpuSetsID(DWORD ProcessID, const ULONG* CpuSetIds, ULONG CpuSetIdCount) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_LIMITED_INFORMATION, FALSE, ProcessID);
    if (hProcess == NULL) {
        printf("Error opening process");
        return FALSE;
    }

    BOOL success = SetProcessDefaultCpuSets(hProcess, CpuSetIds, CpuSetIdCount);
    if (!success) {
        printf("Error setting cpu sets");
    }

    CloseHandle(hProcess);
    return success;
}

void SetIdealProcessor(DWORD dwProcessId, DWORD dwIdealProcessor)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        if (Thread32First(hSnapshot, &te32))
        {
            do
            {
                if (te32.th32OwnerProcessID == dwProcessId)
                {
                    HANDLE hThread = OpenThread(THREAD_SET_INFORMATION, FALSE, te32.th32ThreadID);
                    if (hThread != NULL)
                    {
                        SetThreadIdealProcessor(hThread, dwIdealProcessor);
                        CloseHandle(hThread);
                    }
                }
            } while (Thread32Next(hSnapshot, &te32));
        }

        CloseHandle(hSnapshot);
    }
}
