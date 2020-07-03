#ifndef BREATH_DATA_H
#define BREATH_DATA_H

#include "Constants.h"
#include "AlarmManager.h"

class BreathData {
  public:
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
    float targetExpVolume    = 0; // minimum target volume for expiration

    float tidalVolumeInsp = 0.0; // measured inspiratory tidal volume
    float tidalVolumeExp = 0.0;  // measured expiratory tidal volume

    float lastPeep = 0.0/0.0; // PEEP from last breath
    float lastPeak = 0.0/0.0; // peak pressure from last breath

    static unsigned long cycleCount; // number of breaths (including current breath)

    // update functions
    void beginInspiration();
    void beginInspiratorySustain();
    void beginHoldInspiration();
    void beginExpiration();
    void beginPeepPause();
    void beginHoldExpiration();

    // monitoring functions
    void checkSensorReadings();

  private:
    void checkAlarmRange(float reading, float compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode);
    void checkAlarmRangeWithUpdate(float reading, float &compareValue, float sensitivity, alarmCode highAlarmCode, alarmCode lowAlarmCode);
};

#endif
