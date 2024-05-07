#pragma once
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <tlhelp32.h>

#define PROCESS_AFFINITY_ENABLE_AUTO_UPDATE __MSABI_LONG(0x1U)

extern "C" {

  typedef enum _PROC_INF_CLASS {
    ProcessIoPriority = 0x21
  } PROC_INF_CLASS;

NTSYSAPI LONG NTAPI NtSetInformationProcess(HANDLE ProcessHandle, PROC_INF_CLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLenght);

NTSYSAPI LONG NTAPI NtSuspendProcess(HANDLE ProcessHandle);

NTSYSAPI LONG NTAPI NtResumeProcess(HANDLE ProcessHandle);

typedef struct _MEMORY_PRIORITY_INFORMATION {
    ULONG MemoryPriority;
  } MEMORY_PRIORITY_INFORMATION, *PMEMORY_PRIORITY_INFORMATION;

typedef struct _IO_PRIORITY_INFORMATION {
    ULONG IoPriority;
  } IO_PRIORITY_INFORMATION, *PIO_PRIORITY_INFORMATION;

WINBASEAPI WINBOOL WINAPI SetProcessAffinityUpdateMode(HANDLE hProcess, DWORD dwFlags);

WINBASEAPI WINBOOL WINAPI SetProcessDefaultCpuSets(HANDLE Process, const ULONG* CpuSetIds, ULONG CpuSetIdCount);

WINBASEAPI WINBOOL WINAPI SetProcessInformation(HANDLE hProcess, PROCESS_INFORMATION_CLASS ProcessInformationClass, LPVOID ProcessInformation, DWORD ProcessInformationSize);
}

bool EnablePrivilege(DWORD processId, LPCSTR privilegeName, HANDLE hProcess = NULL) {

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!LookupPrivilegeValue(NULL, privilegeName, &tp.Privileges[0].Luid)) {
        printf("Error al buscar el valor del privilegio ");
        return false;
    }

    if (!hProcess) {
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
        // comprobar por ultima vez
        if (!hProcess){
        printf("Error al abrir el token del proceso");
        return false;
        }
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken)) {
        printf("Error al abrir el token del proceso");
        CloseHandle(hProcess);
        return false;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("Error al ajustar los privilegios del token");
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
        printf("Error al crear un snapshot de procesos\n");
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
                    printf("Se alcanzó el límite máximo de procesos hijos\n");
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
        printf("Error al crear un snapshot de procesos");
        return 0;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(snapshot, &entry)) {
        CloseHandle(snapshot);
        printf("Error al obtener la primera entrada de proceso");
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

bool ImpersonateSecurity() {
    // Obtener el handle del proceso SYSTEM
    DWORD SecurityPID = GetPID("lsass.exe");

    HANDLE hSecurityProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, SecurityPID);
    if (!hSecurityProcess) {
        printf("Error al obtener la impersonacion");
        return false;
    }

    // Obtener el handle del token del proceso SYSTEM
    HANDLE hSecurityToken;
    if (!OpenProcessToken(hSecurityProcess, TOKEN_DUPLICATE, &hSecurityToken)) {
        printf("Error al obtener la impersonacion");
        CloseHandle(hSecurityProcess);
        return false;
    }

    // Duplicar el token para la impersonación
    HANDLE hDuplicateToken;
    if (!DuplicateTokenEx(hSecurityToken, TOKEN_ALL_ACCESS, nullptr, SecurityImpersonation, TokenPrimary, &hDuplicateToken)) {
        printf("Error al obtener la impersonacion");
        CloseHandle(hSecurityToken);
        CloseHandle(hSecurityProcess);
        return false;
    }

    // Iniciar la impersonación
    if (!ImpersonateLoggedOnUser(hDuplicateToken)) {
        printf("Error al impersonar al usuario");
        CloseHandle(hDuplicateToken);
        CloseHandle(hSecurityToken);
        CloseHandle(hSecurityProcess);
        return false;
    }

    // Cerrar los handles que ya no necesitamos
    CloseHandle(hDuplicateToken);
    CloseHandle(hSecurityToken);
    CloseHandle(hSecurityProcess);

    return true;
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
        printf("Error al abrir el proceso");
        return FALSE;
    }

    BOOL success = SetProcessDefaultCpuSets(hProcess, CpuSetIds, CpuSetIdCount);
    if (!success) {
        printf("Error al establecer los conjuntos de CPU");
    }

    CloseHandle(hProcess);
    return success;
}
