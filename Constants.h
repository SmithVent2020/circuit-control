/**
 * Constants.h
 * Defines all the global constants used in `circuit-control.ino`
 */

#ifndef Constants_h
#define Constants_h

#include <arduino.h>

// States
enum States {
  DEBUG_STATE,
  INSP_STATE,
  INSP_SUSTAIN_STATE,
  HOLD_INSP_STATE,
  EXP_STATE,
  PEEP_PAUSE_STATE,
  HOLD_EXP_STATE,
  OFF_STATE
};

// Ventilator modes
enum VentMode {
  VC_MODE, // 0
  PS_MODE  // 1
};

// Timing interval settings (all values in milliseconds)
const unsigned long LOOP_PERIOD = 30;         // The period of the control loop
const unsigned long HOLD_INSP_DURATION = 500; // Interval to pause after inhalation
const unsigned long MIN_PEEP_PAUSE = 50;      // Interval to pause after exhalation / before watching for an assisted inhalation
const unsigned long MAX_EXP_DURATION = 1000;  // Maximum exhale duration (ms)

// ---------------------
// PINS
// ---------------------

// Alarm related pins
const int BUZZER     = 2;
const int YELLOW_LED = 3;
const int RED_LED    = 4;

// Valves pins
const int SV1_CONTROL = 22; // relay pin that controls SV1 (air)
const int SV2_CONTROL = 24; // relay pin that controls SV1 (O2)
const int SV4_CONTROL = 26; // relay pin that controls SV4 (exp)
const int SV3_CONTROL = 5;  // proportional valve pin SV3 (insp)

// Flow sensors pins
const int FLOW_INSP = A0;
const int FLOW_EXP  = A1;

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
const float TIDAL_VOLUME = 400;            // in mL (cc's)
const float CYCLE_OFF = 0.25;              // % peak flow at which to switch to EXP in PS_MODE
const unsigned long APNEA_BACKUP = 15000;  // in milliseconds
const unsigned long RISE_TIME = 4000;      // max time (ms) it takes to reach PiP

// Safety settings
const float MAX_PRESSURE = 40.0;        // Trigger high pressure alarm
const float MAX_PLATEAU_PRESSURE = 30;  // Trigger Pplat (cmH2O) alarm
const float MIN_PLATEAU_PRESSURE = 5.0; // Trigger low insp pressure alarm
const float SENSITIVITY_PRESSURE = 2; // acceptable margin of error in inspiratory pressure (in cmH2O)
const float EXP_TIME_SENSITIVITY = 400; //ms the time "wiggle room" we allow for the patient to exhail 80% of air in VC mode 


// ---------------------
// PID Control Values
// ---------------------

// PID gains for inspiratory valve (to be tuned after testing)
const float VKP = 0.225; //proportional constant
const float VKI = 1.08;  //integral constant
const float VKD = 0;     //derivative constant





// --------------------------
// Generally-useful Constants
// --------------------------

// One atmosphere (standard pressure) expressed in cmH2O
const float ATM_IN_CMH2O = 1033.23;

// Conversion factor: Liters per Min to CCs per Ms.  Note that this is a
// conversion from actual liters, not a conversion from SLPM, as such a
// conversion involves pressure.
const float LPM_TO_CC_PER_MS =  1000.0 / 60000.0;
const float CC_PER_MS_TO_LPM = 60000.0/1000.0;

#endif
