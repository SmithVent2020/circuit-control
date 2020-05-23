/**
 * SmithVent
 * Ventilator Control Circuit
 * 
 * The circuit:
 * Inputs 
 *    analog  -- oxygen sensor, and 2 flow sensors (one for each line)
 *    digital -- 3 pressure sensors (inspiratory, expiratory, and reservoir), LCD
 * Outputs: 
 *    analog  -- none
 *    digital -- SV1 (air), SV2 (O2), SV3 (reservoir), SV4 (exp)
 */

//------------------Libraries------------------
#include "Constants.h"
#include "Pressure.h"
#include "Flow.h"
#include "Oxygen.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "AlarmManager.h"


//-----------------------------------------------INITIALIZE VARIABLES---------------------------------------------------
// Cycle parameters
unsigned long cycleCount = 0;
unsigned long tCycleTimer;     // Absolute time (s) at start of each breathing cycle
unsigned long tInsp;           // Calculated time (s) since tCycleTimer for end of IN_STATE
unsigned long tHoldInsp;       // Calculated time (s) since tCycleTimer for end of HOLD_IN_STATE
unsigned long tExp;            // Calculated time (s) since tCycleTimer for end of EX_STATE
unsigned long tPeriod;         // Calculated time (s) since tCycleTimer for end of cycle
unsigned long tPeriodActual;   // Actual time (s) since tCycleTimer at end of cycle (for logging)
unsigned long tLoopTimer;      // Actual time (s) since start of each control loop iteration

// States
States state;
bool enteringState;
float tStateTimer;

// Flags
bool DEBUG;
bool ventilatorOn;
VentMode ventMode = VC_MODE;

// @TODO: Implement Display class
// Display display();

// Valves
Valve oxygenValve(SV1_CONTROL);
Valve airValve(SV2_CONTROL);
Valve expValve(SV4_CONTROL);
ProportionalValve propValve;

// Pressure
Pressure inspPressureReader(PRESSURE_INSP);
Pressure expPressureReader(PRESSURE_EXP);
Pressure reservoirPressureReader(PRESSURE_RESERVOIR);

// Flow
Flow flowInReader(FLOW_IN);
Flow flowOutReader(FLOW_OUT);

// Oxygen
Oxygen oxygenReader;

// Alarms
AlarmManager alarm;

// settings
// @TODO: update these values from UI inputs
struct vs_settings {
  int volume = 500;  // Tidal volume
  int bpm = 35;      // Respiratory rate
  float ie = 0.5;    // Inhale/exhale ratio
  float peak = 35;   // peak pressure (PiP)
  int o2 = 21;       // O2 concentration
  float sensitivity = 3;  // pressure sensitivity
} vc_settings;

// settings
// @TODO: update these values from UI inputs
struct ps_settings {
  int bpm = 35;           // Respiratory rate
  float cycleOff = 0.25;  // cycle off % (peak flow % at which to stop pumping air)
  float peak = 35;        // peak pressure (PiP) above peep
  float sensitivity = 3;  // pressure sensitivity
  int o2 = 21;            // O2 concentration
  int backup_bpm = 35;    // Backup BPM
  float backup_ie = 0.5;  // Backup IE ratio
} ps_settings;

//--------------Declare Functions--------------

// Helper function that returns time elapsed 
inline unsigned long now() { return millis() / 1000; }

// Set the current state in the state machine
void setState(States newState);

// Calculates the waveform parameters from the user inputs
void calculateWaveform();

// Check for errors and take appropriate action
void handleErrors();

// Helper function that gets all sensor readings
void readSensors();

// Helper function that returns time elapsed in sec
unsigned long timeElapsed(unsigned long startTimeSec);

// PS algorithm
void pressureSupport();

// VC algorithm
void volumeControl();


//-------------------Set Up--------------------
void setup() {
  Serial.begin(9600);   // open serial port for debugging

  // initialize pins with pinMode command
  pinMode(SV1_CONTROL.pin, OUTPUT);
  pinMode(SV2_CONTROL.pin, OUTPUT);
  pinMode(SV4_CONTROL.pin, OUTPUT);
  pinMode(SV3_CONTROL, OUTPUT);

  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(PRESSURE_RESERVOIR, INPUT);
  pinMode(PRESSURE_INSP, INPUT);
  pinMode(PRESSURE_EXP, INPUT);
  pinMode(O2_SENSOR, INPUT);
  pinMode(FLOW_IN, INPUT);
  pinMode(FLOW_OUT, INPUT);

  // @TODO: implement startup sequence on display
  // display.begin();
  tCycleTimer = now();
}


// Run forever
void loop() {
  // All States
  // @TODO: alarm maintenance
  // display.fetchValues() // @TODO: fetch new values from display
  calculateWaveform();
  readSensors();
  handleErrors();        // check thresholds against sensor values
  // display.update();   // @TODO: update display with sensor readings and flow graph

  // @TODO: implement OFF button functionality to UI with confirmation
  // if (display.turnedOff()) {
  //   setState(OFF_STATE);
  // }

  // @TODO: pull out common code
  // PS and VC currently have a lot of common code that can be re-used
  if (ventMode == PS_MODE) {
    // update ps_settings
    pressureSupport();
  } 
  else {
    // update vc_settings
    volumeControl();
  }
}

void pressureSupport() {
  // State machine
  switch (state) {
    case OFF_STATE: 
      // @TODO: implement a turnOffAll method in AlarmManager class to be used here
      // turn off ongoing alarms after confirmation
      // close all valves?
      airValve.close();   
      oxygenValve.close(); 
      propValve.endBreath();

      // keep expiratory valve open?
      expValve.open();
      break;

    case INSP_STATE:
      // inspiration starts
      if (enteringState) {
        enteringState = false;
        tPeriodActual = timeElapsed(tCycleTimer);  
        tCycleTimer = now();;  // the cycle begins at the start of inspiration

        // close valves
        airValve.close();   
        oxygenValve.close(); 
        expValve.close();

        // turn on PID for inspiratory valve (input = pressure, setpoint = 0)
        cycleCount++;
      }

      // compute PID based on linear pressure function 

      // check if we reached peak pressure
      if (inspPressureReader.peak() >= ps_settings.peak) {
        flowInReader.setPeakAndReset();
        setState(HOLD_INSP_STATE);
      }
      break;

    case HOLD_INSP_STATE:
      if (enteringState) {
        enteringState = false;
      }

      // still PID, setpoint should be peak constantly 

      if(flowInReader.get() < (CYCLE_OFF * flowInReader.peak())) {
        setState(EXP_STATE);
      }
      break;  

    case EXP_STATE:
      if (enteringState) {
        enteringState = false;
        // close prop valve and open all others
        propValve.endBreath();
        expValve.open();   
        airValve.open();

        // turn on PID for oxygen valve (beginBreath)
      }

      // check whether or not to go into PEEP_PAUSE_STATE
      // not sure if this check is correct here
      if (timeElapsed(tCycleTimer) > tExp) {
        setState(PEEP_PAUSE_STATE);
      }
      break;
      
    case PEEP_PAUSE_STATE:
      if (enteringState) {
        enteringState = false;
      }
      
      if(timeElapsed(tCycleTimer) > tExp + MIN_PEEP_PAUSE) {
        expPressureReader.setPeep();
        setState(HOLD_EXP_STATE);
      }
      break;

    case HOLD_EXP_STATE:
      if (enteringState) {
        enteringState = false;
      }
      
      // Check if patient triggers inhale
      // using peep here, but may need to use a lower pressure threshold
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep();

      if (patientTriggered) {
        inspPressureReader.setPeakAndReset();
        // @TODO: write peak, and PEEP to display
        setState(INSP_STATE);
      } 

      // Apnea check
      if (timeElapsed(tCycleTimer) > APNEA_BACKUP) {
          alarm.activateAlarm(ALARM_APNEA);
          setState(INSP_STATE);
          ventMode = VC_MODE;
      }
    }
  }

void volumeControl() {
  // State machine
  switch (state) {
    case OFF_STATE: 
      // turn off ongoing alarms after confirmation
      // close valves
      oxygenValve.close();
      airValve.close();
      propValve.endBreath();

      // keep expiratory valve open?
      expValve.open();
      break;

    case INSP_STATE:
      if (enteringState) {
        enteringState = false;
        tPeriodActual = timeElapsed(tCycleTimer);  
        tCycleTimer = now();;  // the cycle begins at the start of inspiration

        // close valves
        airValve.close();   
        oxygenValve.close(); 
        expValve.close();

        // move insp valve using set VT and calculated insp time
        propValve.beginBreath(tInsp, vc_settings.volume);
        cycleCount++; 
      }

      // keep opening valve until tInsp is elapsed
      propValve.maintainBreath();

      if (timeElapsed(tCycleTimer) > tInsp) {
        setState(HOLD_INSP_STATE);
      }
      break;

    case HOLD_INSP_STATE:
      if (enteringState) {
        enteringState = false;

        // close prop valve and open air/oxygen
        propValve.endBreath();
        airValve.open();

        // turn on PID control for oxygen 
        // (input: measured FiO2, output: duration to keep open)
        // so we have to keep track of an O2 timer
        // timer since PID was first started, DURATION constant 
        // do we still compute consistentally? => duration should belong to Oxygen
      }
      if (timeElapsed(tCycleTimer) > tHoldInsp) {
        inspPressureReader.setPlateau();
        setState(EXP_STATE);
      }
      break;  

    case EXP_STATE:
      if (enteringState) {
        enteringState = false;
        expValve.open();   
      }

      // timer for when this starts to use for tidal volume

      // check whether or not to go into PEEP_PAUSE_STATE
      if (timeElapsed(tCycleTimer) > tExp) {
        setState(PEEP_PAUSE_STATE);
      }
      break;
      
    case PEEP_PAUSE_STATE:
      if (enteringState) {
        enteringState = false;
      }
      
      if(timeElapsed(tCycleTimer) > tExp + MIN_PEEP_PAUSE) {
        // record the peep as the current pressure
        inspPressureReader.peep();
        setState(HOLD_EXP_STATE);
      }
      break;

    case HOLD_EXP_STATE:
      if (enteringState) {
        enteringState = false;
      }
      
      // Check if patient triggers inhale
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - SENSITIVITY_PRESSURE;

      if (patientTriggered || timeElapsed(tCycleTimer) > tPeriod) {
        if (!patientTriggered) expPressureReader.setPeep();  // set peep again if time triggered
        inspPressureReader.setPeakAndReset();
        // @TODO: write PiP, PEEP and Pplat to display
        setState(INSP_STATE);
      }
  }
}

void setState(States newState) {
  enteringState = true;
  state = newState;
  tStateTimer = now();
}

void calculateWaveform() {
  tPeriod = 60.0 / vc_settings.bpm;  // seconds in each breathing cycle period
  tHoldInsp = tPeriod / (1 + vc_settings.ie); // time to hold inspiration
  tInsp = tHoldInsp - HOLD_INSP_DURATION; 
  tExp = min(tHoldInsp + MAX_EXP_DURATION, tPeriod - MIN_PEEP_PAUSE);
}

unsigned long timeElapsed(unsigned long startTimeSec) {
  // convert ms to sec and calculate duration
  unsigned long now = millis()/1000;
  return now - startTimeSec;
}

void readSensors() { 
  // read pressure sensors
  inspPressureReader.read();
  expPressureReader.read();
  reservoirPressureReader.read();

  // read oxygen sensors
  oxygenReader.read();

  // read flow sensors
  flowInReader.read();
  flowOutReader.read();
}

void handleErrors() {
  // add sensor failure alarm
  // reading should return Nan or something similar.. test
  // do you get zero volts if sensor is disconnected?

  // Pressure alarms
  if (inspPressureReader.get() >= MAX_PRESSURE) {
    alarm.activateAlarm(ALARM_INSP_HIGH);
    setState(EXP_STATE); // start exhaling to relieve pressure?
  }
 
  if (enteringState && state == HOLD_INSP_STATE) {
    // check bad plateau pressure
    if(inspPressureReader.plateau() > MAX_PLATEAU_PRESSURE) {
      // @TODO: add high plateau alarm   
      // alarm.activateAlarm(ALARM_HIGH_PLATEAU);
    }

    // low inspiratory pressure
    // double check about low inspiratory p alarm
    if(inspPressureReader.plateau() < MIN_PLATEAU_PRESSURE) {
      alarm.activateAlarm(ALARM_INSP_LOW);
    }
  }

  // Check if desired volume was reached
  else if (enteringState && state == EXP_STATE) {
    float flowRate = flowInReader.get();
    float vtActual = flowRate * (timeElapsed(tCycleTimer)); // measured in mL (cc's)
    
    if (vtActual >= vc_settings.volume + ERROR_VOLUME) {
      alarm.activateAlarm(ALARM_TIDAL_HIGH);
    } else if (vtActual + ERROR_VOLUME <= vc_settings.volume) {
      alarm.activateAlarm(ALARM_TIDAL_LOW);
    }
  }

  // @TODO: detect various failures (O2, air, pressure sensors, power..) 
}
