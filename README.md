# SetProcess
[![Total Downloads](https://img.shields.io/github/downloads/LuSlower/SetProcess/total.svg)](https://github.com/LuSlower/SetProcess/releases/latest) [![PayPal Badge](https://img.shields.io/badge/PayPal-003087?logo=paypal&logoColor=fff&style=flat)](https://paypal.me/eldontweaks) 

Tool made for advanced process control

## CLI

```
use: sp.exe <Argument> <ImageName or PID> <Argument>

  -c   <IM/PID> <Cores> <-r>              |    CpuSets
  -idp <IM/PID> <Core> <-r>               |    Ideal Processor
  -a   <IM/PID> <Cores> <-r>              |    Affinity
  -p   <IM/PID> <0-5> <-r>                |    CPU Priority
  -pb  <IM/PID> <-r>                      |    CPU Priority Boost
  -mp  <IM/PID> <1-5> <-r>                |    Memory Priority
  -iop <IM/PID> <0-3> <-r>                |    I/O Priority
  -eq  <IM/PID> <-r>                      |    EcoQoS (Efficiency Mode)
  -hq  <IM/PID> <-r>                      |    HighQoS (High Performance Mode)
  -s   <IM/PID> <-r>                      |    Suspend
  -r   <IM/PID> <-r>                      |    Resume
```
## GUI

![image](https://github.com/user-attachments/assets/8aa378cd-b2cf-4e85-8c2b-f64ad73f69bb)

* Press ESC to hide the gui and leave the trayicon

* Settings are saved in `HKCU\Software\SetProcess`

* Reset Config will remove the process registry entry (if there are other child processes with the same name the configuration will be applied to it), I will soon use the configuration via command line

* Do not apply Cpu Sets, Ideal Processor or Affinity back to back, try to use one of the settings for each process

* To revert the CpuSets changes, do not check any core (it will also delete the registry configuration), if you do not check any core or more than two in IdealProcessor the configuration will not be applied (it will also delete the registry configuration), if you do not check any core In Affinity the configuration will not be applied, if you check all the configuration will be deleted from the registry

* If you want to search for the desired process more quickly, write in the ListBox, a cursor will not appear but if you write correctly it will be selected

* Registers a WinEventHook that listens for events `EVENT_SYSTEM_FOREGROUND | EVENT_OBJECT_CREATE` if the Ids of each object do not correspond to a window or a main object, then it will be returned from the function, this makes the program not require a polling interval to check if a process has started or not

# REF
[1](https://github.com/spddl/Hook)
