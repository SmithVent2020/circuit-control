#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>

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
  ALARM_INSP_HIGH,     // 9 - HIGH
  ALARM_PEEP_HIGH,     // 10 - MEDIUM
  ALARM_PEEP_LOW,      // 11 - MEDIUM
  ALARM_INSP_LOW,      // 12 - MEDIUM
  ALARM_TIDAL_HIGH,    // 13 - MEDIUM
  ALARM_TIDAL_LOW,     // 14 - LOW
  ALARM_O2SENSOR_FAIL, // 15 - LOW
  N_ALARMS,            // 16
  NORMAL_STATE         // 17
};

// enumerate alarm priority
enum alarmPriority {
  HIGH_PRIORITY, // 0
  MED_PRIORITY,  // 1
  LOW_PRIORITY,  // 2
  NO_ALARM       // 3
};

enum alarmGroups {
  ALARM_MAX_HIGH_PRIORITY = ALARM_INSP_HIGH,
  ALARM_MAX_MED_PRIORITY = ALARM_TIDAL_HIGH,
  ALARM_MAX_LOW_PRIORITY = ALARM_O2SENSOR_FAIL
};

class AlarmManager {
  public:
    AlarmManager();                         // constructor
    void activateAlarm(alarmCode code);     // activates alarm with specified code
    void deactivateAlarm(alarmCode code);   // deactivates alarm with specified code
    bool alarmStatus(alarmCode code);       // returns true if specified alarm is on (even if silenced)
    void silence(unsigned int duration);    // silences current alarms for specified time (e.g. 2 minutes)
    bool isSilenced();                      // is an alarm currently active but not audible?
    alarmCode topAlarm();                   // returns code of current highest priority active alarm
    void maintainAlarms();                  // call every cycle to perform alarm maintenance & update
    alarmPriority getAlarmPriority(alarmCode code);  // returns the priority of the alarm

  private:  
    bool onPriority(alarmPriority level);  // determines whether an alarm of specified priority is on
    void quellAlarm(alarmCode code);        // cease LED display and tone for this alarm
    void beginAlarm();                      // sets up the variables for alarm production
    bool timeIsNow(unsigned long t, unsigned long target);  // checks whether time has arrived

    // alarms initialized as 'OFF'
    bool alarms[N_ALARMS];// = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
    
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
    unsigned long alarmRearm = 0xffffffff;
        
    // timestamp for next alarm tone
    unsigned long alarmNextTone = 0xffffffff;
    
    // timestamp for next alarm tone
    unsigned long alarmNextBlink = 0xffffffff;
};

#endif
