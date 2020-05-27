/**
 * Constants.h
 * Defines all the global constants used in `circuit-control.ino`
 */

#ifndef Constants_h
#define Constants_h

// States
enum States {
  DEBUG_STATE,       // 0
  INSP_STATE,        // 1
  HOLD_INSP_STATE,   // 2
  EXP_STATE,         // 3
  PEEP_PAUSE_STATE,  // 4
  HOLD_EXP_STATE,    // 5
  OFF_STATE          // 8
};

// Ventilator modes
enum VentMode {
  VC_MODE, // 0
  PS_MODE  // 1
};

struct ValveInfo {
  int pin;
  bool normallyOpen;
};

// Timing Settings
const float LOOP_PERIOD = 0.03;       // The period (s) of the control loop
const float HOLD_INSP_DURATION = 0.1;   // Duration (s) to pause after inhalation
const float MIN_PEEP_PAUSE = 0.05;    // Time (s) to pause after exhalation / before watching for an assisted inhalation
const float MAX_EXP_DURATION = 1.00;  // Maximum exhale duration (s)

// ---------------------
// PINS
// ---------------------

// Alarm related pins
const int BUZZER     = 2;
const int YELLOW_LED = 3;
const int RED_LED    = 4;

// Valves pins
const ValveInfo SV1_CONTROL = { 22, false }; // relay pin that controls SV1 (air)
const ValveInfo SV2_CONTROL = { 24, false }; // relay pin that controls SV1 (O2)
const ValveInfo SV4_CONTROL = { 26, true };  // relay pin that controls SV4 (exp)
const int SV3_CONTROL = 5;  // proportional valve pin SV3 (insp)

// Flow sensors pins
const int FLOW_IN  = A0;
const int FLOW_OUT = A1;

// Pressure sensor pins
const int PRESSURE_RESERVOIR = A5; 
const int PRESSURE_INSP      = A6;
const int PRESSURE_EXP       = A7; 

// Oxygen sensor pin
const int O2_SENSOR = A8;

// ---------------------
// Settings
// ---------------------

// Patient Values
const int BPM_MIN = 10;         // respiratory rate
const int BPM_MAX = 35;
const int BPM_RES = 1;          // resolution (increments of 1)
const int O2_MIN = 21;          
const int O2_MAX = 100;         
const int O2_RES = 1;           // resolution (increments of 1)   
const float IE_MIN = 1;
const float IE_MAX = 4;
const float IE_RES = 0.1;       // resolution (increments of 0.1)  
const float ERROR_VOLUME = 80;  // acceptable margin of error in tidal volume (should be 20% of VT)
const float TIDAL_VOLUME = 400; // in mL (cc's)  
const float APNEA_BACKUP = 15;  // in seconds
const float CYCLE_OFF = 025;    // % peak flow at which to switch to EXP in PS_MODE
const float RISE_TIME = 4;      // max time it takes to reach PiP  

// Safety settings
const float MAX_PRESSURE = 40.0;        // Trigger high pressure alarm
const float MAX_PLATEAU_PRESSURE = 30;  // Trigger Pplat (cmH2O) alarm 
const float MIN_PLATEAU_PRESSURE = 5.0; // Trigger low insp pressure alarm
const float SENSITIVITY_PRESSURE = 0.5; // acceptable margin of error in inspiratory pressure (in cmH2O)


// ---------------------
// PID Control Values
// ---------------------

// PID gains for inspiratory valve (to be tuned after testing)
const float VKP = 2.0;
const float VKI = 5.0;
const float VKD = 1.0;

// PID gains for oxygen valve (to be tuned)
const float OKP = 1.0;
const float OKI = 2.0;
const float OKD = 3.0;


#endif
