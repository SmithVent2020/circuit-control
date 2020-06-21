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


//--------------Initialize Variables--------------
unsigned long cycleCount = 0; // number of breaths (including current breath)
float targetExpVolume    = 0; // minimum target volume for expiration

// Calculated target time parameters (in ms)
unsigned long targetCycleEndTime;     // desired time at end of breath (at end of HOLD_EXP_STATE)
unsigned long targetInspEndTime;      // desired time for end of INSP_STATE
unsigned long targetExpDuration;      // desired length of EXP_STATE (VC mode only)
unsigned long targetExpEndTime;       // desired time at end of EXP_STATE (VC mode only)
unsigned long targetInspDuration;     // desired duration of inspiration

// Timers (in ms)
unsigned long cycleTimer;        // start time of start of current breathing cycle
unsigned long inspHoldTimer;     // start time of inpsiratory hold state
unsigned long expTimer;          // start time of expiration cycle (including exp hold & peep pause)
unsigned long peepPauseTimer;    // start time of peep pause

// Measured timer intervals (in ms)
unsigned long cycleDuration;      // measured length of a whole inspiration-expiration cycle
unsigned long inspDuration;       // measured length of inspiration (not including inspiratory hold)
unsigned long expDuration;        // measured length of expiration (including peep pause and expiratory hold)

// breathing circuit values to keep track of
float desiredInspFlow; // desired inspiratory flowrate
bool onButton = true;  // should be set to true when user indicates so on screen

float tidalVolumeInsp = 0.0; // measured inspiratory tidal volume
float tidalVolumeExp = 0.0;  // measured expiratory tidal volume

float lastPeep = 0.0/0.0; // PEEP from last breath
float lastPeak = 0.0/0.0; // peak pressure from last breath

// Flags
bool DEBUG = false;          // for debugging mode
VentMode ventMode = VC_MODE; //set the default ventilation mode to volume control 

//--------------Declare Functions--------------
/**
 * function to set the current state in the state machine 
 */
void setState(States newState);

/**
 * helper function that reads all sensors and updates values 
 */
void readSensors(){
  //inspiratory sensors
  inspFlowReader.read();                   // inspiratory flow (SLPM)
  inspPressureReader.read();               // inspiratory pressure (cmH2O)
  reservoirPressureReader.readReservoir(); // gas reservoir pressure (cmH2O)

  //expiratory sensors
  expFlowReader.read();                   // expiratory flow (SLPM)
  expPressureReader.read();               // expiratory pressure (cmH2O)
}

/** 
 * function to check if sensor readings are within acceptable ranges
 * activate an alarm if they are not and deactivate the alarm once they are again
 * 
 * @param reading -- sensor reading
 * @param compareValue -- value to compare reading to
 * @param sensitivity -- the range of error
 * @param highAlarmCode -- the high alarm code for a specific sensor
 * @param lowAlarmCode -- the low alarm code for a specific sensor
 */ 
void checkAlarmRangeWithUpdate(float reading, float &compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode) { 
  // if not the first reading, compare and alarm if abnormal
  if (!isnan(compareValue)) {
    if (reading > compareValue + sensitivity) { 
      alarmMgr.activateAlarm(highAlarmCode);
    } else if (reading < compareValue - sensitivity) { 
      alarmMgr.activateAlarm(lowAlarmCode);
    } else {
      alarmMgr.deactivateAlarm(lowAlarmCode); 
      compareValue = reading; // update to remember value for next comparison
    }
  } else {
    compareValue = reading; // update to remember value for next comparison
  }
}

/** 
 * check against a value that doesn't need to be stored -- call above with dummy variable
 */ 
void checkAlarmRange(float reading, float compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode){ 
  float dummyCompareValue = compareValue;
  checkAlarmRangeWithUpdate(reading, dummyCompareValue, sensitivity, highAlarmCode, lowAlarmCode);
}

/** 
 * check for errors in sensor readings and take appropriate action
 */ 
void checkSensorReadings() {
  checkAlarmRangeWithUpdate(inspPressureReader.peak(), lastPeak, INSP_PRESSURE_SENSITIVITY, ALARM_INSP_HIGH, ALARM_INSP_LOW);
  checkAlarmRange(tidalVolumeInsp, display.volume(), display.volume()/TIDAL_VOLUME_SENSITVITY, ALARM_TIDAL_HIGH, ALARM_TIDAL_LOW);
}

// PS algorithm
void pressureSupportStateMachine();

// VC algorithm
void volumeControlStateMachine();

//-------------------Set Up--------------------
void setup() {
  Serial.begin(115200);   // open serial port for debugging

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

  // setup PID controller (for VC mode, the default mode)
  inspValve.initializePID(OUTPUT_MIN, OUTPUT_MAX, SAMPLE_TIME); 
  inspValve.previousPosition = DEFAULT_VALVE_POSITION;     

  // warm up SV3 valve by opening it to unstick it
  analogWrite(SV3_CONTROL, 255);
  delay(35);
  analogWrite(SV3_CONTROL, 0);

  // set to VC_MODE (@FutureWork: ideally this would be indicated through the UI startup sequence)
  ventMode = VC_MODE;   // for testing VC mode only
  expValve.close();     // close exp valve initially
  setState(OFF_STATE);  // start in OFF_STATE

  // calibrate flow meters -- seems to change when SV4 closes
  inspFlowReader.calibrateToZero(); // set non-flow analog readings as the 0 in the flow reading functions
  expFlowReader.calibrateToZero();  

  // @FutureWork: implement startup sequence on display
  // display.start();

  cycleTimer = millis(); // begin breath cycle timer
}


//-------------------Run Forever--------------------
void loop() {
  display.listen(); // listen for interactions with display

  // check if the user has indicated standby mode (to turn ventilator off)
  if (display.isTurnedOff()) {
    setState(OFF_STATE);
    alarmMgr.activateAlarm(ALARM_SHUTDOWN); // activate shutdown alarm
  }

  readSensors(); 

  // @FutureWork: We only alarm after first 5 breaths (this is a "warm up" issue where it takes time to stabilize)
  if (cycleCount > 5){  
    checkSensorReadings();     
    alarmMgr.maintainAlarms(); // maintain onging alarms 
  }
  
  display.updatePressureWave(inspPressureReader.get()); 

  // manage reservoir refilling based on FIO2 concentration set by user on the display
  o2Management(display.oxygen());

  // go to VC State machine
  volumeControlStateMachine();
}


//////////////////////////////////////////////////////////////////////////////////////
// VOLUME CONTROL STATE MACHINE
//////////////////////////////////////////////////////////////////////////////////////

// States
States state;

void setState(States newState) {
  state = newState;
}

/**
 * This is normally the first state
 * @FutureWork: implement a turnOffAll method in AlarmManager class to be used here
 */ 
void beginOff() {
  inspValve.endBreath(); // close the inspiratory valve
  expValve.open();       // keep expiratory valve open for safety (also does not use as much power)
}

/**
 * Runs during any transition to the INSP_STATE from any other state
 */  
void beginInspiration() {
  cycleDuration = millis() - cycleTimer;   // calculate the length of the last breath
  cycleTimer = millis();                   // reset the cycle timer at the start of inspiration

  // record values from last breath
  expDuration = cycleTimer - expTimer;        // measured duration of last expiration (EXP_STATE + PEEP_PAUSE + EXP_HOLD)
  tidalVolumeExp = expFlowReader.getVolume(); // set current inspired volume as the measured inspiratory tidal volume for the last breath

  // calculate actual minute volume from last breath
  const float minuteVolume = inspFlowReader.getVolume() * CC_PER_MS_TO_LPM / cycleDuration;

  // Update patient data on display to reflect values from last breath
  display.writePeak(inspPressureReader.peak());        // measured pip cmH2O
  display.writePlateau(inspPressureReader.plateau());  // measured plateau (only measured if HOLD_INSP_STATE is activated) cmH2O
  display.writePeep(expPressureReader.peep());         // measured PEEP cmH2O
  display.writeVolumeExp(expFlowReader.getVolume());   // measured expired volume
  display.writeMinuteVolume(minuteVolume);             // measured minute volume
  display.writeBPM(60000.0/cycleDuration);             // measured respiratory rate (seconds)
  display.writeO2(O2);                                 // measured FIO2 concentration (@FutureWork: pending addition of O2 sensor)  

  // close expiratory valve
  expValve.close();

  // Compute intervals at current settings
  unsigned long targetCycleDuration = 60000UL / display.bpm(); // ms from start of cycle to end of inspiration
  targetInspDuration = 105 * targetCycleDuration * display.inspPercent() / 10000; // allowing a bit more time to complete tidal volume inhailation
  targetCycleEndTime = cycleTimer + targetCycleDuration;                          // target time for breath to end (for HOLD_EXP_STATE to end)
  targetInspEndTime  = cycleTimer + targetInspDuration;                           // target time for INSP_STATE to end
  targetExpDuration  = targetCycleDuration - targetInspDuration - MIN_PEEP_PAUSE; // target time for EXP_STATE to end
  desiredInspFlow = display.volume() * CC_PER_MS_TO_LPM / targetInspDuration;     // desired inspiratory flowrate cc/ms

  // begin PID control based on desired flow and reset tidal volume
  inspValve.beginBreath(desiredInspFlow); 
  inspFlowReader.resetVolume();           
  cycleCount++;                           
}

/**
 * Run every time we transition to HOLD_INSP state
 */ 
void beginHoldInspiration() {
  // close inspiratory valve, turn off PID control and reset timer                 
  inspValve.endBreath();
  inspHoldTimer = millis();

  // Perform inspiration hold only once per button press on the UI
  display.resetInspHold();
}

/**
 * Run every time we enter EXP_STATE from any other state
 */ 
void beginExpiration() {
  inspPressureReader.setPeakAndReset(); // reset pip, cmH2O

  // record and display inspiratory tidal volume
  tidalVolumeInsp = inspFlowReader.getVolume(); 
  display.writeVolumeInsp(tidalVolumeInsp);     

  inspValve.endBreath(); // close insp valve and turn off PID control
  expValve.open();       // open expiration valve
  expTimer = millis();   // reset  timer

  // calculate 80% of inspired volume, whic is the condition to leave this state
  targetExpVolume = inspFlowReader.getVolume() * 8 / 10; 
  targetExpEndTime = expTimer + targetExpDuration;       

  expFlowReader.resetVolume();
}

/**
 * run every time we enter PEEP_PAUSE state
 */ 
void beginPeepPause() {
  peepPauseTimer = millis(); // reset timer
}

/**
 * Volume Control state machine
 */ 
void volumeControlStateMachine(){
  switch (state) {
    case OFF_STATE:
      if (!display.isTurnedOff()) { 
        setState(INSP_STATE);
        beginInspiration();  // beging inspiration!
      }
      break;

    case INSP_STATE:     
      display.updateFlowWave(inspFlowReader.get());                           
      inspFlowReader.updateVolume();                                          

      // calculate if the INSP_STATE should time out
      bool timeout = (millis() >= targetInspEndTime + INSP_TIME_SENSITIVITY); 

      // transition out of INSP_STATE if we either the desired tidal volume or state timed out
      if (inspFlowReader.getVolume() >= display.volume() || timeout) { 
        if (timeout) {
          // this means we didn't reach tidal volume so trigger alarm
          alarmMgr.activateAlarm(ALARM_TIDAL_LOW); 
        } else if(alarmMgr.alarmStatus(ALARM_TIDAL_LOW)){
          alarmMgr.deactivateAlarm(ALARM_TIDAL_LOW);      //otherwise deactivate a low tidal volume alarm if one is currently on
        }

        // if user turned on inspiratory hold, transition to INSP_HOLD_STATE
        if (display.inspHold()) { 
          setState(HOLD_INSP_STATE); 
          beginHoldInspiration(); 
        } else {
          // otherwise, transition to EXP_STATE
          setState(EXP_STATE);
          beginExpiration();
        }

        inspDuration = millis() - cycleTimer; // Record length of inspiration
      } else {
        // if transition criteria is not met, keep adjusting inspiratory valve
        inspValve.maintainBreath(cycleTimer);
      }
      break;

    case HOLD_INSP_STATE:
      display.updateFlowWave(inspFlowReader.get()); 

      // if we reached the end time for HOLD_INSP_STATE
      if (HOLD_INSP_DURATION <= millis() - inspHoldTimer) {
        inspPressureReader.setPlateau();
        // check plateau pressure range and alarm if abnormal  
        if (inspPressureReader.plateau() > PPLAT_MAX) { 
         alarmMgr.activateAlarm(ALARM_PPLAT_HIGH);
        } else if (alarmMgr.alarmStatus(ALARM_PPLAT_HIGH)) {
          alarmMgr.deactivateAlarm(ALARM_PPLAT_HIGH);
        }

        setState(EXP_STATE); // switch to EXP_STATE
        beginExpiration();   
      }
      break;

    case EXP_STATE:
      // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
      display.updateFlowWave(expFlowReader.get() * -1); 
      expFlowReader.updateVolume();                     
      
      // if 80% of inspired volume has been expired, transition to PEEP_PAUSE_STATE 
      if (expFlowReader.getVolume() >= targetExpVolume || millis() > targetExpEndTime + EXP_TIME_SENSITIVITY){ 
        setState(PEEP_PAUSE_STATE); 
        beginPeepPause();
      }
      break;

    case PEEP_PAUSE_STATE:
      // To update flow graph, flip sign of expiratory flow sensor to show flow out of lungs 
      display.updateFlowWave(expFlowReader.get() * -1); 
      expFlowReader.updateVolume();                     
      
      // if the PEEP pause time has run out, transition to HOLD_EXP_STATE
      if (millis() - peepPauseTimer >= MIN_PEEP_PAUSE) {
        expPressureReader.setPeep(); 
        setState(HOLD_EXP_STATE);    
      }
      break;

    case HOLD_EXP_STATE: {
      display.updateFlowWave(expFlowReader.get() * -1); //update flow waveform on display
      expFlowReader.updateVolume();                     //update expiratory flow counter

      // Check if patient triggers inhale or state timed out 
      bool patientTriggered = expPressureReader.get() < expPressureReader.peep() - display.sensitivity();
      bool timeout = (millis()  > targetCycleEndTime); 

      if (patientTriggered || timeout) { 
        beginInspiration();   
        setState(INSP_STATE);
      }

    } break;
  } // End switch
}
