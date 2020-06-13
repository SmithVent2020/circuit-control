# SmithVent: Pneuamtic, Cost-effective Ventilator
## Designed for Covid-19 patients through the CoVent Challenge

**This code is provided for reference use only.** Our goal is to showcase the functionality of the Volume Control mode, which was partially tested, and its auxiliary specification in terms of valve behavior and UI interaction. Pressure Support, which is meant to assist patient as they breath on their own, is largely written but has not been tested. 

Caution: The enclsoed control software is specific to the SmithVent system and is not to be used as is. This is essential to ensure patient safety. Futher testing and software validation is needed to in order to correct any faulty behavior and reach a fully functional state. 

The SmithVent control algorithm is licensed under the MIT License. 

### Nextion Library Details
The original [Nextion Library](https://github.com/itead/ITEADLIB_Arduino_Nextion) was used, with some changes to the following files:
- [NexConfig.h](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexConfig.h)
- [NexHardware.h](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexHardware.h)
- [NexHardware.cpp](https://github.com/SmithVent2020/circuit-control/blob/master/Nextion/NexHardware.cpp)

These changes were mostly to work with circuit configuration. For example, we use Serial1 as opposed to the default Serial2

### PID Library Details

***************************************************************
* Arduino PID Library - Version 1.2.1
* by Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
*
* This Library is licensed under the MIT License
***************************************************************

 - For an ultra-detailed explanation of why the code is the way it is, please visit: 
   http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

 - For function documentation see:  http://playground.arduino.cc/Code/PIDLibrary
