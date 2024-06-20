# SetProcess
[![Total Downloads](https://img.shields.io/github/downloads/LuSlower/SetProcess/total.svg)](https://github.com/LuSlower/SetProcess/releases/latest) [![PayPal Badge](https://img.shields.io/badge/PayPal-003087?logo=paypal&logoColor=fff&style=flat)](https://paypal.me/eldontweaks) 

pequeña CLI para control avanzado de procesos

```
use: sp.exe <Argument> <ImageName> <Parameter>

  -c   <IM> <core0,core2...> <-r>     |    CpuSets
  -idp <IM> <core> <-r>               |    Ideal Processor
  -a   <IM> <core0,core2...> <-r>     |    Affinity
  -au  <IM> <0-1> <-r>                |    Affinity Update Mode
  -p   <IM> <0-5> <-r>                |    CPU Priority
  -pb  <IM> <-r>                      |    CPU Priority Boost
  -mp  <IM> <1-5> <-r>                |    Memory Priority
  -iop <IM> <0-3> <-r>                |    I/O Priority
  -eq  <IM> <-r>                      |    EcoQoS (Efficiency Mode)
  -hq  <IM> <-r>                      |    HighQoS (High Performance Mode)
  -s   <IM> <-r>                      |    Suspend
  -r   <IM> <-r>                      |    Resume
```

> [!important]
> Use esta herramienta si realmente le es útil o sabe lo que hace
>
> disfrutelo
