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
unsigned long cycleCount = 0;

// Time parameters
unsigned long tInsp;           // Calculated desired time (s) since cycleSecTimer for end of INSP_STATE
unsigned long tHoldInsp;       // Calculated desired time (s) since cycleSecTimer for end of HOLD_INSP_STATE
unsigned long tExp;            // Calculated desired time (s) since cycleSecTimer for end of EXP_STATE
unsigned long tPeriod;         // Calculated desired time (s) since cycleSecTimer for end of cycle

// Timers (start times):
unsigned long cycleSecTimer;     // Measured time (s) at start of each breathing cycle
unsigned long inspStartTimer;    // Measured time (ms) at which inspiration started
unsigned long expStartTimer;     // Measured time (ms) at which expiration (including exp hold & peep pause) started

// Timer results (elapsed times):
unsigned long cycleSecElapsed;   // Measured time (s) since cycleSecTimer at end of cycle (for logging)
unsigned long inspMsElapsed;     // Measured time (ms) since inspStartTimer at end of inspiration
unsigned long expMsElapsed;      // Measured time (ms) since expStartTimer at end of expiration


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

// Volume Control settings type
// @TODO: update these values from UI inputs
struct vc_settings_t {
  int   volume      = 500;  // Tidal volume
  int   bpm         = 35;   // Respiratory rate
  float ie          = 0.5;  // Inhale/exhale ratio
  float peak        = 35;   // peak pressure (PiP)
  int   o2          = 21;   // O2 concentration
  float sensitivity = 3;    // pressure sensitivity
};

// Global Volume Control settings
vc_settings_t vc_settings;

// Pressure support settings type.
// @TODO: update these values from UI inputs
struct ps_settings_t {
  int   bpm         = 35;   // Respiratory rate
  float cycleOff    = 0.25; // cycle off % (peak flow % at which to stop pumping air)
  float peak        = 35;   // peak pressure (PiP) above peep
  float sensitivity = 3;    // pressure sensitivity
  int   o2          = 21;   // O2 concentration
  int   backup_bpm  = 35;   // Backup BPM
  float backup_ie   = 0.5;  // Backup IE ratio
};

// Global Pressure Support settings
ps_settings_t ps_settings;

//--------------Declare Functions--------------

// Helper function that returns seconds elapsed since system startup
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
void pressureSupportStateMachine();

// VC algorithm
void volumeControlStateMachine();


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
  cycleSecTimer = now();
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

  if (ventMode == PS_MODE) {
    // Run pressure support mode
    pressureSupportStateMachine();
  }
  else {
    // Run volume control mode
    volumeControlStateMachine();
  }
}

//////////////////////////////////////////////////////////////////////////////////////
// STATE MACHINE
//////////////////////////////////////////////////////////////////////////////////////

// The state machine should be moved into its own .cpp file.  However, doing
// so would involve making sure that a header exists with all of the global
// variables declared.  Doing that is too much refactoring for now.

// States
States state;

void setState(States newState) {
    state = newState;
}

void beginOff() {
  // @TODO: implement a turnOffAll method in AlarmManager class to be used here
  // turn off ongoing alarms after confirmation
  // close all valves?
  airValve.close();
  oxygenValve.close();
  propValve.endBreath();

  // keep expiratory valve open?
  expValve.open();
}

void beginInspiration() {
  cycleSecElapsed = timeElapsed(cycleSecTimer);
  cycleSecTimer = now();;  // the cycle begins at the start of inspiration

  // End expiratory cycle timer and start the inpiration timer
  expMsElapsed = millis() - expStartTimer;
  inspStartTimer = millis();

  // close valves
  airValve.close();
  oxygenValve.close();
  expValve.close();

  // @TODO: This will change based on recent information.
  // move insp valve using set VT and calculated insp time
  propValve.beginBreath(tInsp, vc_settings.volume);

  // turn on PID for inspiratory valve (input = pressure, setpoint = 0)
  cycleCount++;
}

void beginHoldInspiration() {
  // Volume control only. Not used for pressure support mode.

  // close prop valve and open air/oxygen
  propValve.endBreath();
  airValve.open();

  // turn on PID control for oxygen
  // (input: measured FiO2, output: duration to keep open)
  // so we have to keep track of an O2 timer
  // timer since PID was first started, DURATION constant
  // do we still compute consistentally? => duration should belong to Oxygen
}

// Perform these actions when transitioning from inspiration to any of the
// states in the expiratory cycle (EXP_STATE, HOLD_EXP_STATE, or PEEP_PAUSE).
void beginExpiratoryCycle() {
  // End inspiration timer and start the epiratory cycle timer
  inspMsElapsed = millis() - inspStartTimer;
  expStartTimer = millis();
}

void beginExpiration() {
  propValve.endBreath();
  expValve.open();
  // @TODO in main loop: turn on PID for oxygen valve (beginBreath)
}

void beginPeepPause() {
  // Nothing to do when entering peep pause state
}

void beginHoldExpiration() {
  // Nothing to do when entering hold expiration state
}

void pressureSupportStateMachine() {
  switch (state) {
    case OFF_STATE:
      // @TODO How do we transition out of the OFF_STATE?
      bool onButton = /* isOnButtonPressed(); */ false;
      if (onButton) {
        setState(INSP_STATE);
        beginInspiration();  // close valves, etc.
      }
      break;

    case INSP_STATE:
      // compute PID based on linear pressure function

      // check if we reached peak pressure
      if (inspPressureReader.peak() >= ps_settings.peak) {
        flowInReader.setPeakAndReset();
        setState(EXP_STATE);
        beginExpiratoryCycle();
        beginExpiration();
      }
      break;

    case HOLD_INSP_STATE:
      // Shouldn't normally get here in pressure-support mode, but it's
      // possible that the user switched from volume control to pressure
      // support in the middle of a hold inspiration.  Just go on to the
      // expiration state.
      setState(EXP_STATE);
      beginExpiration();
      break;

    case EXP_STATE:
      if (timeElapsed(cycleSecTimer) > tExp) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      if(timeElapsed(cycleSecTimer) > tExp + MIN_PEEP_PAUSE) {
        expPressureReader.setPeep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE:
      // Check if patient triggers inhale
      // using peep here, but may need to use a lower pressure threshold
      if (expPressureReader.get() < expPressureReader.peep()) {
        inspPressureReader.setPeakAndReset();
        // @TODO: write peak, and PEEP to display
        setState(INSP_STATE);
        beginInspiration();
      }

      // Apnea check
      if (timeElapsed(cycleSecTimer) > APNEA_BACKUP) {
        alarm.activateAlarm(ALARM_APNEA);
        ventMode = VC_MODE;
        setState(INSP_STATE);
        beginInspiration();
      }
      break;

  } // End switch
}

void volumeControlStateMachine()
{
  switch (state) {
    case OFF_STATE:
      // @TODO How do we transition out of the OFF_STATE?
      bool onButton = /* isOnButtonPressed(); */ false;
      if (onButton) {
        setState(INSP_STATE);
        beginInspiration();  // close valves, etc.
      }
      break;

    case INSP_STATE:
      if (timeElapsed(cycleSecTimer) > tInsp) {
        setState(HOLD_INSP_STATE);
        beginHoldInspiration();
      }
      else {
        // keep opening valve until tInsp is elapsed
        propValve.maintainBreath();
        // Stay in INSP_STATE
      }
      break;

    case HOLD_INSP_STATE:
      if (timeElapsed(cycleSecTimer) > tHoldInsp) {
        inspPressureReader.setPlateau();
        setState(EXP_STATE);
        beginExpiratoryCycle();
        beginExpiration();
      }
      break;

    case EXP_STATE:
      if (timeElapsed(cycleSecTimer) > tExp) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      if(timeElapsed(cycleSecTimer) > tExp + MIN_PEEP_PAUSE) {
        // record the peep as the current pressure
        inspPressureReader.peep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE:
      // Check if patient triggers inhale
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - SENSITIVITY_PRESSURE;

      if (patientTriggered || timeElapsed(cycleSecTimer) > tPeriod) {
        if (!patientTriggered) expPressureReader.setPeep();  // set peep again if time triggered
        // @TODO: Move the next line into beginExpiration()
        inspPressureReader.setPeakAndReset();
        // @TODO: write PiP, PEEP and Pplat to display
        beginInspiration();
        setState(INSP_STATE);
      }
      break;
  } // End switch
}
