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
#include "O2management.h"
#include "AlarmManager.h"
#include "UI.h"

//-----------------------------------------------INITIALIZE VARIABLES---------------------------------------------------
unsigned long cycleCount = 0; // number of breaths (including current breath)
int inspVolume           = 0; // volume of inhaled air

// Target time parameters (in milliseconds). Calculated, not measured.
unsigned long targetInspEndTime;      // Desired interval since cycleSecTimer for end of INSP_STATE
unsigned long targetExpEndTime;       // Desired interval since cycleSecTimer for end of EXP_STATE
unsigned long targetCycleEndTime;     // Desired interval since cycleSecTimer for end of cycle

// Timers (i.e., start times, in milliseconds):
unsigned long cycleTimer;        // Start time of start of current breathing cycle
unsigned long inspStartTimer;    // Start time of inspiration state
unsigned long inspHoldTimer;     // Start time of inpsiratory state
unsigned long expStartTimer;     // Start time of expiration cycle (including exp hold & peep pause)

// Timer results (intervals, in milliseconds):
unsigned long cycleInterval;     // Milliseconds elapsed since cycleTimer at end of cycle (for logging)
unsigned long inspInterval;      // Milliseconds elapsed since inspStartTimer at end of inspiration
unsigned long expInterval;       // Milliseconds elapsed since expStartTimer at end of expiration

// Flags
bool DEBUG;
bool ventilatorOn;
VentMode ventMode = VC_MODE;

// @TODO: Implement Display class
// Display display();

//--------------Declare Functions--------------

// Set the current state in the state machine
void setState(States newState);

// Calculates the waveform parameters from the user inputs
void calculateWaveform();

// Check for errors and take appropriate action
void handleErrors();

// Helper function that gets all sensor readings
void readSensors();

// PS algorithm
void pressureSupportStateMachine();

// VC algorithm
void volumeControlStateMachine();

// Elapsed time (in ms) since start of this cycle
inline unsigned long cycleElapsedTime() {
  return millis() - cycleTimer;
}

//-------------------Set Up--------------------
void setup() {
  Serial.begin(9600);   // open serial port for debugging

  // initialize pins with pinMode command
  pinMode(SV1_CONTROL, OUTPUT);
  pinMode(SV2_CONTROL, OUTPUT);
  pinMode(SV4_CONTROL, OUTPUT);
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
  cycleTimer = millis();
}


// Run forever
void loop() {
  // All States
  // @TODO: alarm maintenance
  // display.fetchValues() // @TODO: fetch new values from display
  //calculateWaveform();
  //readSensors();
  //handleErrors();        // check thresholds against sensor values
  // display.update();   // @TODO: update display with sensor readings and flow graph

  // @TODO: implement OFF button functionality to UI with confirmation
  // if (display.turnedOff()) {
  //   setState(OFF_STATE);
  // }

  if (ventMode == PS_MODE) {
    // Run pressure support mode
    o2Management(vc_settings.o2concentration);
    pressureSupportStateMachine();
  }
  else {
    // Run volume control mode
    o2Management(ps_settings.o2concentration);
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

  // Close the inspiratory valve
  inspValve.endBreath();

  // keep expiratory valve open?
  expValve.open();
}

void beginInspiration() {
  cycleInterval = cycleElapsedTime();
  cycleTimer = millis();;  // the cycle begins at the start of inspiration

  // End expiratory cycle timer and start the inpiration timer
  expInterval = millis() - expStartTimer;
  inspStartTimer = millis();

  // close expiratory valve
  expValve.close();

  // @TODO: This will change based on recent information.
  // move insp valve using set VT and calculated insp time
  inspValve.beginBreath(targetInspEndTime, vc_settings.volume);

  // turn on PID for inspiratory valve (input = pressure, setpoint = 0)
  cycleCount++;
}

void beginInsiratorySustain() {
  // Pressure has reached set point. Record peak flow.
  flowInReader.setPeakAndReset();
}

void beginHoldInspiration() {
  // Volume control only. Not used for pressure support mode.
  inspVolume = 0;

  // close prop valve and open air/oxygen
  inspValve.endBreath();
  airValve.open();

  inspHoldTimer = millis();

  // Measure inspiration hold only once per button-press
  vc_settings.inspHoldOn = false;
}

// Perform these actions when transitioning from inspiration to any of the
// states in the expiratory cycle (EXP_STATE, HOLD_EXP_STATE, or PEEP_PAUSE).
void beginExpiratoryCycle() {
  // End inspiration timer and start the epiratory cycle timer
  inspInterval = millis() - inspStartTimer;
  expStartTimer = millis();
}

void beginExpiration() {
  inspValve.endBreath();
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
      if (/* onButtonPressed */ true) {
        setState(INSP_STATE);
        beginInspiration();  // close valves, etc.
      }
      break;

    case INSP_STATE:
      // compute PID based on linear pressure function

      // check if we reached peak pressure
      // still PID, setpoint should be peak constantly

      if (inspPressureReader.peak() >= ps_settings.peak) {
        beginInsiratorySustain();
        setState(INSP_SUSTAIN_STATE);
      }
      break;

    case INSP_SUSTAIN_STATE:
      if (flowInReader.get() < (CYCLE_OFF * flowInReader.peak())) {
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
      if (cycleElapsedTime() > targetExpEndTime) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      if(cycleElapsedTime() > targetExpEndTime + MIN_PEEP_PAUSE) {
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
      if (cycleElapsedTime() > APNEA_BACKUP) {
        alarmMgr.activateAlarm(ALARM_APNEA);
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
      if (/* onButtonPressed */ true) {
        setState(INSP_STATE);
        beginInspiration();  // close valves, etc.
      }
      break;

    case INSP_STATE:
      if (inspVolume >= vc_settings.volume || cycleElapsedTime() >= targetInspEndTime) {
        if (vc_settings.inspHoldOn) {
          setState(HOLD_INSP_STATE);
          beginHoldInspiration();
        }
        else {
          setState(EXP_STATE);
          beginExpiration();
        }
      }
      else {
        // keep opening valve until targetInspEndTime is elapsed
        inspValve.maintainBreath();
        // Stay in INSP_STATE
      }
      break;

    case INSP_SUSTAIN_STATE:
      // Should never get here in volume control mode.  In the unlikely event
      // that we find ourselves here, switch immediately to expiration state.
      setState(EXP_STATE);
      beginExpiration();
      break;

    case HOLD_INSP_STATE:
      if (HOLD_INSP_DURATION <= millis() - inspHoldTimer) {
        inspPressureReader.setPlateau();
        setState(EXP_STATE);
        beginExpiratoryCycle();
        beginExpiration();
      }
      break;

    case EXP_STATE:
      if (cycleElapsedTime() > targetExpEndTime) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      if(cycleElapsedTime() > targetExpEndTime + MIN_PEEP_PAUSE) {
        // record the peep as the current pressure
        inspPressureReader.peep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE:
      // Check if patient triggers inhale
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - SENSITIVITY_PRESSURE;

      if (patientTriggered || cycleElapsedTime() > targetCycleEndTime) {
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
