#include "SetProcessGui.h"

BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
    {
        // Impersonacion
        ImpersonateSystem();
        EnablePrivilege(GetCurrentProcessId(), SE_INC_BASE_PRIORITY_NAME);
        EnablePrivilege(GetCurrentProcessId(), SE_DEBUG_NAME);

        //establecer hIcon
        HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
        SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

        // Inicializar la estructura NOTIFYICONDATA
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwndDlg;
        nid.uID = 1;
        nid.uFlags = NIF_ICON | NIF_MESSAGE;
        nid.uCallbackMessage = WM_ICON;
        nid.hIcon = hIcon;
        strcpy(nid.szTip, "SetProcess");

        // Destruir icono
        DestroyIcon(hIcon);

        // Agregar el icono a la bandeja del sistema
        Shell_NotifyIcon(NIM_ADD, &nid);

        // Crear ListBox
        PopulateListBox(GetDlgItem(hwndDlg, IDC_LISTP));

        // Crear ToolTip
        CreateToolTip(hwndDlg, GetDlgItem(hwndDlg, IDC_LISTP), NULL);

        // verificar si el inicio automatico esta habilitado
        char* start = RegKeyQuery(HKEY_CURRENT_USER, "Software\\SetProcess", "Start");
        if (start)
        {
            Sleep(5000);
            CheckDlgButton(hwndDlg, _START, BST_CHECKED);
        }

        if (start)
        {
            //enviar mensaje de ocultar el dialogo
            PostMessage(hwndDlg, WM_HIDE, 0, 0);
        }

        _drain();

    }
    return TRUE;

    case WM_HIDE:
    {
        // Ocultar dialogo
        ShowWindow(hwndDlg, SW_HIDE);
    }
    return TRUE;

    case WM_ICON:
    {
        switch (lParam)
        {
            case WM_RBUTTONDOWN:
            {
                // Mostrar el menú de la bandeja al hacer clic derecho en el icono
                ShowTrayMenu(hwndDlg);
            }
            return TRUE;
            case WM_LBUTTONDOWN:
            {
                //Mostrar dialogo al hacer clic izquierdo
                SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_END);
                ShowWindow(hwndDlg, SW_SHOWDEFAULT);
                PopulateListBox(GetDlgItem(hwndDlg, IDC_LISTP));

            }
            return TRUE;
        }
    }
    return TRUE;

    case WM_CONTEXTMENU:
    {
        HWND hListBox = GetDlgItem(hwndDlg, IDC_LISTP);
        if ((HWND)wParam == hListBox)  // Verifica si el evento ocurrió en el ListBox
        {
            int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
            if (index != LB_ERR)
            {
                // Obtener el texto del item seleccionado, Obtener PID
                char itemText[MAX_PATH];
                SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)itemText);
                DWORD dwProcessId = ExtractPID(itemText);
                char ProcessName[MAX_PATH];
                ExtractProcessName(itemText, ProcessName, sizeof(ProcessName));

                // Obtén las coordenadas del cursor en la pantalla
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);

                // Mostrar el menú contextual del ListBox
                ShowListBoxMenu(hListBox, ProcessName, dwProcessId, x, y);
            }
        }
    }
    return TRUE;

    case WM_COMMAND:
    {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        DWORD MAX_CPUS = sysinfo.dwNumberOfProcessors;

        // Obtener el item seleccionado
        HWND hListBox = GetDlgItem(hwndDlg, IDC_LISTP);
        int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);

        // Obtener el texto del item seleccionado, Obtener PID
        char itemText[MAX_PATH];
        SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)itemText);
        DWORD dwProcessId = ExtractPID(itemText);
        char ProcessName[MAX_PATH];
        ExtractProcessName(itemText, ProcessName, sizeof(ProcessName));

        // Obtener privilegios
        EnablePrivilege(dwProcessId, SE_DEBUG_NAME);
        EnablePrivilege(dwProcessId, SE_INC_BASE_PRIORITY_NAME);

        // Construir la clave del registro
        char registryKey[MAX_PATH];
        snprintf(registryKey, sizeof(registryKey), "Software\\SetProcess\\%s", ProcessName);

        switch(LOWORD(wParam))
        {

            case IDC_REFRESH:
                {
                    // refrescar listbox
                    PopulateListBox(GetDlgItem(hwndDlg, IDC_LISTP));
                }
                return TRUE;
            case IDC_LISTP:
                {
                    if (index != LB_ERR)
                    {
                        // Obtener el PID, Obtener Path y Refrescar ToolTip
                        char SzInfo[MAX_PATH] = { 0 };
                        GetProcInfo(dwProcessId, SzInfo, MAX_PATH);
                        UpdateToolTipText(hListBox, SzInfo);
                    }
                }
                return TRUE;

            case IDM_CPUSETS:
            {
                if (index != LB_ERR)
                {
                    ULONG cpuSetIds[MAX_CPUS] = {0};
                    ULONG cpuSetIdCount = MAX_CPUS;

                    // Obtener los IDs de los conjuntos de CPU para el proceso actual
                    if(!GetProcessCpuSetId(dwProcessId, cpuSetIds, &cpuSetIdCount)){
                        memset(cpuSetIds, 0, sizeof(cpuSetIds));
                        cpuSetIdCount = 0;
                    }

                    DWORD cpuSetBitMask[MAX_CPUS] = {0}; // Crear la bitmask como un array
                    for (ULONG i = 0; i < cpuSetIdCount; ++i)
                    {
                        // Convertir cada ID de CPU en la bitmask correspondiente
                        if (cpuSetIds[i] >= 256) {
                            cpuSetBitMask[cpuSetIds[i] - 256] = 1;
                        }
                    }

                    // Mostrar el diálogo para seleccionar conjuntos de CPU
                    ShowBitMaskDialog(hwndDlg, cpuSetBitMask);

                    bool anyProcessorSelected = false;
                    for (ULONG i = 0; i < MAX_CPUS; ++i)
                    {
                        if (cpuSetBitMask[i] != 0)
                        {
                            anyProcessorSelected = true;
                            break;
                        }
                    }

                    if (!anyProcessorSelected)
                    {
                        // Si no se seleccionó ningún procesador, revertir los cambios
                        SetProcessCpuSetId(dwProcessId, NULL, 0);
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "CpuSets");
                        break;
                    }

                    memset(cpuSetIds, 0, sizeof(cpuSetIds));
                    ULONG idIndex = 0;

                    for (ULONG i = 0; i < MAX_CPUS; ++i)
                    {
                        // Verificar si el bit en la posición i está activo en cpuSetBitMask
                        if (cpuSetBitMask[i] == 1)
                        {
                            cpuSetIds[idIndex++] = i + 256;
                        }
                    }

                    // Establecer los conjuntos de CPU para el proceso
                    SetProcessCpuSetId(dwProcessId, cpuSetIds, idIndex);

                    char cpuSetIdsString[MAX_CPUS + 1] = { 0 }; // +1 para el terminador nulo
                    for (ULONG i = 0; i < MAX_CPUS; ++i)
                    {
                        cpuSetIdsString[i] = (cpuSetBitMask[i] == 1) ? '1' : '0';
                    }
                    cpuSetIdsString[MAX_CPUS] = '\0'; // Asegurar el terminador nulo
                    RegKeySet(HKEY_CURRENT_USER, registryKey, "CpuSets", cpuSetIdsString);
                }
            }
            return TRUE;

            case IDM_IDEAL_PROCESSOR:
                {
                    if (index != LB_ERR)
                    {
                        DWORD CPUs[MAX_CPUS] = {0};

                        // Obtener el procesador ideal del hilo del proceso
                        PROCESSOR_NUMBER idealProcessor;
                        BOOL success = GetIdealProcessor(dwProcessId, &idealProcessor);

                        DWORD idealProcessorIndex = idealProcessor.Number;

                        for (DWORD i = 0; i < MAX_CPUS; ++i)
                        {
                            CPUs[i] = (i == idealProcessorIndex) ? 1 : 0;
                        }

                        ShowBitMaskDialog(hwndDlg, CPUs);
                        int activeIndex = GetSingleCoreIndex(CPUs, MAX_CPUS);
                        if (activeIndex == -1){
                            RegKeyDelete(HKEY_CURRENT_USER, registryKey, "IdealProc");
                            break;
                        }

                        // Establecer procesador ideal
                        SetIdealProcessor(dwProcessId, activeIndex);

                       // Convertir el índice del procesador a cadena
                        char activeIndexStr[16];
                        snprintf(activeIndexStr, sizeof(activeIndexStr), "%d", activeIndex);
                        RegKeySet(HKEY_CURRENT_USER, registryKey, "IdealProc", activeIndexStr);

                    }
                }
                return TRUE;

            case IDM_AFFINITY:
                {
                    if (index != LB_ERR)
                    {
                        DWORD CPUs[MAX_CPUS] = {0};
                        DWORD_PTR dwProcessAffinityMask = GetProcessAffinity(dwProcessId);
                        if (dwProcessAffinityMask != 0)
                        {
                            // Preparar la bitmask para mostrar en el diálogo
                            for (DWORD i = 0; i < MAX_CPUS; ++i)
                            {
                                if (dwProcessAffinityMask & (1 << i))
                                {
                                    CPUs[i] = 1; // Marcar el CPU i en el arreglo CPUs
                                }
                                else
                                {
                                    CPUs[i] = 0;
                                }
                            }
                        }

                        ShowBitMaskDialog(hwndDlg, CPUs);

                        bool OneProcessorSelected = false;
                        bool AllProcessorsSelected = true;
                        DWORD selectedCoreCount = 0;

                        // Verificar si al menos un procesador está seleccionado y contar los seleccionados
                        for (DWORD i = 0; i < MAX_CPUS; ++i) {
                            if (CPUs[i] == 1) {
                                OneProcessorSelected = true;
                                selectedCoreCount++;
                            } else {
                                AllProcessorsSelected = false;
                            }
                        }

                        if (!OneProcessorSelected) {
                            break;
                        }

                        dwProcessAffinityMask = 0;

                        // Configurar la máscara de afinidad de procesador
                        for (DWORD i = 0; i < MAX_CPUS; ++i) {
                            if (CPUs[i] != 0) {
                                dwProcessAffinityMask |= (1 << i); // Establecer el bit correspondiente a CPU i
                            }
                        }

                        if (AllProcessorsSelected) {
                            // Establecer afinidad
                            SetProcessAffinity(dwProcessId, dwProcessAffinityMask);
                            RegKeyDelete(HKEY_CURRENT_USER, registryKey, "AffinityMask");
                            break;
                        }

                        // Establecer afinidad
                        SetProcessAffinity(dwProcessId, dwProcessAffinityMask);

                        // Convertir la nueva bitmask a una cadena de bits para almacenar en el registro
                        char AffinityMaskStr[MAX_CPUS + 1] = { 0 }; // Más 1 para el terminador nulo
                        for (DWORD i = 0; i < MAX_CPUS; ++i)
                        {
                            AffinityMaskStr[i] = (dwProcessAffinityMask & (1 << i)) ? '1' : '0';
                        }

                        // Escribir la nueva bitmask en el registro
                        RegKeySet(HKEY_CURRENT_USER, registryKey, "AffinityMask", AffinityMaskStr);

                    }
                }
                return TRUE;

            case IDM_CPU_PRIORITY_BOOST:
                {
                    if (index != LB_ERR)
                    {
                        HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
                        BOOL pDisablePriorityBoost;
                        if (GetProcessPriorityBoost(hProcess, &pDisablePriorityBoost)) {
                            if (pDisablePriorityBoost == FALSE) {
                                SetProcessPriorityBoost(hProcess, TRUE);
                                if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityBoost") != nullptr) {
                                    RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityBoost", "0");
                                }
                            } else {
                                SetProcessPriorityBoost(hProcess, FALSE);
                                if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityBoost") != nullptr) {
                                    RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityBoost", "1");
                                }
                            }
                        }
                    }

                }
                return TRUE;

            case IDM_CPU_PRIORITY_IDLE:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 0);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "0");
                        }
                    }

                }
                return TRUE;

            case IDM_CPU_PRIORITY_BELOW_NORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 1);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "1");
                        }

                    }

                }
                return TRUE;

            case IDM_CPU_PRIORITY_NORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 2);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "2");
                        }
                    }
                }
                return TRUE;

            case IDM_CPU_PRIORITY_ABOVE_NORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 3);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "3");
                        }
                    }
                }
                return TRUE;

            case IDM_CPU_PRIORITY_HIGH:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 4);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "4");
                        }
                    }

                }
                return TRUE;

            case IDM_CPU_PRIORITY_REALTIME:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessPriority(dwProcessId, 5);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", "5");
                        }
                    }

                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_VERYLOW:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("MEM", dwProcessId, 1);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", "1");
                        }
                    }
                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_LOW:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("MEM", dwProcessId, 2);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", "2");
                        }

                    }
                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_MEDIUM:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("MEM", dwProcessId, 3);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", "3");
                        }
                    }
                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_BELOWNORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("MEM", dwProcessId, 4);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", "4");
                        }
                    }
                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_NORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("MEM", dwProcessId, 5);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", "5");
                        }
                    }
                }
                return TRUE;

            case IDM_IO_PRIORITY_VERYLOW:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("IO", dwProcessId, 0);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "IoPriority", "0");
                        }
                    }
                }
                return TRUE;

            case IDM_IO_PRIORITY_LOW:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("IO", dwProcessId, 1);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "IoPriority", "1");
                        }
                    }
                }
                return TRUE;

            case IDM_IO_PRIORITY_NORMAL:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("IO", dwProcessId, 2);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "IoPriority", "2");
                        }
                    }
                }
                return TRUE;

            case IDM_IO_PRIORITY_HIGH:
                {
                    if (index != LB_ERR)
                    {
                        SetPriority("IO", dwProcessId, 3);
                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "IoPriority", "3");
                        }
                    }
                }
                return TRUE;

            case IDM_QOS_ECOQOS:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessQos("ECO", dwProcessId);
                        SetProcessPriority(dwProcessId, 0);

                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "HighQos") != nullptr) {
                            RegKeyDelete(HKEY_CURRENT_USER, registryKey, "HighQos");
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "EcoQos", "1");
                        }
                    }
                }
                return TRUE;

            case IDM_QOS_HIGHQOS:
                {
                    if (index != LB_ERR)
                    {
                        SetProcessQos("HIGH", dwProcessId);

                        if (GetProcessPriority(dwProcessId) == 0) {
                            SetProcessPriority(dwProcessId, 2);
                        }

                        if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "EcoQos") != nullptr) {
                            RegKeyDelete(HKEY_CURRENT_USER, registryKey, "EcoQos");
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "HighQos", "1");
                        }
                    }
                }
                return TRUE;

            case IDM_SUSPEND_RESUME:
                {
                    if (index != LB_ERR)
                    {
                        EnablePrivilege(dwProcessId, SE_DEBUG_NAME);
                        if(IsSuspended(dwProcessId)) {
                            _SR("RESUME", dwProcessId);
                        } else {
                            _SR("SUSPEND", dwProcessId);
                        }
                    }
                }
                return TRUE;

            case IDM_CPU_PRIORITY_BOOST_SAVE:
                {
                    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityBoost") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "PriorityBoost");
                        break;
                    }

                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessId);
                    BOOL pDisablePriorityBoost;
                    if (GetProcessPriorityBoost(hProcess, &pDisablePriorityBoost)) {
                        if (pDisablePriorityBoost == FALSE) {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityBoost", "1");
                        } else {
                            RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityBoost", "0");
                        }
                    }
                }
                return TRUE;

            case IDM_QOS_SAVE:
                {
                    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "EcoQos") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "EcoQos");
                        break;
                    } else if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "HighQos") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "HighQos");
                        break;
                    }

                    ULONG Qos;
                    GetProcessQos(dwProcessId, Qos);

                    if (Qos == 0) {
                        RegKeySet(HKEY_CURRENT_USER, registryKey, "HighQos", "1");
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "EcoQos");
                    } else if (Qos == PROCESS_POWER_THROTTLING_EXECUTION_SPEED) {
                        RegKeySet(HKEY_CURRENT_USER, registryKey, "EcoQos", "1");
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "HighQos");
                    }
                }
                return TRUE;

            case IDM_CPU_PRIORITY_SAVE:
                {
                    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "PriorityClass") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "PriorityClass");
                        break;
                    }

                    DWORD Priority = GetProcessPriority(dwProcessId);
                    char strPriority[2];
                    sprintf_s(strPriority, sizeof(strPriority), "%lu", Priority);
                    RegKeySet(HKEY_CURRENT_USER, registryKey, "PriorityClass", strPriority);
                }
                return TRUE;

            case IDM_MEMORY_PRIORITY_SAVE:
                {
                    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "MemoryPriority") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "MemoryPriority");
                        break;
                    }

                    ULONG Priority;
                    GetPriority("MEM", dwProcessId, Priority);
                    char strPriority[2];
                    sprintf_s(strPriority, sizeof(strPriority), "%lu", Priority);
                    RegKeySet(HKEY_CURRENT_USER, registryKey, "MemoryPriority", strPriority);
                }
                return TRUE;

            case IDM_IO_PRIORITY_SAVE:
                {
                    if (RegKeyQuery(HKEY_CURRENT_USER, registryKey, "IoPriority") != nullptr) {
                        RegKeyDelete(HKEY_CURRENT_USER, registryKey, "IoPriority");
                        break;
                    }
                    ULONG Priority;
                    GetPriority("IO", dwProcessId, Priority);
                    char strPriority[2];
                    sprintf_s(strPriority, sizeof(strPriority), "%lu", Priority);
                    RegKeySet(HKEY_CURRENT_USER, registryKey, "IoPriority", strPriority);
                }
                return TRUE;

            case IDM_EXIT:
                {
                    PostMessage(hwndDlg, WM_CLOSE, 0, 0);
                }
                return TRUE;

            case IDM_RESET:
                {
                    RegKeyDelete(HKEY_CURRENT_USER, registryKey);
                }
                return TRUE;

            } //end Switch wParam
            return TRUE;
        }
        return TRUE; //end WM_COMMAND

        case WM_CLOSE:
        {
        //comprobar valores
        UINT state = IsDlgButtonChecked(hwndDlg, _START);
            if (state == BST_CHECKED)
            {
                char path[MAX_PATH];
                if (GetModuleFileName(NULL, path, MAX_PATH) != 0)
                    {
                        RegKeySet(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "SetProcess", path);
                        RegKeySet(HKEY_CURRENT_USER, "Software\\SetProcess", "Start", "1");
                    }
            }
            else
            {
                RegKeyDelete(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "SetProcess");
                RegKeyDelete(HKEY_CURRENT_USER, "Software\\SetProcess", "Start");
            }
            //exit
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DestroyWindow(hwndDlg);
        }
        return TRUE;
    }
    return FALSE; //end Switch
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    hInst=hInstance;
    InitCommonControls();

    // Configurar el WinEventHook
    g_hookID = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND,
        EVENT_OBJECT_CREATE,
        NULL,
        WinEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS | WINEVENT_SKIPOWNTHREAD
    );

    // Cargar el acelerador
    HWND hwndR = CreateDialog(hInst, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgMain);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {

        if(msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
        {
            SetPriorityClass(GetCurrentProcess(), PROCESS_MODE_BACKGROUND_BEGIN);
            ShowWindow(hwndR, SW_HIDE);
            _drain();
        }
       if (!IsDialogMessage(hwndR, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!IsWindow(hwndR)) {
            UnhookWinEvent(g_hookID);
            DestroyWindow(hwndR);
            DestroyWindow(hwndDlg);
        break; // Salir del bucle si el HWND ya no existe
        }
    }
    return TRUE;
}
