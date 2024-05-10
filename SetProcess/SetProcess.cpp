#include "HelpProcess.h"

// Definir una estructura para asociar opciones con funciones
typedef struct {
    const char* option;
    void (*action)(int argc, char* argv[]);
} OptionAction;

// Funciones para cada acción
void help_action(int argc, char* argv[]) {
    printf("uso: sp.exe <Argument> <ImageName> <Parameter>\n");
    printf("\n  -c   <IM> <core0,core2...> <-r>     |    CpuSets");
    printf("\n  -idp <IM> <core> <-r>               |    Ideal Processor");
    printf("\n  -a   <IM> <core0,core2...> <-r>     |    Affinity");
    printf("\n  -au  <IM> <0-1> <-r>                |    Affinity Update Mode");
    printf("\n  -p   <IM> <0-5> <-r>                |    CPU Priority");
    printf("\n  -pb  <IM> <-r>                      |    CPU Priority Boost");
    printf("\n  -mp  <IM> <1-5> <-r>                |    Memory Priority");
    printf("\n  -iop <IM> <0-3> <-r>                |    I/O Priority");
    printf("\n  -eq  <IM> <-r>                      |    EcoQoS (Efficiency Mode)");
    printf("\n  -hq  <IM> <-r>                      |    HighQoS (High Performance Mode)");
    printf("\n  -s   <IM> <-r>                      |    Suspend");
    printf("\n  -r   <IM> <-r>                      |    Resume\n");
}

void cpu_set_action(int argc, char* argv[]) {


    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Habilitar privilegio para aumentar la prioridad de programación
    EnablePrivilege(PID, SE_INC_BASE_PRIORITY_NAME);

    // parser
    ULONG CpuSetIdCount, Ids = ConvertToBitMask(argv[3], CpuSetIdCount);
    ULONG* CpuSetIds = &Ids;

    // Establecer Conjuntos de CPU
    if (SetProcessDefaultCpuSetsID(PID, CpuSetIds, CpuSetIdCount)){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD ChildPIDs[64] = {0}; // Inicializar el array a 0
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                SetProcessDefaultCpuSetsID(ChildPIDs[i], CpuSetIds, CpuSetIdCount);
            }

        }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer los conjuntos de CPU: %lu", GetLastError());
    }

    return;

}

void ideal_processor_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Numero de procesadores ideales
    DWORD dwIdealProcessor = ParseArg(argv[3]);

    // Establecer procesador ideal
    SetIdealProcessor(PID, dwIdealProcessor);
        // Establecerlo en todos los procesos hijos
    if (argc == 5 && strcmp(argv[4], "-r") == 0){
        DWORD ChildPIDs[64] = {0}; // Inicializar el array a 0
        DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

        for (DWORD i = 0; i < NumProcesses; i++) {
            // Establecer procesador ideal
            SetIdealProcessor(ChildPIDs[i], dwIdealProcessor);
        }

    }
    printf("Sucess");
    return;
}

void affinity_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // parser
    DWORD dwProcessAffinityMask = ConvertToBitMask(argv[3]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer Afinidad
    if (SetProcessAffinityMask(hProcess, dwProcessAffinityMask) != 0){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD ChildPIDs[64] = {0}; // Inicializar el array a 0
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                SetProcessAffinityMask(hProcess, dwProcessAffinityMask);
                CloseHandle(hProcess);
            }

        }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer la afinidad: %lu", GetLastError());
    }

    // Cerrar Handle
    CloseHandle(hProcess);

    return;
}

void affinity_update_mode_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    DWORD dwFlags;
    switch (atoi(argv[3])){
        case 0:
        {
            dwFlags = 0;
        }
        break;
        case 1:
        {
            dwFlags = PROCESS_AFFINITY_ENABLE_AUTO_UPDATE;
        }
        break;
    }


    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer Modo de Actualizacion de la Afinidad
    if (SetProcessAffinityUpdateMode(hProcess, dwFlags) == 0){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD ChildPIDs[64] = {0}; // Inicializar el array a 0
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                SetProcessAffinityUpdateMode(hProcess, dwFlags);
                CloseHandle(hProcess);
            }

        }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer el modo de actualizacion de afinidad: %lu", GetLastError());
    }

    // Cerrar Handle
    CloseHandle(hProcess);

    return;
}

void cpu_priority_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

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
            printf("Prioridad maxima 5 (REALTIME_PRIORITY_CLASS)");
            return;
        }
        break;

    }

    // Establecer prioridad
    if (SetPriorityClass(hProcess, dwPriorityClass) != 0){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD ChildPIDs[64] = {0};
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                SetPriorityClass(hProcess, dwPriorityClass);
                CloseHandle(hProcess);
            }

        }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer la prioridad de cpu: %lu", GetLastError());
    }

    // Cerrar Handle
    CloseHandle(hProcess);

    return;
}

void cpu_priority_boost_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    if (SetProcessPriorityBoost(hProcess, FALSE) != 0){
        // Establecerlo en todos los procesos hijos
        if (argc == 4 && strcmp(argv[3], "-r") == 0){
            DWORD ChildPIDs[64] = {0};
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                SetProcessPriorityBoost(hProcess, FALSE);
                CloseHandle(hProcess);
            }

        }
        printf("Sucess");

    }
    else
    {
        printf("Error al establecer el refuerzo dinámico");
    }
    CloseHandle(hProcess);
    return;

}

void memory_priority_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer prioridad de memoria
    MEMORY_PRIORITY_INFORMATION MemPrio;
    SecureZeroMemory(&MemPrio, sizeof(MemPrio));
    MemPrio.MemoryPriority = atoi(argv[3]);

    if (SetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio)) != 0){
        // Establecerlo en todos los procesos hijos
        if (argc == 5 && strcmp(argv[4], "-r") == 0){
            DWORD ChildPIDs[64] = {0};
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                SetProcessInformation(hProcess, ProcessMemoryPriority, &MemPrio, sizeof(MemPrio));
                CloseHandle(hProcess);
            }

        }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer la prioridad de memoria: %lu", GetLastError());
    }

    CloseHandle(hProcess);
    return;

}

void io_priority_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer prioridad E/S
    IO_PRIORITY_INFORMATION IoPrio;
    SecureZeroMemory(&IoPrio, sizeof(IoPrio));
    IoPrio.IoPriority = atoi(argv[3]);

    NtSetInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio));
    // Establecerlo en todos los procesos hijos
    if (argc == 5 && strcmp(argv[4], "-r") == 0){
        DWORD ChildPIDs[64] = {0};
        DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

        for (DWORD i = 0; i < NumProcesses; i++) {
            HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
            NtSetInformationProcess(hProcess, ProcessIoPriority, &IoPrio, sizeof(IoPrio));
            CloseHandle(hProcess);
        }

    }
    printf("Sucess");

    CloseHandle(hProcess);
    return;

}

void eco_qos_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer estado de eficiencia (EcoQos)
    PROCESS_POWER_THROTTLING_STATE PowerThrottling;

    SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

    if (SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling)) != 0){
            // Establecerlo en todos los procesos hijos
            if (argc == 4 && strcmp(argv[3], "-r") == 0){
                DWORD ChildPIDs[64] = {0};
                DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

                for (DWORD i = 0; i < NumProcesses; i++) {
                    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                    SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
                    CloseHandle(hProcess);
                }
            }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer el modo EcoQos: %lu", GetLastError());
    }

    CloseHandle(hProcess);
    return;
}

void high_qos_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    // Obtener HANDLE
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, PID);

    // Establecer estado de maximo rendimiento (HighQos)
    PROCESS_POWER_THROTTLING_STATE PowerThrottling;

    SecureZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
    PowerThrottling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
    PowerThrottling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
    PowerThrottling.StateMask = 0;

    if (SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling)) != 0){
            // Establecerlo en todos los procesos hijos
            if (argc == 4 && strcmp(argv[3], "-r") == 0){
                DWORD ChildPIDs[64] = {0};
                DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

                for (DWORD i = 0; i < NumProcesses; i++) {
                    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, ChildPIDs[i]);
                    SetProcessInformation(hProcess, ProcessPowerThrottling, &PowerThrottling, sizeof(PowerThrottling));
                    CloseHandle(hProcess);
                }
            }
        printf("Sucess");
    }
    else
    {
        printf("Error al establecer el modo HighQoS: %lu", GetLastError());
    }

    CloseHandle(hProcess);
    return;
}

void suspend_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, PID);

    // Obtener privilegio de depuracion (por si falla)
    EnablePrivilege(PID, SE_DEBUG_NAME);

    // Suspender
    NtSuspendProcess(hProcess);
        // Establecerlo en todos los procesos hijos
        if (argc == 4 && strcmp(argv[3], "-r") == 0){
            DWORD ChildPIDs[64] = {0};
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, ChildPIDs[i]);
                EnablePrivilege(ChildPIDs[i], SE_DEBUG_NAME);
                NtSuspendProcess(hProcess);
                CloseHandle(hProcess);
            }
        }
    printf("Sucess");

    CloseHandle(hProcess);
    return;
}

void resume_action(int argc, char* argv[]) {

    // Obtener PID
    DWORD PID = GetPID(argv[2]);

    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, PID);

    // Obtener privilegio de depuracion (por si falla)
    EnablePrivilege(PID, SE_DEBUG_NAME);

    // Reanudar
    NtResumeProcess(hProcess);
        // Establecerlo en todos los procesos hijos
        if (argc == 4 && strcmp(argv[3], "-r") == 0){
            DWORD ChildPIDs[64] = {0};
            DWORD NumProcesses = GetChildProcesses(PID, ChildPIDs);

            for (DWORD i = 0; i < NumProcesses; i++) {
                HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, ChildPIDs[i]);
                EnablePrivilege(ChildPIDs[i], SE_DEBUG_NAME);
                NtResumeProcess(hProcess);
                CloseHandle(hProcess);
            }
        }
    printf("Sucess");

    CloseHandle(hProcess);
    return;
}

// Definir las opciones y las funciones asociadas
OptionAction option_actions[] = {
    {"help", help_action},
    {"-c", cpu_set_action},
    {"-idp", ideal_processor_action},
    {"-a", affinity_action},
    {"-au", affinity_update_mode_action},
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
    EnablePrivilege(GetCurrentProcessId(), SE_DEBUG_NAME, GetCurrentProcess());

    // Hacerse pasar por el hilo del sistema
    ImpersonateSecurity();

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
