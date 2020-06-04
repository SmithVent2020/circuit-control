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

float targetExpVolume    = 0; // minimum target volume for expiration

// Target time parameters (in milliseconds). Calculated, not measured.
unsigned long targetCycleEndTime;     // desired time at end of breath (at end of Hold exp state)
unsigned long targetInspEndTime;      // desired time for end of INSP_STATE
unsigned long targetExpDuration;      // Desired length of EXP_STATE (VC mode only)
unsigned long targetExpEndTime;       // Desired time at end of EXP_STATE (VC mode only)

// Timers (i.e., start times, in milliseconds):
unsigned long cycleTimer;        // Start time of start of current breathing cycle
unsigned long inspHoldTimer;     // Start time of inpsiratory hold state
unsigned long expTimer;          // Start time of expiration cycle (including exp hold & peep pause)
unsigned long peepPauseTimer;    // Start time of peep pause

// Timer results (intervals, in milliseconds):
unsigned long cycleDuration;     // Measured length of a whole inspiration-expiration cycle
unsigned long inspDuration;      // Measured length of inspiration (not including inspiratory hold)
unsigned long expDuration;       // Measured length of expiration (including peep pause and expiratory hold)
unsigned long targetInspDuration; //desired duration of inspiration

//desired inspiratory flowrate
float desiredInspFlow;
bool onButton = true;

//PID meomory


// Flags
bool DEBUG = false;
bool ventilatorOn = false;
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
void readSensors(){

  //inspiratory sensors
  inspFlowReader.read();  // L/min
  inspPressureReader.read();  //cmH2O
  reservoirPressureReader.readReservoir(); //cmH2O

  //expiratory sensors
  expFlowReader.read();  //L/min
  expPressureReader.read(); //cmH2O

}


// PS algorithm
void pressureSupportStateMachine();

// VC algorithm
void volumeControlStateMachine();


void displaySensors(){ //for @debugging and testing purposes
  Serial.println(" ");
  Serial.print("time"); Serial.print("\t");
  Serial.print("IF");Serial.print("\t");
  Serial.print("EF");Serial.print("\t");
  Serial.print("IP");Serial.print("\t");
  Serial.print("EP");Serial.print("\t");
  Serial.print("RP");Serial.print("\t");
  Serial.print("IV");Serial.print("\t");
  Serial.println("EV");

  Serial.print(millis()); Serial.print("\t");
  Serial.print(inspFlowReader.get()); Serial.print("\t"); //L/min
  Serial.print(expFlowReader.get()); Serial.print("\t");  //L/min
  Serial.print(inspPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(expPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(reservoirPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(inspFlowReader.getVolume());  Serial.print("\t");//cc
  Serial.println(expFlowReader.getVolume());   //cc
  
  
  
}
//-------------------Set Up--------------------
void setup() {
  delay(10000); //allow 10 seconds for the tester to get they system ready @debugging
  Serial.begin(115200);   // open serial port for @debugging

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
  pinMode(FLOW_INSP, INPUT);
  pinMode(FLOW_EXP, INPUT);

  //setup PID controller
  inspValve.initializePID(40, 120, 50); //set output max to 40, output min to 120 and sample time to 50
  inspValve.previousPIDOutput = 65;                                              //initial value for valve to open according to previous tests (close to desired)

  //initialize timers:
  //targetExpDuration = (100-vc_settings.inspPercent)*vc_settings.bpm/200; //begin the targeExpDuration at half what the entire expiratory cycle (exp, PEEP pause, and exp hold) will take

  ventMode = VC_MODE; //for testing VC mode only
  expValve.close(); //close exp valve
  Serial.println("closing expValve");
  //digitalWrite(SV4_CONTROL, HIGH); //@debugging to see if SV4 is being controlled correctly
  setState(OFF_STATE);

  // @TODO: implement startup sequence on display
  // display.begin();

  //Unstick the valve
  analogWrite(SV3_CONTROL, 255);
  delay(100);
  analogWrite(SV3_CONTROL, 0);
  
  cycleTimer = millis();
}


// Run forever
void loop() {
  // All States
  // @TODO: alarm maintenance
  // display.fetchValues() // @TODO: fetch new values from display



  //calculateWaveform();

  //read all the pressure and flow sensors
  readSensors();
  displaySensors(); //for @debugging

  //handleErrors();        // check thresholds against sensor values
  // display.update();     // @TODO: update display with sensor readings and flow graph

  // @TODO: implement OFF button functionality to UI with confirmation
  // if (display.turnedOff()) {
  //   setState(OFF_STATE);
  // }

  //manage reservoir refilling
  o2Management(vc_settings.o2concentration);

  if (ventMode == PS_MODE) {
    //Serial.println("entering PS mode"); @debugging
    // Run pressure support mode
    o2Management(ps_settings.o2concentration);
    pressureSupportStateMachine();
  }
  else if (ventMode == VC_MODE) {
    //Serial.println("Entering VC_Mode");
    // Run volume control mode

    o2Management(vc_settings.o2concentration);

    //Serial.println("entering VC state machine");

    volumeControlStateMachine();

    //Serial.println("exiting VC state machine");
  }
  else{
    Serial.print("no mode entered");
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
  //digitalWrite(SV4_CONTROL, LOW);  //@debugging to see if SV4 open and close function is working correctly
  Serial.println("opened expValve");
}

void beginInspiration() {
  //Serial.println("entering insp state"); //uncomment for @debugging
  cycleDuration = millis() - cycleTimer;
  cycleTimer = millis();  // the cycle begins at the start of inspiration
  // We could have an inspTimer, but it would be the same as cycleTimer.

  // End expiratory cycle timer and start the inpiration timer
  expDuration = cycleTimer - expTimer;

  // close expiratory valve
  expValve.close();
  // digitalWrite(SV4_CONTROL, HIGH); //@debugging to see if SV4 is being controlled correctly
  Serial.println("closed expValve");

  // Compute intervals at current settings
  if (ventMode == PS_MODE) {
    unsigned long targetCycleDuration = 60000UL / ps_settings.bpm; // ms from start of cycle to end of inspiration
    targetCycleEndTime = cycleTimer + targetCycleDuration;
    targetInspEndTime  = cycleTimer + targetCycleDuration / 2;    // @TODO: How should this be set?
  }
  else {
    unsigned long targetCycleDuration = 60000UL / vc_settings.bpm; // ms from start of cycle to end of inspiration
    targetInspDuration = targetCycleDuration * vc_settings.inspPercent / 100;
    targetCycleEndTime = cycleTimer + targetCycleDuration;
    targetInspEndTime  = cycleTimer + targetInspDuration;
    targetExpDuration  = targetCycleDuration - targetInspDuration - MIN_PEEP_PAUSE;
    //Serial.print("targetCycleEndTime set to:"); Serial.print("\t"); Serial.println(targetCycleEndTime);
    //Serial.print("target duration set to:"); Serial.print("\t"); Serial.println(targetCycleDuration);
  }

  inspPressureReader.setPeakAndReset(); //cmH2O

  // @TODO: This will change based on recent information.
  // move insp valve using set VT and calculated insp time
  inspValve.beginBreath(desiredInspFlow);
  //Serial.println("begin breath with prop valve");

  // Start computing inspiration volume
  inspFlowReader.resetVolume();

  // turn on PID for inspiratory valve (input = pressure, setpoint = 0)
  cycleCount++;
}

void beginInsiratorySustain() {
  //Serial.println("entering insp Sustain state"); //uncomment for @debugging
  // Pressure has reached set point. Record peak flow.
  inspFlowReader.setPeakAndReset();
}

void beginHoldInspiration() {
  //Serial.println("entering hold insp state"); //uncomment for @debugging
  // Volume control only. Not used for pressure support mode.

  // close prop valve and                     air/oxygen
  inspValve.endBreath();
  //Serial.println("end breath with prop vale");

  inspHoldTimer = millis();

  // Measure inspiration hold only once per button-press
  vc_settings.inspHoldOn = false;
}

void beginExpiration() {
  //Serial.println("entering exp state"); //uncomment for @debugging
  inspValve.endBreath();
  //Serial.println("endBreath with prop valve");
  expValve.open();
  //digitalWrite(SV4_CONTROL, LOW);
  expTimer = millis();
  //digitalWrite(SV4_CONTROL, LOW); //@debugging to see if SV4 is being controlled correctly
  //Serial.println("opened expValve"); //@debugging

  targetExpVolume = inspFlowReader.getVolume() * 7 / 10;  // Leave EXP_STATE when expVolume is 80% of inspVolume

  //Serial.print("expStartTime ="); Serial.print("\t"); Serial.println(expTimer);
  targetExpEndTime = expTimer + targetExpDuration;
  expFlowReader.resetVolume();
}

void beginPeepPause() {
  //Serial.println("entering PEEP Pause state"); //uncomment for @debugging
  peepPauseTimer = millis();
}

void beginHoldExpiration() {
  // Nothing to do when entering hold expiration state
  //Serial.println("entering exp hold state"); //uncomment for @debugging
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
      inspFlowReader.updateVolume();
      expFlowReader.updateVolume();

      // @TODO compute PID based on linear pressure function

      // check if we reached peak pressure
      // still PID, setpoint should be peak constantly

      if (inspPressureReader.peak() >= ps_settings.peak) {
        inspDuration = millis() - cycleTimer; // Record length of inspiration
        beginInsiratorySustain();
        setState(INSP_SUSTAIN_STATE);
      }
      break;

    case INSP_SUSTAIN_STATE:
      inspFlowReader.updateVolume();
      expFlowReader.updateVolume();
      if (inspFlowReader.get() < (CYCLE_OFF * inspFlowReader.peak())) {
        setState(EXP_STATE);
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
      expFlowReader.updateVolume();
      inspFlowReader.updateVolume();
      // Move to peep pause if the expiratory volume is at least at the target volume
      if (expFlowReader.getVolume() >= targetExpVolume) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      else if (millis() - expTimer > APNEA_BACKUP) {
        alarmMgr.activateAlarm(ALARM_APNEA);
        ventMode = VC_MODE;
        setState(INSP_STATE);
        beginInspiration();
      }
      break;

    case PEEP_PAUSE_STATE:
      // We don't need to keep track of the volume anymore, but we might want to, e.g., to display to user.
      // expFlowReader.updateVolume();
      if (millis() - peepPauseTimer >= MIN_PEEP_PAUSE) {
        expPressureReader.setPeep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE:
      // We don't need to keep track of the volume anymore, but we might want to, e.g., to display to user.
      // expFlowReader.updateVolume();

      // Check if patient triggers inhale
      // using peep here, but may need to use a lower pressure threshold
      if (expPressureReader.get() < expPressureReader.peep()) {
        inspPressureReader.setPeakAndReset();
        // @TODO: write peak, and PEEP to display
        setState(INSP_STATE);
        beginInspiration();
      }

      // Apnea check
      if (millis() - expTimer > APNEA_BACKUP) {
        alarmMgr.activateAlarm(ALARM_APNEA);
        ventMode = VC_MODE;
        setState(INSP_STATE);
        beginInspiration();
      }
      break;

  } // End switch
}

void volumeControlStateMachine(){
  //Serial.println(state);

  switch (state) {
    case OFF_STATE:
      // @TODO How do we transition out of the OFF_STATE?
      Serial.println("in off state");
      //if (onButton == true) { //@debugging put this if statement back in when we have an on button
      setState(INSP_STATE);
      desiredInspFlow = vc_settings.volume/targetInspDuration; //desired inspiratory flowrate cc/ms
      beginInspiration();  // close valves, etc.
      //}
      break;

    case INSP_STATE: {
      Serial.println("in insp state"); //@debugging
      inspFlowReader.updateVolume();
      expFlowReader.updateVolume();
      bool timeout = (millis() >= targetInspEndTime);
      //Serial.print("targetInspEndTime ="); Serial.print("\t"); Serial.println(targetInspEndTime); //@debugging
      //Serial.print("set TV volume ="); Serial.print("\t"); Serial.println(vc_settings.volume); //@debugging
      if (inspFlowReader.getVolume() >= vc_settings.volume || timeout) { //@debugging add the following back: 
        if (timeout) {
          alarmMgr.activateAlarm(ALARM_TIDAL_LOW);
        }

        if (vc_settings.inspHoldOn) {
          setState(HOLD_INSP_STATE);
          Serial.print("calling beginHoldInspiration");
          beginHoldInspiration();
        }
        else {
          setState(EXP_STATE);
          Serial.println("calling beginExpiration");
          beginExpiration();
        }

        inspDuration = millis() - cycleTimer; // Record length of inspiration
      }
      else {
        // keep opening valve until targetInspEndTime is reached
        Serial.println("maintaining breath");
        inspValve.maintainBreath(cycleTimer);
        // Stay in INSP_STATE
      }
    } break;

    case INSP_SUSTAIN_STATE:
      Serial.println("in insp sustain state");
      // Should never get here in volume control mode.  In the unlikely event
      // that we find ourselves here, switch immediately to expiration state.
      setState(EXP_STATE);
      beginExpiration();
      break;

    case HOLD_INSP_STATE:
      Serial.println("in hold insp state");
      //Serial.print("inspHoldTimer"); Serial.print("\t"); Serial.println(millis() - inspHoldTimer); //@debugging
      if (HOLD_INSP_DURATION <= millis() - inspHoldTimer) {
        inspPressureReader.setPlateau();
        setState(EXP_STATE);
        beginExpiration();
      }
      break;

    case EXP_STATE:
      Serial.println("in exp state");
      expFlowReader.updateVolume();
      inspFlowReader.updateVolume();
      //Serial.print("targetEndExpTime ="); Serial.print("\t"); Serial.println(targetExpEndTime); //@debugging + EXP_TIME_SENSITIVITY
      //Serial.print("target exp volume"); Serial.print("\t"); Serial.println(targetExpVolume);
      if (expFlowReader.getVolume() >= targetExpVolume || millis() > targetExpEndTime) { //@debugging: add the following back:  EXP_TIME_SENSITIVITY +
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      Serial.println("in PEEP state");
      expFlowReader.updateVolume();
      inspFlowReader.updateVolume();
      //Serial.print("target PEEP pause"); Serial.print("\t"); Serial.println(millis() + MIN_PEEP_PAUSE); //@debugging
      //Serial.print("millis - peepPauseTimer"); Serial.print("\t"); Serial.println(millis() - peepPauseTimer); //@debugging
      if (millis() - peepPauseTimer >= MIN_PEEP_PAUSE) {
        // record the peep as the current pressure
        inspPressureReader.peep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE: {
      Serial.println("in exp Hold state");
      expFlowReader.updateVolume();
      inspFlowReader.updateVolume();
      
      // Check if patient triggers inhale
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - SENSITIVITY_PRESSURE;
      bool timeout = (millis() > targetCycleEndTime);
     
      //Serial.print("patientTriggered?"); Serial.print("\t"); Serial.println(patientTriggered); //@debugging
      //Serial.print("targetCycleEndTime ="); Serial.print("\t"); Serial.println(targetCycleEndTime); //@debugging
      
      if (timeout) { //@debugging add back: patientTriggered || 
        if (!patientTriggered) expPressureReader.setPeep();  // set peep again if time triggered
        // @TODO: write PiP, PEEP and Pplat to display
        beginInspiration();
        setState(INSP_STATE);
      }
    } break;
  } // End switch
}
