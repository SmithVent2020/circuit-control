#include "Constants.h"
#include "Display.h"
#include "Flow.h"
#include "Pressure.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "BreathData.h"

static unsigned long BreathData::cycleCount = 0; // number of breaths (including current breath)

void BreathData::beginInspiration() {
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

void BreathData::beginInspiratorySustain() {
  //run the following every time we transition to INSPIRATORY_SUSTAIN_STATE (PS mode only)
  //Serial.println("entering insp Sustain state"); //uncomment for @debugging @cleanup
  // Pressure has reached set point. Record peak flow.
  inspFlowReader.setPeakAndReset();
}

void BreathData::beginHoldInspiration() {
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

void BreathData::beginExpiration() {
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

void BreathData::beginPeepPause() {
  //run every time we enter PEEP_PAUSE state
  //Serial.println("entering PEEP Pause state"); //uncomment for @debugging
  peepPauseTimer = millis(); //reset peep pause timer
}

void BreathData::beginHoldExpiration() {
  // Nothing to do when entering  HOLD_EXP state
  //Serial.println("entering exp hold state"); //uncomment for @debugging @cleanup
}
