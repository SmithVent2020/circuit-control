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
#include "Display.h"
#include "MeanSmooth.h"


//-----------------------------------------------INITIALIZE VARIABLES---------------------------------------------------
unsigned long cycleCount = 0; // number of breaths (including current breath)

float targetExpVolume    = 0; // minimum target volume for expiration

// Target time parametaers (in milliseconds). Calculated, not measured.
unsigned long targetCycleEndTime;     // desired time at end of breath (at end of HOLD_EXP_STATE)
unsigned long targetInspEndTime;      // desired time for end of INSP_STATE
unsigned long targetExpDuration;      // Desired length of EXP_STATE (VC mode only)
unsigned long targetExpEndTime;       // Desired time at end of EXP_STATE (VC mode only)

// Timers (i.e., start times, in milliseconds):
unsigned long cycleTimer;        // Start time of start of current breathing cycle
unsigned long inspHoldTimer;     // Start time of inpsiratory hold state
unsigned long expTimer;          // Start time of expiration cycle (including exp hold & peep pause)
unsigned long peepPauseTimer;    // Start time of peep pause

// Timer results (intervals, in milliseconds):
unsigned long cycleDuration;      // Measured length of a whole inspiration-expiration cycle
unsigned long inspDuration;       // Measured length of inspiration (not including inspiratory hold)
unsigned long expDuration;        // Measured length of expiration (including peep pause and expiratory hold)
unsigned long targetInspDuration; //desired duration of inspiration


float desiredInspFlow; //desired inspiratory flowrate
bool onButton = true;  //should be connected to a physical on button


float tidalVolumeInsp = 0.0; //measured inspiratory tidal volume
float tidalVolumeExp = 0.0;  //measured expiratory tidal volume

float lastPeep = 0.0/0.0; //PEEP from last breath
float lastPeak = 0.0/0.0; //peak pressure from last breath


// Flags
bool DEBUG = false;          // for debugging mode
bool ventilatorOn = false;   //@cleaning: get rid of this, we already have an on button 
VentMode ventMode = VC_MODE; //set the default ventilation mode to volume control 

// @TODO: Implement Display class @cleaning
// Display display();

//--------------Declare Functions--------------

//function to Set the current state in the state machine
void setState(States newState);

// function to Calculates the waveform parameters from the user inputs
void calculateWaveform(); //@cleaning


// Helper function that reads all sensors
void readSensors(){

  //inspiratory sensors
  inspFlowReader.read();                   // inspiratory flow in L/min (SLPM)
  inspPressureReader.read();               // inspiratory pressure in cmH2O
  reservoirPressureReader.readReservoir(); // gas reservoir pressure in cmH2O

  //expiratory sensors
  expFlowReader.read();                   // expiratory flow in L/min (SLPM)
  expPressureReader.read();               // expiratory pressure in cmH2O

}

//function to check if sensor readings are within acceptable ranges,
//activate an alarm if they are not and deactivate the alarm once they are again
void checkAlarmRangeWithUpdate(float reading, float &compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode){ 
  Serial.print("max value ="); Serial.print("\t"); Serial.println(compareValue + sensitivity); 
  bool updateComparison = true; //flag that indicates if the comparison value should be updated from the sensor reading or not

  // if this is the first reading, just store the value
  if (!isnan(compareValue)) {
    // otherwise compare to previous:
    if(reading > compareValue + sensitivity) { 
      // if the reading is abnormally high
      alarmMgr.activateAlarm(highAlarmCode);
      Serial.print("Activating alarmCodes:"); Serial.print("\t"); Serial.println(highAlarmCode);
      updateComparison = false;   //don't update the comparison value while alarm is going off so alarm will continue until value returns to a normal (pre-alarm) value
    } else {
      alarmMgr.deactivateAlarm(highAlarmCode);
      Serial.print("deactivating alarmCode:"); Serial.print("\t"); Serial.println(highAlarmCode);
    }
    
    if (reading < compareValue - sensitivity) { 
      alarmMgr.activateAlarm(lowAlarmCode);
      Serial.print("Activating alarmCodes:"); Serial.print("\t"); Serial.println(lowAlarmCode);   
      updateComparison = false;   //don't update the comparison value while alarm is going off so alarm will continue until value returns to a normal (pre-alarm) value
    } else {
      alarmMgr.deactivateAlarm(lowAlarmCode); 
      Serial.print("deactivating alarmCode:"); Serial.print("\t"); Serial.println(lowAlarmCode);
    }
  }

  // remember value for next comparison, if we're not already abnormal
  if (updateComparison) {
    compareValue = reading;
  }
}

// check against a value that doesn't need to be stored -- call above with dummy variable
void checkAlarmRange(float reading, float compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode){ 
  float dummyCompareValue = compareValue;
  checkAlarmRangeWithUpdate(reading, dummyCompareValue, sensitivity, highAlarmCode, lowAlarmCode);
}

// Check for errors in sensor readings and take appropriate action
void checkSensorReadings(){

  Serial.print("current pip"); Serial.print("\t"); Serial.println(inspPressureReader.peak());
  Serial.print("last pip"); Serial.print("\t"); Serial.println(lastPeak);
  checkAlarmRangeWithUpdate(inspPressureReader.peak(), lastPeak, INSP_PRESSURE_SENSITIVITY, ALARM_INSP_HIGH, ALARM_INSP_LOW);
  //checkAlarmRangeWithUpdate(expPressureReader.peep(), lastPeep, PEEP_SENSITIVITY, ALARM_PEEP_HIGH,  ALARM_PEEP_LOW); @cleanup remove this
  checkAlarmRange(tidalVolumeInsp, display.volume(), display.volume()/TIDAL_VOLUME_SENSITVITY, ALARM_TIDAL_HIGH, ALARM_TIDAL_LOW);
}

void recordBreathValues(){

    /*int pip =*/
    //@cleaup

 }



// PS algorithm
void pressureSupportStateMachine();

// VC algorithm
void volumeControlStateMachine();




void displaySensors(){ //for @debugging and testing purposes, displays sensor readings and key patient data to the serial port
  Serial.println(" ");
  Serial.print("time"); Serial.print("\t");
  Serial.print("IF");Serial.print("\t");
  Serial.print("EF");Serial.print("\t");
  Serial.print("IP");Serial.print("\t");
  Serial.print("EP");Serial.print("\t");
  Serial.print("RP");Serial.print("\t");
  Serial.print("IV");Serial.print("\t");
  Serial.println("EV"); Serial.print("\t");
  Serial.println("pip"); Serial.print("\t");
  Serial.println("pPlat"); Serial.print("\t");
  Serial.print("PEEP");  Serial.print("\t");
  Serial.println("TVi"); Serial.print("\t");
  Serial.println("TVe"); 

  Serial.print(millis()); Serial.print("\t");
  Serial.print(inspFlowReader.get()); Serial.print("\t"); //L/min
  Serial.print(expFlowReader.get()); Serial.print("\t");  //L/min
  Serial.print(inspPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(expPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(reservoirPressureReader.get()); Serial.print("\t"); //cmH2O
  Serial.print(inspFlowReader.getVolume());  Serial.print("\t");//cc
  Serial.print(expFlowReader.getVolume()); Serial.print("\t");//cc
 
  Serial.print(inspPressureReader.peak());  Serial.print("\t");
  Serial.print(inspPressureReader.plateau()); Serial.print("\t");
  Serial.print(expPressureReader.peep());  Serial.print("\t");
  Serial.print(tidalVolumeInsp); Serial.print("\t");
  Serial.println(tidalVolumeExp );  
  

}

//for @debugging: displays valve states to the serial port
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
  delay(9000); // allow 10 seconds for the tester to get the system to get ready @debugging @cleanup
  Serial.begin(115200);   // open serial port for @debugging

  // initialize screen
  display.init();

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


  // setup PID controller (for VC mode, the default mode) @cleanup: add PID initialization to PS mode
  inspValve.initializePID(40, 120, 50); // set output max to 40, output min to 120 and sample time to 50
  inspValve.previousPosition = 65;      // initial value for valve to open according to previous tests (close to desired)

  // warm up SV3 valve by opening it to unstick it
  analogWrite(SV3_CONTROL, 255);
  delay(35);
  analogWrite(SV3_CONTROL, 0);

  //set to VC_MODE (@FutureWork this should change once a UI startup sequence is built)
  ventMode = VC_MODE;                   // for testing VC mode only
  expValve.close();                     // close exp valve
  Serial.println("closing expValve");   //@cleanup
  setState(OFF_STATE);                  //set state to OFF_STATE

  // calibrate flow meters -- seems to change when SV4 closes
  inspFlowReader.calibrateToZero(); //set non-flow analog readings as the 0 in the flow reading functions
  expFlowReader.calibrateToZero();  

  // @FutureWork: implement startup sequence on display
  // display.start();

  cycleTimer = millis(); //begin cycle timer
}


// Run forever
void loop() {
  // All States
  display.listen();

  //check if the user has turned of ventilation
  if (display.isTurnedOff()) {
    setState(OFF_STATE);
  }

  
  readSensors();               //read all sensors, and check to see if readings are within acceptable ranges
  displaySensors();            // for @debugging display readings to serial monitor

  //@debugging only alarm after first 5 breaths (because we have not yet solved "warm up" issue)
  if(cycleCount > 5){  
    checkSensorReadings();     // check thresholds against sensor values
    alarmMgr.maintainAlarms(); //maintain alarms 
  }
  
  display.updatePressureWave(inspPressureReader.get()); //update the inspiratory pressure waveform on the display

  //manage reservoir refilling based on FIO2 concentration set by user on the display
  o2Management(display.oxygen());

  if (ventMode == PS_MODE) {
    // Run pressure support mode
    //Serial.println("entering PS mode"); //@debugging 
    o2Management(display.oxygen()); //@cleanup
    pressureSupportStateMachine(); 
  } else if (ventMode == VC_MODE) {
    // Run volume control mode
    //Serial.println("Entering VC_Mode"); //@debugging
    
    o2Management(display.oxygen()); //@clanup

    //Serial.println("entering VC state machine"); //@cleanup
    volumeControlStateMachine();

    //Serial.println("exiting VC state machine"); //@cleanup
  } else {
    Serial.println("no mode entered");
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

  // @FutureWork: implement a turnOffAll method in AlarmManager class to be used here, turn off ongoing alarms after confirmation from user

  // Close the inspiratory valve
  inspValve.endBreath();

  // keep expiratory valve open durring off because it is the safer position (and does not use as much power)
  expValve.open();
  
  //digitalWrite(SV4_CONTROL, LOW);  //@debugging to see if SV4 open and close function is working correctly @cleanup
  Serial.println("opened expValve"); //@debugging
}

void beginInspiration() {
  //runs during any transition to the INSP_STATE from any other state
  //Serial.println("entering insp state"); //uncomment @cleanup
  cycleDuration = millis() - cycleTimer;   //calculate the length of the last breath
  cycleTimer = millis();                   // reset the cycle timer at the start of inspiration
  // We could have an inspTimer, but it would be the same as cycleTimer.

  //record values from previous breath
  expDuration = cycleTimer - expTimer;        //calculate measured duration of expiration from last breath (EXP_STATE + PEEP_PAUSE + EXP_HOLD)
  tidalVolumeExp = expFlowReader.getVolume(); //set current inspired volume as the measured inspiratory tidal volume for the last breath

  //update patient data on display to reflect values from last breath
  display.writePeak(inspPressureReader.peak());           // measured pip cmH2O
  display.writePlateau(inspPressureReader.plateau());     // measured plateau (only measured if HOLD_INSP_STATE is activated) cmH2O
  display.writePeep(expPressureReader.peep());            // measured PEEP cmH2O
  display.writeVolumeExp(expFlowReader.getVolume());      // measured expired volume
  display.writeMinuteVolume(inspFlowReader.getVolume() * CC_PER_MS_TO_LPM / cycleDuration); //measured minute volume
  display.writeBPM(60000.0/cycleDuration);                // measured respiratory rate (seconds)
  display.writeO2(O2);                                    // measured FIO2 concentration (@FutureWork: pending addition of O2 sensor)  

 
  // close expiratory valve
  expValve.close();
  // digitalWrite(SV4_CONTROL, HIGH); //@debugging to see if SV4 is being controlled correctly
  Serial.println("closed expValve"); //@cleanup

  // Compute intervals at current settings
  if (ventMode == PS_MODE) {
    unsigned long targetCycleDuration = 60000UL / display.bpm();  // ms from start of cycle to end of inspiration
    targetCycleEndTime = cycleTimer + targetCycleDuration;        // target end time for the breath
    targetInspEndTime  = cycleTimer + targetCycleDuration / 2;    // @FutureWork: How should this be set for PS Mode?
  }
  else {
    unsigned long targetCycleDuration = 60000UL / display.bpm(); // ms from start of cycle to end of inspiration

    targetInspDuration = 105*targetCycleDuration * display.inspPercent() / 10000;  //allowing a bit more time to complete tidal volume inhailation
    //Serial.print("targetInspDuration:"); Serial.print("\t"); Serial.println(targetInspDuration); //@cleaup

    targetCycleEndTime = cycleTimer + targetCycleDuration;                         //target time for breath to end (for HOLD_EXP_STATE to end)
    targetInspEndTime  = cycleTimer + targetInspDuration;                          //target time for INSP_STATE to end
    targetExpDuration  = targetCycleDuration - targetInspDuration - MIN_PEEP_PAUSE;//target time for EXP_STATE to end
    desiredInspFlow = (display.volume() * CC_PER_MS_TO_LPM / targetInspDuration);  //desired inspiratory flowrate cc/ms
    
  }
  
  inspValve.beginBreath(desiredInspFlow); // begin PID control of the insp valve according to the desired inspiratory flowrate
  inspFlowReader.resetVolume();           //reset the inspiration tidal volume counter
  cycleCount++;                           //increment the counter of how many breaths have passed since machine was turned on last
}

void beginInsiratorySustain() {
  //run the following every time we transition to INSPIRATORY_SUSTAIN_STATE (PS mode only)
  //Serial.println("entering insp Sustain state"); //uncomment for @debugging @cleanup
  // Pressure has reached set point. Record peak flow.
  inspFlowReader.setPeakAndReset();
}

void beginHoldInspiration() {
  //run the following every time we transition to HOLD_INSP state
  //Serial.println("entering hold insp state"); //uncomment for @debugging @cleanup
  // Volume control only. Not used for pressure support mode.

  // close proportion insp valve and turn of PID control                  
  inspValve.endBreath();
  //Serial.println("end breath with prop vale"); //@cleanup
  inspHoldTimer = millis(); //reset the inspiratory hold timer

  // Measure inspiration hold only once per button-press
  display.resetInspHold();
}

void beginExpiration() {
  //run every time we enter EXP_STATE from any other state
  inspPressureReader.setPeakAndReset(); // reset pip, cmH2O

  tidalVolumeInsp = inspFlowReader.getVolume(); //record measured inspiratory tidal volume
  display.writeVolumeInsp(tidalVolumeInsp);     //display measured inspiratory tidal Volume

  
  inspValve.endBreath(); //close insp valve and turn off PID control
  expValve.open();       //open expiration valve
  expTimer = millis();   //reset expiration timer

  targetExpVolume = inspFlowReader.getVolume() * 8 / 10; //calculate 80% of inspired volume (we want to leave EXP_STATE when expVolume is 80% of inspVolume)

  //Serial.print("expStartTime ="); Serial.print("\t"); Serial.println(expTimer); //@cleanup
  targetExpEndTime = expTimer + targetExpDuration;       //calculate the target time for EXP_STATE to end

  //Serial.print("max exp volume ="); Serial.print("\t"); Serial.println(inspFlowReader.getVolume());
  expFlowReader.resetVolume();                           //reset the expiratory volume counter
  

}

void beginPeepPause() {
  //run every time we enter PEEP_PAUSE state
  //Serial.println("entering PEEP Pause state"); //uncomment for @debugging
  peepPauseTimer = millis(); //reset peep pause timer
}

void beginHoldExpiration() {
  // Nothing to do when entering  HOLD_EXP state
  //Serial.println("entering exp hold state"); //uncomment for @debugging @cleanup
}

void pressureSupportStateMachine() {
  switch (state) {
    case OFF_STATE:
      // @TODO How do we transition out of the OFF_STATE?
      if (onButton) {
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

      if (inspPressureReader.peak() >= display.peak()) {
        inspDuration = millis() - cycleTimer; // Record length of inspiration
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

//volume control state machine
void volumeControlStateMachine(){
  //Serial.println(state); @cleanup

  switch (state) {
    case OFF_STATE:
      Serial.println("in off state"); //@debugging
      //if (onButton) { //@debugging put this if statement back in when we have an on button @cleanup, add this back
      setState(INSP_STATE);
      beginInspiration();  // close valves, etc.
      //}
      break;

    case INSP_STATE: {
      Serial.println("in insp state"); //@debugging
      
      display.updateFlowWave(inspFlowReader.get());                           //update flow waveform on the display
      inspFlowReader.updateVolume();                                          //update the inpsiratory volume counter
      
      bool timeout = (millis() >= targetInspEndTime + INSP_TIME_SENSITIVITY); //calculate when the INSP_STATE should time out

      //transition out of INSP_STATE if either the inspired volume >= tidal volume set by user on the display
      //or if the INSP_STATE  has timed out (according to set BPM, IE ratio and INSP_TIME_SENSITIVITY
      if (inspFlowReader.getVolume() >= display.volume() || timeout) { 
        
        if (timeout) {
         //If the transition was triggered by a timeout, not the inspiredv volume:
         alarmMgr.activateAlarm(ALARM_TIDAL_LOW);         // trigger the low tidal volume alarm
         
        }else if(alarmMgr.alarmStatus(ALARM_TIDAL_LOW)){
          alarmMgr.deactivateAlarm(ALARM_TIDAL_LOW);      //otherwise deactivate a low tidal volume alarm if one is currently on
        }

        if (display.inspHold()) { 
          //if user has turned on the inspiratory hold, 
          setState(HOLD_INSP_STATE); //transition to INSP_HOLD_STATE
          Serial.print("calling beginHoldInspiration"); //@cleanup
          display.resetInspHold();   //reset inspiratory hold value on display
          beginHoldInspiration();    //begin the INSP_HOLD_STATE
        }
        else {
          //if inspiratory hold is not on, transition to EXP_STATE
          setState(EXP_STATE);
          Serial.println("calling beginExpiration"); //@cleanup
          beginExpiration();
        }

        inspDuration = millis() - cycleTimer; // Record length of inspiration
      }
      else {
        // keep adjusting inspiratory valve until targetInspEndTime is reached
        Serial.println("maintaining breath"); //@cleanup
        inspValve.maintainBreath(cycleTimer);
        // Stay in INSP_STATE
      }
    } break;

    case INSP_SUSTAIN_STATE:
      Serial.println("in insp sustain state");      //@debugging
      display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display based on current flow reading
      // Should never get here in volume control mode.  In the unlikely event
      // that we find ourselves here, switch immediately to expiration state.
      setState(EXP_STATE);
      beginExpiration();
      break;

    case HOLD_INSP_STATE:
      Serial.println("in hold insp state");         //@debugging
      display.updateFlowWave(inspFlowReader.get()); //update flow waveform on display, based on current flow reading
      
      if (HOLD_INSP_DURATION <= millis() - inspHoldTimer) {
        //if we have not reached the end time for HOLD_INSP_STATE
        inspPressureReader.setPlateau();              //record current inspiratory pressure as the plateau pressure
        if(inspPressureReader.plateau() > PPLAT_MAX){ 
         //check if the plateau pressure is too high and alarm if so
         alarmMgr.activateAlarm(ALARM_PPLAT_HIGH);
         Serial.println("Activating PPlat High Alarm"); //@cleanup
         Serial.print("PPlat ="); Serial.print("\t"); Serial.println(inspPressureReader.plateau()); //@cleanup
        }else if(alarmMgr.alarmStatus(ALARM_PPLAT_HIGH)){
          //otherwise, deactivate the PPlat alarm if it is currently activated
          alarmMgr.deactivateAlarm(ALARM_PPLAT_HIGH);
        }
        setState(EXP_STATE); //switch to EXP_STATE
        beginExpiration();   
      }
      break;

    case EXP_STATE:
      Serial.println("in exp state");                   //@debugging
      display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display using the expiratory flow sensor and flipping reading to negative (out of the patient)
      expFlowReader.updateVolume();                     //update expiratory volume counter
      
      if (expFlowReader.getVolume() >= targetExpVolume || millis() > targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        //if 80% of inspired volume has been expired, transition to PEEP_PAUSE_STATE 
        setState(PEEP_PAUSE_STATE); 
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      Serial.println("in PEEP state");                  //@debugging
      display.updateFlowWave(expFlowReader.get() * -1); //update expiratory flow waveform on display
      expFlowReader.updateVolume();                     //update expiratory volume counter
      
      if (millis() - peepPauseTimer >= MIN_PEEP_PAUSE) {
        //if the PEEP pause time has run out
        expPressureReader.setPeep(); // record the peep as the current pressure
        setState(HOLD_EXP_STATE);    //transition to HOLD_EXP_STATE
        beginHoldExpiration();
      }
      break;

    case HOLD_EXP_STATE: {
      Serial.println("in exp Hold state");              //@debugging
      display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display
      expFlowReader.updateVolume();                     //update expiratory flow counter

      // Check if patient triggers inhale by checkin if expiratory pressure has dropped below the pressure sensitivity set by user
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - display.sensitivity();
      bool timeout = (millis()  > targetCycleEndTime); //check if the expiratory hold timer has run out

      if (timeout) { //@debugging add back with real patient: patientTriggered || @cleanup: add this back
        //if the patient has triggered a breath, or the timer has run out transition to INSP_STATE
        beginInspiration();   
        setState(INSP_STATE);
      }

    } break;
  } // End switch
}
