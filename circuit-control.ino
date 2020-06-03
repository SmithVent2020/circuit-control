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
#include "Timer.h"

//-----------------------------------------------INITIALIZE VARIABLES---------------------------------------------------
unsigned long cycleCount = 0; // number of breaths (including current breath)

float targetExpVolume    = 0; // minimum target volume for expiration

// Timers
Timer inspTimer;     // Measure length of inspiration (excluding inspiratory hold)
Timer expTimer;      // Measure length of expiration (including peep pause and expiratory hold)
Timer breathTimer;   // Measure length of a whole breath

// Countdown timers
CountdownTimer breathCountdown; // Countdown to desired end of breath (at end of Hold exp state)
CountdownTimer inspCountdown;   // Countdown to desired end of inspiration
CountdownTimer expCountdown;    // Countdown to desired end of expiration (VC mode only)
CountdownTimer inspHoldCountdown(HOLD_INSP_DURATION); // Countdown to desired end of INSP_HOLD
CountdownTimer peepPauseCountdown(MIN_PEEP_PAUSE);    // Countdown to desired end of PEEP_PAUSE
CountdownTimer apneaCountdown(APNEA_BACKUP);          // Countdown to sounding apnea alarm (PS only)

// Timer results (ms):
unsigned long breathDuration;   // Measured length of a whole inspiration-expiration cycle
unsigned long inspDuration;     // Measured length of inspiration (not including inspiratory hold)
unsigned long expDuration;      // Measured length of expiration (including peep pause and expiratory hold)

float desiredInspFlow; // desired inspiratory flowrate
bool onButton = true;  // True if ON button is depressed

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

void recordBreathValues(){

    /*int pip =*/


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


void valveStates() {
  Serial.print(" SV1: ");
  Serial.print(airValve.get());
  Serial.print("  SV2: ");
  Serial.print(oxygenValve.get());
  Serial.print("  SV3: ");
  Serial.print(inspValve.get());
  Serial.print("  SV4: ");
  Serial.print(expValve.get());
}

//-------------------Set Up--------------------
void setup() {
  delay(10000); //allow 10 seconds for the tester to get they system ready @debugging
  Serial.begin(115200);   // open serial port for @debugging

  inspFlowReader.calibrateToZero(); //set non-flow analog readings as the 0 in the flow reading functions
  expFlowReader.calibrateToZero();

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
  inspValve.previousPosition = 65;      //initial value for valve to open according to previous tests (close to desired)

  ventMode = VC_MODE; //for testing VC mode only
  expValve.close(); //close exp valve
  Serial.println("closing expValve");
  //digitalWrite(SV4_CONTROL, HIGH); //@debugging to see if SV4 is being controlled correctly
  setState(OFF_STATE);

  // @TODO: implement startup sequence on display
  // display.begin();
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

  // Record length of previous expiration and entire previous breath
  expDuration    = expTimer.elapsed();
  breathDuration = breathTimer.elapsed();

  vc_display.pip = inspPressureReader.peak();             // cmH2O
  vc_display.pPlat = inspPressureReader.plateau();        // cmH2O
  vc_display.PEEP =expPressureReader.peep();              // cmH2O
  vc_display.expTidalVolume = expFlowReader.getVolume();                                  // record expiratory tidal volume
  vc_display.respiratoryRate = (60000.0/breathDuration);                                 //measured bpm
  vc_display.minuteVolume = (vc_display.inspTidalVolume*CC_PER_MS_TO_LPM/breathDuration); //record minuteVolume in LPM

  // close expiratory valve
  expValve.close();
  // digitalWrite(SV4_CONTROL, HIGH); //@debugging to see if SV4 is being controlled correctly
  Serial.println("closed expValve");

  // Compute target intervals at current settings
  if (ventMode == PS_MODE) {
    unsigned long targetBreathDuration = 60000UL / ps_settings.bpm; // ms per breath
    breathCountdown.setCountdown(targetBreathDuration);
    inspCountdown.setCountdown(targetBreathDuration / 2); // @TODO: How should this be set?
  }
  else {
    unsigned long targetBreathDuration = 60000UL / vc_settings.bpm; // ms per breath
    breathCountdown.setCountdown(targetBreathDuration);

    // Compute target INSP duration as a percent of cycle time
    unsigned long targetInspDuration = targetBreathDuration * vc_settings.inspPercent / 100;
    //Serial.print("targetInspDuration:"); Serial.print("\t"); Serial.println(targetInspDuration);
    inspCountdown.setCountdown(targetInspDuration + INSP_TIME_SENSITIVITY);
    expCountdown.setCountdown(targetBreathDuration - targetInspDuration - MIN_PEEP_PAUSE + EXP_TIME_SENSITIVITY);
    //Serial.print("vc_settings.volume"); Serial.print("\t"); Serial.println(vc_settings.volume);
    desiredInspFlow = (vc_settings.volume*CC_PER_MS_TO_LPM/targetInspDuration); //desired inspiratory flowrate cc/ms
    //Serial.print("desiredInspFlow:"); Serial.print("\t"); Serial.println(desiredInspFlow);
  }
  // Start timers for entire breath and for just inspiration
  breathTimer.start();
  breathCountdown.start();
  inspTimer.start();
  inspCountdown.start();

  inspPressureReader.setPeakAndReset(); //cmH2O

  // @TODO: This will change based on recent information.
  // move insp valve using set VT and calculated insp time
  inspValve.beginBreath(desiredInspFlow);
  //Serial.println("begin breath with prop valve");

  // Start computing inspiration volume
  //Serial.print("max Insp volume ="); Serial.print("\t"); Serial.println(inspFlowReader.getVolume());
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
  inspHoldCountdown.start();

  // Measure inspiration hold only once per button-press
  vc_settings.inspHoldOn = false;
}

void beginExpiration() {
  //Serial.println("entering exp state"); //uncomment for @debugging

  vc_display.inspTidalVolume = inspFlowReader.getVolume(); //record inspiratory tidal Volume

  inspValve.endBreath();
  //Serial.println("endBreath with prop valve");
  expValve.open();
  //digitalWrite(SV4_CONTROL, LOW);
  //digitalWrite(SV4_CONTROL, LOW); //@debugging to see if SV4 is being controlled correctly
  //Serial.println("opened expValve"); //@debugging

  targetExpVolume = inspFlowReader.getVolume() * 8 / 10;  // Leave EXP_STATE when expVolume is 80% of inspVolume

  //Serial.print("expStartTime ="); Serial.print("\t"); Serial.println(millis());
  expTimer.start();
  expCountdown.start();   // VC mode
  apneaCountdown.start(); // PS mode

  //Serial.print("max exp volume ="); Serial.print("\t"); Serial.println(inspFlowReader.getVolume());
  expFlowReader.resetVolume();

}

void beginPeepPause() {
  //Serial.println("entering PEEP Pause state"); //uncomment for @debugging
  peepPauseCountdown.start();
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
      //expFlowReader.updateVolume();

      // @TODO compute PID based on linear pressure function

      // check if we reached peak pressure
      // still PID, setpoint should be peak constantly

      if (inspPressureReader.peak() >= ps_settings.peak) {
        inspDuration = inspTimer.elapsed(); // Record length of inspiration
        beginInsiratorySustain();
        setState(INSP_SUSTAIN_STATE);
      }
      break;

    case INSP_SUSTAIN_STATE:
      inspFlowReader.updateVolume();
      //expFlowReader.updateVolume();
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
      //inspFlowReader.updateVolume();
      // Move to peep pause if the expiratory volume is at least at the target volume
      if (expFlowReader.getVolume() >= targetExpVolume) {
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      else if (apneaCountdown.hasExpired()) {
        alarmMgr.activateAlarm(ALARM_APNEA);
        ventMode = VC_MODE;
        setState(INSP_STATE);
        beginInspiration();
      }
      break;

    case PEEP_PAUSE_STATE:
      // We don't need to keep track of the volume anymore, but we might want to, e.g., to display to user.
      // expFlowReader.updateVolume();
      if (peepPauseCountdown.hasExpired()) {
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

        // @TODO: write peak, and PEEP to display
        setState(INSP_STATE);
        beginInspiration();
      }

      // Apnea check
      if (apneaCountdown.hasExpired()) {
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
      //if (onButton) { //@debugging put this if statement back in when we have an on button
      setState(INSP_STATE);
      beginInspiration();  // close valves, etc.
      //}
      break;

    case INSP_STATE: {
      Serial.println("in insp state"); //@debugging
      inspFlowReader.updateVolume();
      //expFlowReader.updateVolume();
      bool timeout = inspCountdown.hasExpired();
      //Serial.print("inspCountdown ="); Serial.print("\t"); Serial.println(inspCountdown.remaining()); //@debugging
      //Serial.print("set TV volume ="); Serial.print("\t"); Serial.println(vc_settings.volume); //@debugging
      if (inspFlowReader.getVolume() >= vc_settings.volume || timeout) { //@debugging add the following back:
        if (timeout) {
          alarmMgr.activateAlarm(ALARM_TIDAL_LOW);
        }

        inspDuration = inspTimer.elapsed(); // Record length of inspiration
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
      }
      else {
        // keep opening valve until inspCountdown has expired
        Serial.println("maintaining breath");
        inspValve.maintainBreath(inspTimer.elapsed());
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
      //Serial.print("inspHoldCountdown"); Serial.print("\t"); Serial.println(inspHoldCountdown.remaining()); //@debugging
      if (inspHoldCountdown.hasExpired()) {
        inspPressureReader.setPlateau();
        setState(EXP_STATE);
        beginExpiration();
      }
      break;

    case EXP_STATE:
      Serial.println("in exp state");
      expFlowReader.updateVolume();
      //inspFlowReader.updateVolume();
      //Serial.print("expCountdown ="); Serial.print("\t"); Serial.println(expCountdown.remaining()); //@debugging
      //Serial.print("target exp volume"); Serial.print("\t"); Serial.println(targetExpVolume);
      if (expFlowReader.getVolume() >= targetExpVolume || expCountdown.hasExpired()){
        setState(PEEP_PAUSE_STATE);
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      Serial.println("in PEEP state");
      expFlowReader.updateVolume();
      //inspFlowReader.updateVolume();
      // Serial.print("peepPauseCountdown"); Serial.print("\t"); Serial.println(peepPauseCountdown.remaining()); //@debugging
      if (peepPauseCountdown.hasExpired()) {
        // record the peep as the current pressure
        expPressureReader.setPeep();
        setState(HOLD_EXP_STATE);
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE: {
      Serial.println("in exp Hold state");
      expFlowReader.updateVolume();
      //inspFlowReader.updateVolume();

      // Check if patient triggers inhale
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - SENSITIVITY_PRESSURE;
      bool timeout = breathCountdown.hasExpired();

      //Serial.print("patientTriggered?"); Serial.print("\t"); Serial.println(patientTriggered); //@debugging
      //Serial.print("breath time"); Serial.print("\t"); Serial.println(breathCountdown.remaining()); //@debugging

      if (timeout) { //@debugging add back with real patient: patientTriggered ||
        if (!patientTriggered){

        }

        // @TODO: write PiP, PEEP and Pplat to display
        beginInspiration();
        setState(INSP_STATE);
      }

    } break;
  } // End switch
}
