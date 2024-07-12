#pragma once
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <processthreadsapi.h>
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>
#define IDC_STATIC (-1)
#define DLG_MAIN 1
#define IDC_REFRESH 2
#define IDC_LISTP 3
#define _START 4
#define IDI_ICON 5
#define IDM_EXIT 6
#define WM_HIDE 7
#define WM_ICON 8
#define WM_TRAYICON (WM_ICON)
#define IDR_CONTEXT_MENU 9
#define IDM_CPUSETS                  10
#define IDM_IDEAL_PROCESSOR          11
#define IDM_AFFINITY                 12
#define IDM_CPU_PRIORITY_IDLE        14
#define IDM_CPU_PRIORITY_BELOW_NORMAL 15
#define IDM_CPU_PRIORITY_NORMAL      16
#define IDM_CPU_PRIORITY_ABOVE_NORMAL 17
#define IDM_CPU_PRIORITY_HIGH        18
#define IDM_CPU_PRIORITY_REALTIME    19
#define IDM_CPU_PRIORITY_BOOST       20
#define IDM_MEMORY_PRIORITY_VERYLOW  21
#define IDM_MEMORY_PRIORITY_LOW      22
#define IDM_MEMORY_PRIORITY_MEDIUM   23
#define IDM_MEMORY_PRIORITY_BELOWNORMAL 24
#define IDM_MEMORY_PRIORITY_NORMAL   25
#define IDM_IO_PRIORITY_VERYLOW      26
#define IDM_IO_PRIORITY_LOW          27
#define IDM_IO_PRIORITY_NORMAL       28
#define IDM_IO_PRIORITY_HIGH         29
#define IDM_QOS_ECOQOS               31
#define IDM_QOS_HIGHQOS              32
#define IDM_SUSPEND_RESUME           33
#define IDC_CPU_0                    35
#define IDC_CPU_1                    36
#define IDC_CPU_2                    37
#define IDC_CPU_3                    38
#define IDC_CPU_4                    39
#define IDC_CPU_5                    40
#define IDC_CPU_6                    41
#define IDC_CPU_7                    42
#define IDC_CPU_8                    43
#define IDC_CPU_9                    44
#define IDC_CPU_10                   45
#define IDC_CPU_11                   46
#define IDC_CPU_12                   47
#define IDC_CPU_13                   48
#define IDC_CPU_14                   49
#define IDC_CPU_15                   50
#define IDC_CPU_16                   51
#define IDC_CPU_17                   52
#define IDC_CPU_18                   53
#define IDC_CPU_19                   54
#define IDC_CPU_20                   55
#define IDC_CPU_21                   56
#define IDC_CPU_22                   57
#define IDC_CPU_23                   58
#define IDC_CPU_24                   59
#define IDC_CPU_25                   60
#define IDC_CPU_26                   61
#define IDC_CPU_27                   62
#define IDC_CPU_28                   63
#define IDC_CPU_29                   64
#define IDC_CPU_30                   65
#define IDC_CPU_31                   66
#define IDC_CPU_32                   67
#define IDC_CPU_33                   68
#define IDC_CPU_34                   69
#define IDC_CPU_35                   70
#define IDC_CPU_36                   71
#define IDC_CPU_37                   72
#define IDC_CPU_38                   73
#define IDC_CPU_39                   74
#define IDC_CPU_40                   75
#define IDC_CPU_41                   76
#define IDC_CPU_42                   77
#define IDC_CPU_43                   78
#define IDC_CPU_44                   79
#define IDC_CPU_45                   80
#define IDC_CPU_46                   81
#define IDC_CPU_47                   82
#define IDC_CPU_48                   83
#define IDC_CPU_49                   84
#define IDC_CPU_50                   85
#define IDC_CPU_51                   86
#define IDC_CPU_52                   87
#define IDC_CPU_53                   88
#define IDC_CPU_54                   89
#define IDC_CPU_55                   90
#define IDC_CPU_56                   91
#define IDC_CPU_57                   92
#define IDC_CPU_58                   93
#define IDC_CPU_59                   94
#define IDC_CPU_60                   95
#define IDC_CPU_61                   96
#define IDC_CPU_62                   97
#define IDC_CPU_63                   98
#define IDC_SAVE_BITMASK             99
#define DLG_BITMASK                  100
#define IDM_RESET                    101
#define IDM_CPU_PRIORITY_SAVE        102
#define IDM_MEMORY_PRIORITY_SAVE     103
#define IDM_IO_PRIORITY_SAVE         104
#define IDM_CPU_PRIORITY_BOOST_SAVE  105
#define IDM_QOS_SAVE                 106
#define IDC_CHECK_ALL                107
#define IDC_UNCHECK                  108
HINSTANCE hInst;
HWND hwndDlg;
HWND hwndToolTip;

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

    WINBASEAPI WINBOOL WINAPI GetProcessDefaultCpuSetMasks(
      HANDLE          Process,
      PGROUP_AFFINITY CpuSetMasks,
      USHORT          CpuSetMaskCount,
      PUSHORT         RequiredMaskCount
    );

    typedef enum _PROCINFOCLAS {
        ProcPowerThrottlingState = 77
    } PROCINFOCLASS;

    NTSYSAPI NTSTATUS NTAPI ZwQueryInformationProcess(
        HANDLE ProcessHandle,
        PROCINFOCLASS ProcessInformationClass,
        PVOID ProcessInformation,
        ULONG ProcessInformationLength,
        PULONG ReturnLength
    );

    NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation (
        SYSTEM_INFORMATION_CLASS SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        ULONG *ReturnLength
    );
}

void _drain()
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_SET_QUOTA, false, GetCurrentProcessId());
    //drenar WorkingSet
    SetProcessWorkingSetSize(hProcess, (SIZE_T) -1, (SIZE_T) -1);
}

//Leer REG_SZ del registro
char* RegKeyQuery(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName)
{
    static char result[MAX_PATH]; // Static para que persista fuera del ámbito de la función
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, lpSubKey, 0, KEY_QUERY_VALUE, &hSubKey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD dwSize = MAX_PATH;
        if (RegQueryValueExA(hSubKey, lpValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(result), &dwSize) == ERROR_SUCCESS && dwType == REG_SZ)
        {
            RegCloseKey(hSubKey);
            return result;
        }
        RegCloseKey(hSubKey);
    }
    return nullptr; // Devolver nullptr si no se pudo leer el valor
}

bool RegKeyExists(HKEY hKeyRoot, const char* subKey) {
    HKEY hKey;
    if (RegOpenKeyEx(hKeyRoot, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

//Escribir REG_SZ en el registro
bool RegKeySet(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName, LPCSTR lpData)
{
    HKEY hSubKey;
    if (RegCreateKeyExA(hKey, lpSubKey, 0, NULL, 0, KEY_SET_VALUE, NULL, &hSubKey, NULL) == ERROR_SUCCESS)
    {
        DWORD dwSize = static_cast<DWORD>(strlen(lpData) + 1);
        if (RegSetValueExA(hSubKey, lpValueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(lpData), dwSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return true;
        }
        RegCloseKey(hSubKey);
    }
    return false;
}

//Eliminar REG_SZ del registro
bool RegKeyDelete(HKEY hKey, LPCSTR lpSubKey, LPCSTR lpValueName = NULL)
{
    HKEY hSubKey;
    if (RegOpenKeyExA(hKey, lpSubKey, 0, KEY_SET_VALUE, &hSubKey) == ERROR_SUCCESS)
    {
        if (RegDeleteValueA(hSubKey, lpValueName) == ERROR_SUCCESS)
        {
            RegCloseKey(hSubKey);
            return true;
        } else if (lpValueName == NULL) {
            RegDeleteKey(hKey, lpSubKey);
            RegCloseKey(hSubKey);
            return true;
        }
        RegCloseKey(hSubKey);
    }
    return false;
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
        MessageBox(0, "Error creating a process snapshot", "Error", MB_OK | MB_ICONERROR);
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

DWORD ExtractPID(const char* str) {
    DWORD pid = 0;
    char buffer[256]; // Buffer para manipular la cadena
    snprintf(buffer, sizeof(buffer), "%s", str); // Copiar la cadena original a buffer

    char* start = strstr(buffer, "(PID:"); // Buscar la cadena "(PID: "
    if (start) {
        start += strlen("(PID:"); // Moverse al inicio del número de PID
        pid = strtoul(start, NULL, 10); // Convertir el número a DWORD
    }
    return pid;

}

void ExtractProcessName(const char* str, char* processName, size_t bufferSize) {
    if (!str || !processName || bufferSize == 0) {
        return; // Manejo básico de errores de parámetros nulos o tamaño de buffer cero
    }

    const char* start = str; // El inicio del nombre del proceso
    const char* end = strstr(str, " (PID:"); // Buscar el inicio del PID

    if (end && end > start) {
        size_t length = end - start;
        if (length < bufferSize - 1) {
            strncpy(processName, start, length);
            processName[length] = '\0'; // Asegurar que la cadena termina con '\0'
        } else {
            strncpy(processName, start, bufferSize - 1);
            processName[bufferSize - 1] = '\0'; // Asegurar que la cadena termina con '\0'
        }
    } else {
        // No se encontró el nombre del proceso en el formato esperado
        processName[0] = '\0'; // Cadena vacía
    }
}

void CreateToolTip(HWND hwndParent, HWND hwndControl, LPSTR text)
{
    // Inicializar el tooltip
    hwndToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, NULL, NULL);

    // Configurar el estilo para un tooltip con fondo oscuro
    SendMessage(hwndToolTip, TTM_SETTIPBKCOLOR, RGB(30, 30, 30), 0);
    SendMessage(hwndToolTip, TTM_SETTIPTEXTCOLOR, RGB(255, 255, 255), 0);
    SendMessage(hwndToolTip, TTM_SETMAXTIPWIDTH, 0, 300); // Ancho máximo del tooltip

    // Asociar el tooltip con el control
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(TOOLINFO);
    toolInfo.hwnd = hwndParent;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndControl;
    toolInfo.lpszText = (LPSTR)text;

    // Agregar la herramienta al tooltip
    SendMessage(hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void UpdateToolTipText(HWND hwndControl, LPSTR text)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(TOOLINFO);
    toolInfo.hwnd = GetParent(hwndControl); // Obtener el HWND del control padre
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndControl;
    toolInfo.lpszText = text;  // Cambiado a text (LPSTR)

    SendMessage(hwndToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&toolInfo);
}

BOOL GetProcessCpuSetMask(DWORD dwProcessId, PGROUP_AFFINITY cpuSetMasks, USHORT CpuSetMaskCount, PUSHORT RequiredMaskCount)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return FALSE;
    }

    BOOL success = GetProcessDefaultCpuSetMasks(hProcess, cpuSetMasks, CpuSetMaskCount, RequiredMaskCount);
    if (!success)
    {
        MessageBox(0, "Error getting CPU set masks", "Error", MB_OK | MB_ICONERROR);
    }

    CloseHandle(hProcess);
    return success;
}

BOOL SetProcessCpuSetMask(DWORD dwProcessId, DWORD cpuBitmask) {
    // Abrir el proceso con permisos para establecer información limitada
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

    // Obtener el número máximo de CPUs disponibles en el sistema
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    DWORD MAX_CPUS = sysinfo.dwNumberOfProcessors;

    GROUP_AFFINITY cpuSetMasks[1];
    cpuSetMasks[0].Mask = cpuBitmask;
    cpuSetMasks[0].Group = 0;
    int actualCpuSetMaskCount = 1;

    // Llamar a la función SetProcessDefaultCpuSetMasks para establecer las máscaras de CPU
    BOOL success = SetProcessDefaultCpuSetMasks(hProcess, cpuSetMasks, actualCpuSetMaskCount);
    if (!success) {
        MessageBox(0, "Error setting cpu sets", "Error", MB_OK | MB_ICONERROR);
    }

    // Cerrar el handle del proceso
    CloseHandle(hProcess);
    return success;
}

void SetProcessAffinity(DWORD dwProcessId, DWORD_PTR dwProcessAffinityMask) {
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return;
    }

    // Aplicar la máscara de afinidad de procesador al proceso
    if (!SetProcessAffinityMask(hProcess, dwProcessAffinityMask)) {
        MessageBox(0, "Error setting processor affinity mask", "Error", MB_OK | MB_ICONERROR);
    }
    CloseHandle(hProcess);
}

DWORD_PTR GetProcessAffinity(DWORD dwProcessId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return 0;
    }

    DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;

    // Obtener la máscara de afinidad de procesador del sistema
    if (!GetProcessAffinityMask(hProcess, &dwProcessAffinityMask, &dwSystemAffinityMask)) {
        MessageBox(0, "Error getting process affinity mask", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hProcess);
        return 0;
    }

    CloseHandle(hProcess);
    return dwProcessAffinityMask;
}

int GetSingleCoreIndex(DWORD* CPUs, DWORD MAX_CPUS)
{
    int activeIndex = -1; // Inicializar con valor inválido

    for (DWORD i = 0; i < MAX_CPUS; ++i)
    {
        if (CPUs[i] == 1)
        {
            // Si ya encontramos un bit activo previamente, no hay exactamente uno
            if (activeIndex != -1)
                return -1;

            activeIndex = i;
        }
    }

    return activeIndex; // Retornar el índice del bit activo o -1 si no se encontró exactamente uno
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
                    HANDLE hThread = OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, te32.th32ThreadID);
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

void SetProcessPriority(DWORD dwProcessId, DWORD dwPriorityClass)
{

    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
    return;
    }

    switch (dwPriorityClass)
    {
        case 0:
            {
                dwPriorityClass = IDLE_PRIORITY_CLASS;
            }
            break;
        case 1:
            {
                dwPriorityClass = BELOW_NORMAL_PRIORITY_CLASS;
            }
            break;
        case 2:
            {
                dwPriorityClass = NORMAL_PRIORITY_CLASS;
            }
            break;
        case 3:
             {
                dwPriorityClass = ABOVE_NORMAL_PRIORITY_CLASS;
             }
            break;
        case 4:
            {
                dwPriorityClass = HIGH_PRIORITY_CLASS;
            }
            break;
        case 5:
            {
                dwPriorityClass = REALTIME_PRIORITY_CLASS;
            }
            break;
    }

    // Establecer prioridad
    if (SetPriorityClass(hProcess, dwPriorityClass) == 0){
        MessageBox(0, "Error setting cpu priority", "Error", MB_OK | MB_ICONERROR);
    }

}

DWORD GetProcessPriority(DWORD dwProcessId)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        return -1;
    }
    DWORD dwPriorityClass;
    DWORD dwActualPriorityClass = GetPriorityClass(hProcess);

    switch (dwActualPriorityClass)
    {
        case IDLE_PRIORITY_CLASS:
            {
                dwPriorityClass = 0;
            }
            break;
        case BELOW_NORMAL_PRIORITY_CLASS:
            {
                dwPriorityClass = 1;
            }
            break;
        case NORMAL_PRIORITY_CLASS:
            {
                dwPriorityClass = 2;
            }
            break;
         case ABOVE_NORMAL_PRIORITY_CLASS:
             {
                dwPriorityClass = 3;
             }
            break;
        case HIGH_PRIORITY_CLASS:
            {
                dwPriorityClass = 4;
            }
            break;
        case REALTIME_PRIORITY_CLASS:
            {
                dwPriorityClass = 5;
            }
            break;
    }
    return dwPriorityClass;
}

void SetPriority(const char* PriorityType, DWORD dwProcessId, ULONG Priority){

    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        return;
    }

    if (strcmp(PriorityType, "MEM") == 0) {

        // Establecer prioridad de memoria
        MEMORY_PRIORITY_INFORMATION MemPrio;
        SecureZeroMemory(&MemPrio, sizeof(MemPrio));
        MemPrio.MemoryPriority = Priority;

        if (!SetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio))){
            MessageBox(0, "Error setting memory priority", "Error", MB_OK | MB_ICONERROR);
        }

    } else if (strcmp(PriorityType, "IO") == 0) {

        // Establecer prioridad E/S
        IO_PRIORITY_INFORMATION IoPrio;
        SecureZeroMemory(&IoPrio, sizeof(IoPrio));
        IoPrio.IoPriority = Priority;

        NTSTATUS status = NtSetInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio));
        if (!NT_SUCCESS(status)){
            MessageBox(0, "Error setting io priority", "Error", MB_OK | MB_ICONERROR);
        }
    }
}

bool GetPriority(const char* PriorityType, DWORD dwProcessId, ULONG& Priority) {

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        return false;
    }

    if (strcmp(PriorityType, "MEM") == 0) {
        // Obtener prioridad de memoria
        MEMORY_PRIORITY_INFORMATION MemPrio;
        SecureZeroMemory(&MemPrio, sizeof(MemPrio));

        if (GetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio))) {
            Priority = MemPrio.MemoryPriority;
        } else {
            MessageBox(0, "Error getting memory priority", "Error", MB_OK | MB_ICONERROR);
            CloseHandle(hProcess);
            return false;
        }
    } else if (strcmp(PriorityType, "IO") == 0) {
        // Obtener prioridad E/S
        IO_PRIORITY_INFORMATION IoPrio;
        SecureZeroMemory(&IoPrio, sizeof(IoPrio));

        ULONG ReturnLength = 0;
        NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio), &ReturnLength);
        if (NT_SUCCESS(status)){
            Priority = IoPrio.IoPriority;
        } else {
            MessageBox(0, "Error getting memory priority", "Error", MB_OK | MB_ICONERROR);
        }
    }

    CloseHandle(hProcess);
    return true;
}

void SetProcessQos(const char* QosType, DWORD dwProcessId)
{
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    PROCESS_POWER_THROTTLING_STATE PowerThrottling;
    SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

    if (strcmp(QosType, "ECO") == 0) {
        // Establecer estado de eficiencia (EcoQos)
        PowerThrottling.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

        if (!SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling))){
            MessageBox(0, "Error setting eco qos", "Error", MB_OK | MB_ICONERROR);
        }

    } else if (strcmp(QosType, "HIGH") == 0) {
        // Establecer estado de maximo rendimiento (HighQos)
        PowerThrottling.StateMask = 0x0;

        if (!SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling))){
            MessageBox(0, "Error setting high qos", "Error", MB_OK | MB_ICONERROR);
        }
    }
}

void GetProcessQos(DWORD dwProcessId, ULONG& Qos) {

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (!hProcess) {
        return;
    }
        PROCESS_POWER_THROTTLING_STATE PowerThrottling;
        SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
        PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;

        NTSTATUS status = ZwQueryInformationProcess(hProcess, ProcPowerThrottlingState, &PowerThrottling, sizeof(PowerThrottling), NULL);
        if (NT_SUCCESS(status)){
            Qos = PowerThrottling.StateMask;
        }
        CloseHandle(hProcess);

}

BOOL IsSuspended(DWORD dwProcessId) {
    ULONG bufferSize = 0;
    NTSTATUS status = ZwQuerySystemInformation(SystemProcessInformation, NULL, 0, &bufferSize);
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        MessageBox(0, "Error obtaining initial buffer size", "Error", MB_OK | MB_ICONERROR);
        return FALSE;  // Error al obtener tamaño inicial
    }

    PVOID buffer = NULL;
    do {
        buffer = malloc(bufferSize);
        if (!buffer) {
            MessageBox(0, "Error allocating memory", "Error", MB_OK | MB_ICONERROR);
            return FALSE;  // Error al asignar memoria
        }

        status = ZwQuerySystemInformation(SystemProcessInformation, buffer, bufferSize, NULL);
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            free(buffer);
            buffer = NULL;
        }
    } while (status == STATUS_INFO_LENGTH_MISMATCH);

    if (!NT_SUCCESS(status)) {
        MessageBox(0, "Error obtaining process information", "Error", MB_OK | MB_ICONERROR);
        free(buffer);
        return FALSE;  // Error al obtener información de procesos
    }

    BOOL anyThreadSuspended = FALSE;
    PSYSTEM_PROCESS_INFORMATION currentProcess = (PSYSTEM_PROCESS_INFORMATION)buffer;

    while (TRUE) {
        if (HandleToUlong(currentProcess->UniqueProcessId) == dwProcessId) {
            PSYSTEM_THREAD_INFORMATION threadInfo = (PSYSTEM_THREAD_INFORMATION)((PCHAR)currentProcess + sizeof(SYSTEM_PROCESS_INFORMATION));

            for (ULONG i = 0; i < currentProcess->NumberOfThreads; i++) {
                if (threadInfo->ThreadState == StateWait && threadInfo->WaitReason == Suspended) {
                    anyThreadSuspended = TRUE;
                    break;
                }
                threadInfo++;
            }
            break;
        }

        if (currentProcess->NextEntryOffset == 0) break;
        currentProcess = (PSYSTEM_PROCESS_INFORMATION)((PCHAR)currentProcess + currentProcess->NextEntryOffset);
    }

    free(buffer);
    return anyThreadSuspended;
}

void _SR(const char* ActionType, DWORD dwProcessId) {

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessId);

    if (strcmp(ActionType, "SUSPEND") == 0) {
        NtSuspendProcess(hProcess);
    } else if (strcmp(ActionType, "RESUME") == 0) {
        NtResumeProcess(hProcess);
    }
}

DWORD GetProcInfo(DWORD dwProcessId, LPSTR lpPath, DWORD nSize)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    if (hProcess == NULL) {
        return 0;
    }

    DWORD dwLength = nSize;
    if (!QueryFullProcessImageName(hProcess, 0, lpPath, &dwLength)) {
        CloseHandle(hProcess);
        return 0;
    }

    CloseHandle(hProcess);
    return dwLength;
}

void CheckMenuItemProcess(HMENU hMenu, char* ProcessName, DWORD dwProcessId){

    // Construir la clave del registro
    char registryKey[MAX_PATH];
    snprintf(registryKey, sizeof(registryKey), "Software\\SetProcess\\%s", ProcessName);

    EnablePrivilege(dwProcessId, SE_INC_BASE_PRIORITY_NAME);
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Marcar el ítem de menú correspondiente según la prioridad de CPU
    DWORD dwPriorityClass = GetProcessPriority(dwProcessId);
    UINT priorityClassId;

    switch (dwPriorityClass)
    {
        case 0:
            {
                priorityClassId = IDM_CPU_PRIORITY_IDLE;
            }
            break;
        case 1:
            {
                priorityClassId = IDM_CPU_PRIORITY_BELOW_NORMAL;
            }
            break;
        case 2:
            {
                priorityClassId= IDM_CPU_PRIORITY_NORMAL;
            }
            break;
        case 3:
            {
                priorityClassId = IDM_CPU_PRIORITY_ABOVE_NORMAL;
            }
            break;
        case 4:
            {
                priorityClassId = IDM_CPU_PRIORITY_HIGH;
            }

            break;
        case 5:
            {
                priorityClassId = IDM_CPU_PRIORITY_REALTIME;
            }
            break;
    }

    if (priorityClassId != 0)
    {
        CheckMenuItem(hMenu, priorityClassId, MF_BYCOMMAND | MF_CHECKED);
    }

    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
        CheckMenuItem(hMenu, IDM_CPU_PRIORITY_SAVE, MF_BYCOMMAND | MF_CHECKED);
    }

    // Marcar el ítem de menú correspondiente según la prioridad de memoria
    ULONG memPriority;
    GetPriority("MEM", dwProcessId, memPriority);
    UINT memPriorityId;
    switch (memPriority)
    {
        case 1:
            {
                memPriorityId = IDM_MEMORY_PRIORITY_VERYLOW;
            }
            break;
        case 2:
            {
                memPriorityId = IDM_MEMORY_PRIORITY_LOW;
            }
            break;
        case 3:
            {
                memPriorityId = IDM_MEMORY_PRIORITY_MEDIUM;
            }
            break;
        case 4:
            {
                memPriorityId = IDM_MEMORY_PRIORITY_BELOWNORMAL;
            }
            break;
        case 5:
            {
                memPriorityId = IDM_MEMORY_PRIORITY_NORMAL;
            }
            break;
    }

    if (memPriorityId != 0)
    {
        CheckMenuItem(hMenu, memPriorityId, MF_BYCOMMAND | MF_CHECKED);
    }

    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
        CheckMenuItem(hMenu, IDM_MEMORY_PRIORITY_SAVE, MF_BYCOMMAND | MF_CHECKED);
    }

    // Marcar el ítem de menú correspondiente según la prioridad de memoria
    ULONG ioPriority;
    GetPriority("IO", dwProcessId, ioPriority);
    UINT ioPriorityId;
    switch (ioPriority)
    {
        case 0:
            {
                ioPriorityId = IDM_IO_PRIORITY_VERYLOW;
            }
            break;
        case 1:
            {
                ioPriorityId = IDM_IO_PRIORITY_LOW;
            }
            break;
        case 2:
            {
                ioPriorityId = IDM_IO_PRIORITY_NORMAL;
            }
            break;
        case 3:
            {
                ioPriorityId = IDM_IO_PRIORITY_HIGH;
            }
            break;
    }

    if (ioPriorityId != 0)
    {
        CheckMenuItem(hMenu, ioPriorityId, MF_BYCOMMAND | MF_CHECKED);
    }

    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
        CheckMenuItem(hMenu, IDM_IO_PRIORITY_SAVE, MF_BYCOMMAND | MF_CHECKED);
    }

    // Verificar PriorityBoost
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
    BOOL pDisablePriorityBoost;
    if (GetProcessPriorityBoost(hProcess, &pDisablePriorityBoost)) {
        if (pDisablePriorityBoost == FALSE) {
            CheckMenuItem(hMenu, IDM_CPU_PRIORITY_BOOST, MF_BYCOMMAND | MF_CHECKED);
        }
    }

    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityBoost") != nullptr) {
        CheckMenuItem(hMenu, IDM_CPU_PRIORITY_BOOST_SAVE, MF_BYCOMMAND | MF_CHECKED);
    }

    //Verificar Qos
    ULONG Qos;
    GetProcessQos(dwProcessId, Qos);

    if (Qos == 0) {
        CheckMenuItem(hMenu, IDM_QOS_HIGHQOS, MF_BYCOMMAND | MF_CHECKED);
        if (GetProcessPriority(dwProcessId) == 0) {
            SetProcessPriority(dwProcessId, 2);
        }
    } else if (Qos == PROCESS_POWER_THROTTLING_EXECUTION_SPEED) {
        CheckMenuItem(hMenu, IDM_QOS_ECOQOS, MF_BYCOMMAND | MF_CHECKED);
        SetProcessPriority(dwProcessId, 0);


    }

    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "EcoQos") != nullptr) {
        CheckMenuItem(hMenu, IDM_QOS_SAVE, MF_BYCOMMAND | MF_CHECKED);
    } else if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "HighQos") != nullptr) {
        CheckMenuItem(hMenu, IDM_QOS_SAVE, MF_BYCOMMAND | MF_CHECKED);
    }

    if (IsSuspended(dwProcessId)) {
        ModifyMenu(hMenu, IDM_SUSPEND_RESUME, MF_BYCOMMAND | MF_STRING, IDM_SUSPEND_RESUME, "Resume");
    } else {
        ModifyMenu(hMenu, IDM_SUSPEND_RESUME, MF_BYCOMMAND | MF_STRING, IDM_SUSPEND_RESUME, "Suspend");
    }

}

void LoadConfigProcess(const char* ProcessName, DWORD dwProcessId)
{
    char registryKey[MAX_PATH];
    snprintf(registryKey, sizeof(registryKey), "Software\\SetProcess\\%s", ProcessName);

    EnablePrivilege(dwProcessId, SE_INC_BASE_PRIORITY_NAME);
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Cargar CpuSets
    char* cpuSetMask = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "CpuSets");

    if (cpuSetMask != nullptr)
    {
        DWORD newCpuBitmask = 0;
        for (DWORD i = 0; i < strlen(cpuSetMask); ++i) {
            if (cpuSetMask[i] == '1') {
                newCpuBitmask |= (1 << i);
            }
        }
        SetProcessCpuSetMask(dwProcessId, newCpuBitmask);
    }

    // Cargar IdealProcessor
    char* IdealProcStr = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IdealProc");
    if (IdealProcStr != nullptr)
    {
        DWORD IdealProc = atoi(IdealProcStr);
        SetIdealProcessor(dwProcessId, IdealProc);
    }

    // Cargar AffinityMask
    char* affinityMask = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "AffinityMask");

    if (affinityMask != nullptr)
    {
        DWORD_PTR dwProcessAffinityMask = 0;
        for (DWORD i = 0; i < strlen(affinityMask); ++i) {
            if (affinityMask[i] == '1') {
                dwProcessAffinityMask |= (1 << i);
            }
        }
        SetProcessAffinity(dwProcessId, dwProcessAffinityMask);
    }

    // Cargar PriorityBoost
    char* priorityBoost = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityBoost");
    if (priorityBoost != nullptr) {
        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);
        SetProcessPriorityBoost(hProcess, FALSE);
    }

    // Cargar PriorityClass
    char* Priority = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass");
    if (Priority != nullptr) {
        EnablePrivilege(dwProcessId, SE_DEBUG_NAME);
        SetProcessPriority(dwProcessId, atoi(Priority));
    }

    // Cargar MemPriority
    char* memPriority = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority");
    if (memPriority != nullptr) {
        SetPriority("MEM", dwProcessId, atoi(memPriority));
    }

    // Cargar IoPriority
    char* ioPriority = RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority");
    if (ioPriority != nullptr) {
        SetPriority("IO", dwProcessId, atoi(ioPriority));
    }

    // Cargar EcoQos
    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "EcoQos") != nullptr) {
        SetProcessQos("ECO", dwProcessId);
    }

    // Cargar HighQos
    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "HighQos") != nullptr) {
        SetProcessQos("HIGH", dwProcessId);
    }
}

void AddProcessToListBox(HWND hListBox, const TCHAR* processName, DWORD dwProcessId) {
    char buffer[MAX_PATH];
    _snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "%s (PID:%lu)", processName, dwProcessId);
    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)buffer);
}

void PopulateListBox(HWND hListBox) {
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Añadir procesos al ListBox
            AddProcessToListBox(hListBox, pe32.szExeFile, pe32.th32ProcessID);

            // Leer la configuración del registro para este proceso
            LoadConfigProcess(pe32.szExeFile, pe32.th32ProcessID);
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}

void ShowListBoxMenu(HWND hListBox, char* ProcessName, DWORD dwProcessId, int x, int y)
{
    HMENU hMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_CONTEXT_MENU));
    if (hMenu != NULL)
    {
        HMENU hPopupMenu = GetSubMenu(hMenu, 0);
        CheckMenuItemProcess(hMenu, ProcessName, dwProcessId);
        TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, GetParent(hListBox), NULL);
        DestroyMenu(hMenu);
    }
}

// Declaración del trayicon
NOTIFYICONDATA nid;
bool isIconVisible = false;

//ShowTrayMenu
void ShowTrayMenu(HWND hWnd)
{
    //obtener posición del puntero (x,y)
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, IDM_EXIT, "Exit");

    //poner en primer plano la ventana actual y centrar el menú
    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_NONOTIFY | TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
    DestroyMenu(hMenu);
}

HWINEVENTHOOK g_hookID;
HWND HWNDPrev = NULL;
void CALLBACK WinEventProc(
    HWINEVENTHOOK hWinEventHook,
    DWORD dwEvent,
    HWND hwnd,
    LONG idObject,
    LONG idChild,
    DWORD dwEventThread,
    DWORD dwmsEventTime
)

{
    if (dwEvent == EVENT_SYSTEM_FOREGROUND && hwnd != NULL && idObject == OBJID_WINDOW && hwnd != HWNDPrev || dwEvent == EVENT_OBJECT_CREATE && hwnd != NULL && idObject == OBJID_WINDOW && hwnd != HWNDPrev)
    {

        if (idChild != CHILDID_SELF) {
            return;
        }

        DWORD dwProcessId = 0;
        GetWindowThreadProcessId(hwnd, &dwProcessId);

        // Obtener el nombre del proceso asociado con el ID del proceso
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
        if (hProcess)
        {
            TCHAR szProcessName[MAX_PATH];
            if (GetModuleBaseName(hProcess, NULL, szProcessName, sizeof(szProcessName)))
            {
                char registryKey[MAX_PATH];
                snprintf(registryKey, sizeof(registryKey), "Software\\SetProcess\\%s", szProcessName);
                if (RegKeyExists(HKEY_CURRENT_USER, registryKey))
                {
                    LoadConfigProcess(szProcessName, dwProcessId);
                    CloseHandle(hProcess);
                    return;
                }
            }
            CloseHandle(hProcess);
        }
        HWNDPrev = hwnd;
    }
}

INT_PTR CALLBACK BitMaskDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD* pCPUs = nullptr;
    static DWORD MAX_CPUS = 0; // Tamaño máximo de CPUs, ajustar según tu necesidad

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Asignar pCPUs desde lParam
            pCPUs = reinterpret_cast<DWORD*>(lParam);

            if (!pCPUs)
            {
                EndDialog(hDlg, IDCANCEL);
                return (INT_PTR)FALSE;
            }

            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            MAX_CPUS = sysinfo.dwNumberOfProcessors;

            for (DWORD i = 0; i < 64; ++i)
            {
                if (i < MAX_CPUS) {
                    CheckDlgButton(hDlg, IDC_CPU_0 + i, pCPUs[i] == 1 ? BST_CHECKED : BST_UNCHECKED);
                    EnableWindow(GetDlgItem(hDlg, IDC_CPU_0 + i), TRUE);
                } else {
                    EnableWindow(GetDlgItem(hDlg, IDC_CPU_0 + i), FALSE);
                }
            }
            return (INT_PTR)TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_SAVE_BITMASK:
                {
                    // Leer el estado de los checkboxes y guardar en pCPUs
                    for (DWORD i = 0; i < MAX_CPUS; ++i)
                    {
                        pCPUs[i] = IsDlgButtonChecked(hDlg, IDC_CPU_0 + i) == BST_CHECKED ? 1 : 0;
                    }

                    EndDialog(hDlg, 0);
                    return (INT_PTR)TRUE;
                }

                case IDC_CHECK_ALL:
                {
                    // Marcar todos los checkboxes
                    for (DWORD i = 0; i < MAX_CPUS; ++i)
                    {
                        CheckDlgButton(hDlg, IDC_CPU_0 + i, BST_CHECKED);
                    }

                    return (INT_PTR)TRUE;
                }

                case IDC_UNCHECK:
                {
                    // Desmarcar todos los checkboxes
                    for (DWORD i = 0; i < MAX_CPUS; ++i)
                    {
                        CheckDlgButton(hDlg, IDC_CPU_0 + i, BST_UNCHECKED);
                    }

                    return (INT_PTR)TRUE;
                }
            }
            break;
    }
    return (INT_PTR)FALSE;
}

void ShowBitMaskDialog(HWND hParent, DWORD* CPUs)
{
    DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(DLG_BITMASK), hParent, BitMaskDlgProc, reinterpret_cast<LPARAM>(CPUs));
}

