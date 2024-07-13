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

* Presione ESC para ocultar la gui y dejar el trayicon

* Las configuraciones se guardan en `HKCU\Software\SetProcess`

* Reset Config eliminará la entrada de registro del proceso (si hay otros procesos hijos con el mismo nombre se le aplicará la configuración), pronto emplearé la configuración a traves de command line

* No aplique Cpu Sets, Ideal Processor o Affinity de manera contigua, trate de usar una de las configuraciones para cada proceso

* Para revertir los cambios de CpuSets no marque ningun núcleo (tambien borrará la configuración del registro), si no marca ningún núcleo o más de dos en IdealProcessor no se aplicará la configuración (tambien borrará la configuración del registro), si no marca ningun nucleo en Affinity no se aplicará la configuración, si marca todos se borrara la configuración del registro

* Si quiere buscar de forma más rápida el proceso deseado, escriba en el ListBox no saldrá un cursor pero si escribe correctamente se seleccionará

* Registra un WinEventHook que detecta los eventos `EVENT_SYSTEM_FOREGROUND | EVENT_OBJECT_CREATE` si los Ids de cada objeto no corresponden a una ventana o un objeto principal, entonces se retornará de la función, esto hace que el programa no requiera un intervalo de sondeo para verificar si un proceso se ha iniciado o no

# REFERENCIAS
[1](https://github.com/manciuszz/Thread-Pilot)
[2](https://github.com/spddl/Hook)
