#include "Constants.h"
#include "Display.h"
#include "Flow.h"
#include "Pressure.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "BreathData.h"

/**
 *  Sets up for a new breath.
 *  Called by InsStateVC.enter()
 */  
void BreathData::beginInspiration() {
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
 * function to check if sensor readings are within acceptable ranges
 * activate an alarm if they are not and deactivate the alarm once they are again
 * 
 * @param reading -- sensor reading
 * @param compareValue -- value to compare reading to
 * @param sensitivity -- the range of error
 * @param highAlarmCode -- the high alarm code for a specific sensor
 * @param lowAlarmCode -- the low alarm code for a specific sensor
 */ 
void BreathData::checkAlarmRangeWithUpdate(float reading, float &compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode) { 
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
void BreathData::checkAlarmRange(float reading, float compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode){ 
  float dummyCompareValue = compareValue;
  checkAlarmRangeWithUpdate(reading, dummyCompareValue, sensitivity, highAlarmCode, lowAlarmCode);
}

/** 
 * check for errors in sensor readings and take appropriate action
 */ 
void BreathData::checkSensorReadings() {
  checkAlarmRangeWithUpdate(inspPressureReader.peak(), lastPeak, INSP_PRESSURE_SENSITIVITY, ALARM_INSP_HIGH, ALARM_INSP_LOW);
  checkAlarmRange(tidalVolumeInsp, display.volume(), display.volume()/TIDAL_VOLUME_SENSITVITY, ALARM_TIDAL_HIGH, ALARM_TIDAL_LOW);
}


/**
 *  Set up for inspiratory hold.
 *  Called by InsHoldStateVC.enter()
 */  
void BreathData::beginHoldInspiration() {
  // close inspiratory valve, turn off PID control and reset timer                 
  inspValve.endBreath();
  inspHoldTimer = millis();

  // Perform inspiration hold only once per button press on the UI
  display.resetInspHold();
}

/**
 *  Set up for expiration.
 *  Called by ExpStateVC.enter()
 */  
void BreathData::beginExpiration() {
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
 *  Set up for PEEP pause.
 *  Called by PeepStateVC.enter()
 */  
void BreathData::beginPeepPause() {
  peepPauseTimer = millis(); // reset timer
}

/**
 *  Set up for expiratory hold.  (Currently nothing to do here.)
 *  To be called by ExpHoldStateVC.enter() if necessary
 */  
void BreathData::beginHoldExpiration() {
  // Nothing to do when entering this state
}
