#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <tlhelp32.h>
#include <winternl.h>
#include <ntstatus.h>
#include <processthreadsapi.h>

extern "C" {

    NTSYSAPI NTSTATUS NTAPI NtSuspendProcess(
        HANDLE ProcessHandle
    );

    NTSYSAPI NTSTATUS NTAPI NtResumeProcess(
        HANDLE ProcessHandle
    );

    typedef struct _IO_PRIORITY_INFORMATION {
        ULONG IoPriority;
    } IO_PRIORITY_INFORMATION, *PIO_PRIORITY_INFORMATION;

    WINBASEAPI WINBOOL WINAPI SetProcessDefaultCpuSetMasks(
      HANDLE          Process,
      PGROUP_AFFINITY CpuSetMasks,
      USHORT          CpuSetMaskCount
    );
}

bool EnablePrivilege(DWORD processId, LPCSTR privilegeName, HANDLE hProcess = NULL) {

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!LookupPrivilegeValue(NULL, privilegeName, &tp.Privileges[0].Luid)) {
        return false;
    }

    if (!hProcess) {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
        // comprobar por ultima vez
        if (!hProcess){
        return false;
        }
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        CloseHandle(hProcess);
        return false;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
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
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
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
        return FALSE;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    // Recuperar informaci�n del primer hilo
    if (!Thread32First(hThreadSnap, &te32)) {
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
                RevertToSelf(); // Revertir la suplantaci�n antes de cerrar el hilo
            }
            CloseHandle(hThread);
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);

    return impersonated;
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
            break; // No se pudo convertir a n�mero
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

WORD ConvertToBitMaskHex(const char* Str) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    DWORD MAX_CPUS = sysinfo.dwNumberOfProcessors;

    DWORD Mask = 0;
    const char* ptr = Str;

    while (*ptr != '\0') {
        char* endptr;
        int Num = strtol(ptr, &endptr, 10);
        if (ptr == endptr) {
            break; // No se pudo convertir a n�mero
        }
        if (Num >= MAX_CPUS) {
            MessageBox(0, "cpu number out of range", "Error", MB_OK | MB_ICONERROR);
            return 0;
        }
        Mask |= (1 << Num);
        ptr = endptr;
        if (*ptr == ',') {
            ++ptr; // Saltar la coma
        }
    }

    return Mask;
}

BOOL SetProcessCpuSetMask(DWORD dwProcessId, DWORD cpuBitmask) {
    // Abrir el proceso con permisos para establecer informaci�n limitada
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return FALSE;
    }

    // Si se especifica un bitmask de 0, se debe revertir los CpuSets
    if (cpuBitmask == 0) {
        BOOL success = SetProcessDefaultCpuSetMasks(hProcess, NULL, 0);
        if (!success) {
            MessageBox(0, "Error setting cpu sets", "Error", MB_OK | MB_ICONERROR);
        }
        CloseHandle(hProcess);
        return success;
    }

    // Obtener el n�mero m�ximo de CPUs disponibles en el sistema
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    DWORD MAX_CPUS = sysinfo.dwNumberOfProcessors;

    GROUP_AFFINITY cpuSetMasks[1];
    cpuSetMasks[0].Mask = cpuBitmask;
    cpuSetMasks[0].Group = 0;
    int actualCpuSetMaskCount = 1;

    // Llamar a la funci�n SetProcessDefaultCpuSetMasks para establecer las m�scaras de CPU
    BOOL success = SetProcessDefaultCpuSetMasks(hProcess, cpuSetMasks, actualCpuSetMaskCount);
    if (!success) {
        MessageBox(0, "Error setting cpu sets", "Error", MB_OK | MB_ICONERROR);
    }

    // Cerrar el handle del proceso
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
