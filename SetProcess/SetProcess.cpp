#include "SetProcess.h"

// Definir una estructura para asociar opciones con funciones
typedef struct {
    const char* option;
    void (*action)(int argc, char* argv[]);
} OptionAction;

// Funciones para cada acción
void help_action(int argc, char* argv[]) {
    printf("use: sp.exe <Argument> <ImageName or PID> <Argument>\n");
    printf("\n  -c   <IM/PID> <Cores> <-r>              |    CpuSets");
    printf("\n  -idp <IM/PID> <Core> <-r>               |    Ideal Processor");
    printf("\n  -a   <IM/PID> <Cores> <-r>              |    Affinity");
    printf("\n  -p   <IM/PID> <0-5> <-r>                |    CPU Priority");
    printf("\n  -pb  <IM/PID> <-r>                      |    CPU Priority Boost");
    printf("\n  -mp  <IM/PID> <1-5> <-r>                |    Memory Priority");
    printf("\n  -iop <IM/PID> <0-3> <-r>                |    I/O Priority");
    printf("\n  -eq  <IM/PID> <-r>                      |    EcoQoS (Efficiency Mode)");
    printf("\n  -hq  <IM/PID> <-r>                      |    HighQoS (High Performance Mode)");
    printf("\n  -s   <IM/PID> <-r>                      |    Suspend");
    printf("\n  -r   <IM/PID> <-r>                      |    Resume\n");
}

void cpu_set_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);
    EnablePrivilege(dwProcessId, SE_INC_BASE_PRIORITY_NAME);

    DWORD IdCount;
    ULONG* Ids = ConvertToCpuSetIds(argv[3], &IdCount);

    // Establecer Conjuntos de CPU
    if (SetProcessCpuSetID(dwProcessId, Ids, IdCount)){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD dwChildProcessId[64] = {0}; // Inicializar el array a 0
            DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

            for (DWORD i = 0; i < NumProcesses; i++) {
                SetProcessCpuSetID(dwChildProcessId[i], Ids, IdCount);
            }

        }
    }
    return;

}

void ideal_processor_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Numero de procesadores ideales
    DWORD dwIdealProcessor = atoi(argv[3]);

    // Establecer procesador ideal
    SetIdealProcessor(dwProcessId, dwIdealProcessor);
        // Establecerlo en todos los procesos hijos
    if (argc == 5 && strcmp(argv[4], "-r") == 0){
        DWORD dwChildProcessId[64] = {0}; // Inicializar el array a 0
        DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

        for (DWORD i = 0; i < NumProcesses; i++) {
            // Establecer procesador ideal
            SetIdealProcessor(dwChildProcessId[i], dwIdealProcessor);
        }

    }
    return;
}

void affinity_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    DWORD dwProcessAffinityMask = ConvertToBitMask(argv[3]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Establecer Afinidad
    if (SetProcessAffinityMask(hProcess, dwProcessAffinityMask)){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD dwChildProcessId[64] = {0}; // Inicializar el array a 0
            DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                SetProcessAffinityMask(hProcess, dwProcessAffinityMask);
                CloseHandle(hProcess);
            }

        }
    }

    // Cerrar Handle
    CloseHandle(hProcess);

    return;
}

void cpu_priority_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Convertir char a dword
    DWORD dwPriorityClass;
    switch (atoi(argv[3]))
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
        default:
        {
            MessageBox(0, "Maximum priority: 5 (REALTIME_PRIORITY_CLASS)", "Error", MB_OK | MB_ICONINFORMATION);
            return;
        }
        break;

    }

    // Establecer prioridad
    if (SetPriorityClass(hProcess, dwPriorityClass)){

        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD dwChildProcessId[64] = {0};
            DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                SetPriorityClass(hProcess, dwPriorityClass);
                CloseHandle(hProcess);
            }

        }
    }

    // Cerrar Handle
    CloseHandle(hProcess);

    return;
}

void cpu_priority_boost_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);
    EnablePrivilege(dwProcessId, SE_INC_BASE_PRIORITY_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    if (SetProcessPriorityBoost(hProcess, FALSE)){
        // Establecerlo en todos los procesos hijos
        if (argc == 4 && strcmp(argv[3], "-r") == 0){
            DWORD dwChildProcessId[64] = {0};
            DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                SetProcessPriorityBoost(hProcess, FALSE);
                CloseHandle(hProcess);
            }

        }
    }
    CloseHandle(hProcess);
    return;

}

void memory_priority_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Establecer prioridad de memoria
    MEMORY_PRIORITY_INFORMATION MemPrio;
    SecureZeroMemory(&MemPrio, sizeof(MemPrio));
    MemPrio.MemoryPriority = atoi(argv[3]);

    if (SetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio))){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD dwChildProcessId[64] = {0};
            DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                SetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio));
                CloseHandle(hProcess);
            }

        }
    }

    CloseHandle(hProcess);
    return;

}

void io_priority_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Establecer prioridad E/S
    IO_PRIORITY_INFORMATION IoPrio;
    SecureZeroMemory(&IoPrio, sizeof(IoPrio));
    IoPrio.IoPriority = atoi(argv[3]);

    NtSetInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio));
    // Establecerlo en todos los procesos hijos
    if (argc == 5 && strcmp(argv[4], "-r") == 0){
        DWORD dwChildProcessId[64] = {0};
        DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

        for (DWORD i = 0; i < NumProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
            NtSetInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio));
            CloseHandle(hProcess);
        }

    }

    CloseHandle(hProcess);
    return;

}

void eco_qos_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Establecer estado de eficiencia (EcoQos)
    PROCESS_POWER_THROTTLING_STATE PowerThrottling;

    SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

    if (SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling))){
            // Establecerlo en todos los procesos hijos
            if (argc == 4 && strcmp(argv[3], "-r") == 0){
                DWORD dwChildProcessId[64] = {0};
                DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

                for (DWORD i = 0; i < NumProcesses; i++) {
                    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                    SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
                    CloseHandle(hProcess);
                }
            }
    }

    CloseHandle(hProcess);
    return;
}

void high_qos_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwProcessId);

    // Establecer estado de maximo rendimiento (HighQos)
    PROCESS_POWER_THROTTLING_STATE PowerThrottling;

    SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = 0;

    if (SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling))){
            // Establecerlo en todos los procesos hijos
            if (argc == 4 && strcmp(argv[3], "-r") == 0){
                DWORD dwChildProcessId[64] = {0};
                DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

                for (DWORD i = 0; i < NumProcesses; i++) {
                    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, dwChildProcessId[i]);
                    SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
                    CloseHandle(hProcess);
                }
            }
    }

    CloseHandle(hProcess);
    return;
}

void suspend_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessId);

    // Obtener privilegio de depuracion (por si falla)
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Suspender
    NtSuspendProcess(hProcess);

    // Establecerlo en todos los procesos hijos
    if (argc == 4 && strcmp(argv[3], "-r") == 0){
        DWORD dwChildProcessId[64] = {0};
        DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

        for (DWORD i = 0; i < NumProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwChildProcessId[i]);
            EnablePrivilege(dwChildProcessId[i], SE_DEBUG_NAME);
            NtSuspendProcess(hProcess);
            CloseHandle(hProcess);
        }
    }

    CloseHandle(hProcess);
    return;
}

void resume_action(int argc, char* argv[]) {

    // Obtener PID, Obtener Privilegio
    DWORD dwProcessId = GetPID(argv[2]);
    if (dwProcessId == 0)
    {
        dwProcessId = atoi(argv[2]);
    }
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwProcessId);

    // Obtener privilegio de depuracion (por si falla)
    EnablePrivilege(dwProcessId, SE_DEBUG_NAME);

    // Reanudar
    NtResumeProcess(hProcess);

    // Establecerlo en todos los procesos hijos
    if (argc == 4 && strcmp(argv[3], "-r") == 0){
        DWORD dwChildProcessId[64] = {0};
        DWORD NumProcesses = GetChildProcesses(dwProcessId, dwChildProcessId);

        for (DWORD i = 0; i < NumProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwChildProcessId[i]);
            EnablePrivilege(dwChildProcessId[i], SE_DEBUG_NAME);
            NtResumeProcess(hProcess);
            CloseHandle(hProcess);
        }
    }

    CloseHandle(hProcess);
    return;
}

// Definir las opciones y las funciones asociadas
OptionAction option_actions[] = {
    {"help", help_action},
    {"-c", cpu_set_action},
    {"-idp", ideal_processor_action},
    {"-a", affinity_action},
    {"-p", cpu_priority_action},
    {"-pb", cpu_priority_boost_action},
    {"-mp", memory_priority_action},
    {"-iop", io_priority_action},
    {"-eq", eco_qos_action},
    {"-hq", high_qos_action},
    {"-s", suspend_action},
    {"-r", resume_action},
    {NULL, NULL} // ultimo elemento
};

//buscar parametro que coincida
int main(int argc, char* argv[]) {

    if (argc < 2) {
        help_action(argc, argv);
        return 0;
    }

    // obtener privilegio de depuracion para evitar el descriptor de seguridad de otros procesos
    EnablePrivilege(GetCurrentProcessId(), SE_DEBUG_NAME);
    EnablePrivilege(GetCurrentProcessId(), SE_INC_BASE_PRIORITY_NAME);

    // Hacerse pasar por el hilo del sistema
    ImpersonateSystem();

    for (int i = 0; option_actions[i].option != NULL; i++) {
        if (strcmp(argv[1], option_actions[i].option) == 0) {
            option_actions[i].action(argc, argv);
            return 0;
        }
    }
    // Si la opción no coincide con ninguna acción conocida, mostrar ayuda
    help_action(argc, argv);
    return 0;
}
