#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>
#include <limits.h>

#include "Constants.h"

// enumerate alarm types
enum alarmCode {
  ALARM_SHUTDOWN,      // 0 - HIGH
  ALARM_APNEA,         // 1 - HIGH
  ALARM_POWER_FAIL,    // 2 - HIGH
  ALARM_INLET_GAS,     // 3 - HIGH
  ALARM_INLET_O2,      // 4 - HIGH
  ALARM_BATTERY_LOW,   // 5 - HIGH
  ALARM_P1SENSOR_FAIL, // 6 - HIGH
  ALARM_P2SENSOR_FAIL, // 7 - HIGH
  ALARM_P3SENSOR_FAIL, // 8 - HIGH
  ALARM_INSP_HIGH,     // 9 - HIGH    insp pressure implemented
  ALARM_PEEP_HIGH,     // 10 - MEDIUM               implemented
  ALARM_PEEP_LOW,      // 11 - MEDIUM               implemented
  ALARM_INSP_LOW,      // 12 - MEDIUM insp pressure implemented
  ALARM_TIDAL_HIGH,    // 13 - MEDIUM               implemented
  ALARM_PPLAT_HIGH,    // 14 - MEDIUM
  ALARM_TIDAL_LOW,     // 15 - LOW                  implemented 
  ALARM_O2SENSOR_FAIL, // 16 - LOW    
  N_ALARMS,            // 17 Number of alarms
  ALARM_NONE,          // 18

  FIRST_ALARM = 0,

  // Alarm priority groups
  ALARM_MAX_HIGH_PRIORITY = ALARM_INSP_HIGH,
  ALARM_MAX_MED_PRIORITY = ALARM_TIDAL_HIGH,
  ALARM_MAX_LOW_PRIORITY = ALARM_O2SENSOR_FAIL
};

// Allow incrementing an `alarmCode` to get the next code.
inline alarmCode& operator++(alarmCode& code) { code = alarmCode(code + 1); return code; }

// enumerate alarm priority
enum alarmPriority {
  HIGH_PRIORITY, // 0
  MED_PRIORITY,  // 1
  LOW_PRIORITY,  // 2
  NO_ALARM       // 3
};

// text for display screen -- should objectify this eventually
static const char *alarmText[N_ALARMS] = {
  "Ventilation Shutdown",
  "Apnea Detected",
  "Power Failure",
  "Air Supply Disconnected",
  "Oxygen Supply Disconnected",
  "Low Battery",
  "Pressure Sensor Failure (Reservoir)",
  "Pressure Sensor Failure (Inspiration)",
  "Pressure Sensor Failure (Expiration)",
  "Excess Inspiratory Pressure",
  "High PEEP",
  "Low PEEP",
  "Low Inspiratory Pressure",
  "Tidal Volume High",
  "Plateau Pressure High",
  "Tidal Volume Low",
  "Oxygen Sensor Failure" 
};

class AlarmManager {
  public:
    AlarmManager();                         // constructor
    void activateAlarm(alarmCode code);     // activates alarm with specified code
    void deactivateAlarm(alarmCode code);   // deactivates alarm with specified code
    bool alarmStatus(alarmCode code);       // returns true if specified alarm is on (even if silenced)
    void silence(unsigned int durationMs);  // silences current alarms for specified time in ms (e.g. 2 minutes)
    bool isSilenced();                      // is an alarm currently active but not audible?
    alarmCode topAlarm();                   // returns code of current highest priority active alarm, else `ALARM_NONE`
    void maintainAlarms();                  // call every cycle to perform alarm maintenance & update
    alarmPriority getAlarmPriority(alarmCode code);  // returns the priority of the alarm

  private:
    bool onPriority(alarmPriority level);   // determines whether an alarm of specified priority is on
    void quellAlarm(alarmCode code);        // cease LED display and tone for this alarm
    void beginAlarm();                      // sets up the variables for alarm production
    bool timeIsNow(unsigned long t, unsigned long target);  // checks whether time has arrived

    // Alarm array: each entry is 'true' if the alarm is active, else 'false.
    bool alarms[N_ALARMS];

    // is there a currently sounding alarm?
    // may be false if no alarms set or if temporarily silenced
    bool alarmSounding = false;

    // is there an alarm currently displayed on LED?
    bool alarmLED = false;

    // keeps track of position in multi-tone alarm sequence
    int alarmPhase = 0;

    // keeps track of position in LED alarm sequence
    int alarmPhaseLED = 0;

    // timestamp for unsilencing alarm
    unsigned long alarmRearm = ULONG_MAX;

    // timestamp for next alarm tone
    unsigned long alarmNextTone = ULONG_MAX;

    // timestamp for next alarm tone
    unsigned long alarmNextBlink = ULONG_MAX;
};

// The alarm manager
extern AlarmManager alarmMgr;

#endif
