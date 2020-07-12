/**
 * Constants.h
 * Defines all the global constants used in `circuit-control.ino`
 */

#ifndef Constants_h
#define Constants_h

#include <Arduino.h>

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
const unsigned long LOOP_PERIOD = 30;          // The period of the control loop
const unsigned long HOLD_INSP_DURATION = 500;  // Interval to pause after inhalation
const unsigned long MIN_PEEP_PAUSE = 50;       // Interval to pause after exhalation / before watching for an assisted inhalation
const unsigned long MAX_EXP_DURATION = 1000;   // Maximum exhale duration (ms)
const unsigned long SILENCE_DURATION = 120000; // silence duration (ms) - 2 min

// Graph settings
const int GRAPH_MIN = 0;
const int GRAPH_MAX = 255;
const int FLOW_RANGE_MIN = -60;
const int FLOW_RANGE_MAX = 100;
const int PRESSURE_RANGE_MIN = -7; 
const int PRESSURE_RANGE_MAX = 70;


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
const int BPM = 20;              // respiratory rate
const int O2 = 21;               // O2 concentration  
const int IE_INSP = 1;           // inspiratory portion in IE ratio
const float IE_EXP = 2;          // expiratory portion in IE ratio
const float TIDAL_VOLUME = 400;  // volume in mL (cc's)

// Safety settings
const float MAX_PRESSURE = 40.0;         // Trigger high pressure alarm 
const float SENSITIVITY = 0.5;           // acceptable margin of error in pressure (in cmH2O)
const float EXP_TIME_SENSITIVITY = 400;  // in ms, the "wiggle room" we allow for the patient to exhale 80% of air in VC mode 
const float INSP_TIME_SENSITIVITY = 400; // in ms, the "wiggle room" we allow for patient to inhale correct tidal volume

// ---------------------
// PID Control Values
// ---------------------

// PID gains for inspiratory valve (initial tuning done during prototype testing)
const float VKP = 0.225; // proportional constant
const float VKI = 1.08;  // integral constant
const float VKD = 0;     // derivative constant

// PID controller ranges 
const double OUTPUT_MAX = 120;
const double OUTPUT_MIN = 40;
const int SAMPLE_TIME = 50;

// initial value for valve to open according to previous tests (close to desired)
const int DEFAULT_VALVE_POSITION;

// --------------------------
// Generally-useful Constants
// --------------------------

// One atmosphere (standard pressure) expressed in cmH2O
const float ATM_IN_CMH2O = 1033.23;

// Conversion factor: Liters per Min to CCs per Ms.  Note that this is a
// conversion from actual liters, not a conversion from SLPM, as such a
// conversion involves pressure.
const float LPM_TO_CC_PER_MS =  1000.0 / 60000.0;
const float CC_PER_MS_TO_LPM = 60000.0 / 1000.0;

// Alarm thresholds
const float TIDAL_VOLUME_SENSITVITY   = 10;//% OF SET TIDAL VOLUME
const float PEEP_SENSITIVITY          = 3; //cmH2O
const float INSP_PRESSURE_SENSITIVITY = 7; //cmH2O
const float PPLAT_MAX                 = 33;//cmH2O

// Floating point NaN
static const float NaNf = 0.0f/0.0f;

#endif
