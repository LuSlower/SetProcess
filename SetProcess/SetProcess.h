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

    WINBASEAPI WINBOOL WINAPI SetProcessDefaultCpuSets (
        HANDLE Process,
        const ULONG *CpuSetIds,
        ULONG CpuSetIdCount
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

    // Recuperar información del primer hilo
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
                RevertToSelf(); // Revertir la suplantación antes de cerrar el hilo
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

ULONG* ConvertToCpuSetIds(const char* str, DWORD* Count) {
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    DWORD MAX_CPUS = sysinfo.dwNumberOfProcessors;

    ULONG* cpus = (ULONG*)malloc(MAX_CPUS * sizeof(ULONG));
    if (!cpus) {
        MessageBox(0, "Error allocating memory for CPU sets", "Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    DWORD numCpus = 0;
    const char* ptr = str;

    while (*ptr != '\0') {
        char* endptr;
        int num = strtol(ptr, &endptr, 10);
        if (ptr == endptr) {
            break; // No se pudo convertir a número
        }
        if (num >= MAX_CPUS) {
            char errorMsg[64];
            snprintf(errorMsg, sizeof(errorMsg), "Número de CPU fuera de rango: %d", num);
            MessageBox(NULL, errorMsg, "Error", MB_OK | MB_ICONERROR);
            free(cpus);
            return NULL; // Salir si el número es mayor que el máximo de CPUs
        }
        // Ajustar num al ID de conjunto de CPU específico
        cpus[numCpus++] = 256 + num; // Ejemplo: 256 es el primer ID de conjunto de CPU

        ptr = endptr;
        // No saltar la coma, solo avanzar si es una coma
        if (*ptr == ',') {
            ptr++;
        }
    }

    *Count = numCpus;
    return cpus;
}

BOOL SetProcessCpuSetID(DWORD dwProcessId, ULONG* Ids, DWORD Count) {
    // Abrir el proceso con permisos para establecer información limitada
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        MessageBox(0, "Error opening process", "Error", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // Si se especifica NULL, se debe revertir los CpuSets
    if (Ids == NULL) {
        if (!SetProcessDefaultCpuSets(hProcess, NULL, 0)) {
            MessageBox(0, "Error setting default CPU sets", "Error", MB_OK | MB_ICONERROR);
            CloseHandle(hProcess);
            return FALSE;
        }
        CloseHandle(hProcess);
        return TRUE;
    }

    // Llamar a la función SetProcessDefaultCpuSets para establecer las máscaras de CPU
    if (!SetProcessDefaultCpuSets(hProcess, Ids, Count)) {
        MessageBox(0, "Error setting CPU sets", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Cerrar el handle del proceso
    CloseHandle(hProcess);
    return TRUE;
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
