# SetProcess
[![Total Downloads](https://img.shields.io/github/downloads/LuSlower/SetProcess/total.svg)](https://github.com/LuSlower/SetProcess/releases/latest) [![PayPal Badge](https://img.shields.io/badge/PayPal-003087?logo=paypal&logoColor=fff&style=flat)](https://paypal.me/eldontweaks) 

Herramienta hecha para un control avanzado de procesos

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

![image](https://github.com/user-attachments/assets/9c9381f4-ea8b-4ac9-ba8d-c8cd478f920c)

* Las configuraciones se guardan en `HKCU\Software\SetProcess`

* Reset Config eliminará la entrada de registro del proceso (si hay otros procesos hijos con el mismo nombre se le aplicará la configuración), pronto emplearé la configuración a traves de command line

* No aplique Cpu Sets, Ideal Processor o Affinity de manera contigua, trate de usar una de las configuraciones para cada proceso
