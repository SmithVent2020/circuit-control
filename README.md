# SmithVent: Pneuamtic, Cost-effective Ventilator Design
Designed by a group of Smith College alumni and friends for the CoVent Challenge

## Master System Design
![img](images/system-diagram.png)

## Working Prototype
![img](images/screen.png)

### Defining values
```
Ti = inspiration time
Te = expiration time
p = time period for a full breath cycle
bpm = breaths per minute
IE = ratio of inspiration to expiration
VT = tidal volume
V̇ = flow rate 
```

#### IE/BPM and timings
```
p = 1/BPM = Ti + Te (min)
IE = Ti/Te
```

#### Desired Inspiratory Flow (in L/min)
![equation](http://www.sciweavers.org/tex2img.php?eq=%5Cdot%7BV%7D%3D%5Cfrac%7BV_T%7D%7BTi%7D%3D%20%5Cfrac%7BV_T%2ABPM%2A%281%2BIE%29%7D%7BIE%20%2A%201000%7D&bc=White&fc=Black&im=jpg&fs=12&ff=arev&edit=0)

#### PID

### Disclaimer 
**This code is provided for reference use only.** Our goal is to showcase the functionality of the Volume Control mode, which was partially tested, and its auxiliary specification in terms of valve behavior and UI interaction. Pressure Support, which is meant to assist patient as they breath on their own, is largely written but has not been tested. 

**Caution**: The enclsoed control software is specific to the SmithVent system and is not to be used as is. This is essential to ensure patient safety. Futher testing and software validation is needed to in order to correct any faulty behavior and reach a fully functional state. 

The SmithVent control algorithm is licensed under the MIT License. 

### Nextion Library Details
The original [Nextion Library](https://github.com/itead/ITEADLIB_Arduino_Nextion) was used, with some changes to the following files:
- [NexConfig.h](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexConfig.h)
- [NexHardware.h](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexHardware.h)
- [NexHardware.cpp](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexHardware.cpp)

These changes were made mostly to allow the library to work with our circuit configuration. For example, we use Serial1 as opposed to the library's default Serial2.

### PID Library Details

```
***************************************************************
* Arduino PID Library - Version 1.2.1
* by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
*
* This Library is licensed under the MIT License
***************************************************************
 ```
 - For an ultra-detailed explanation of why the code is the way it is, please visit: 
   http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

 - For function documentation see:  http://playground.arduino.cc/Code/PIDLibrary

