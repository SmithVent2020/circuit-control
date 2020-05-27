#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>

#include "Constants.h"

// enumerate alarm types
enum alarmCodes {
  ALARM_APNEA,         // 0
  ALARM_POWER_FAIL,    // 1
  ALARM_INLET_GAS,     // 2
  ALARM_INLET_O2,      // 3
  ALARM_BATTERY_LOW,   // 4
  ALARM_P1SENSOR_FAIL, // 5
  ALARM_P2SENSOR_FAIL, // 6
  ALARM_P3SENSOR_FAIL, // 7
  ALARM_INSP_HIGH,     // 8
  ALARM_PEEP_HIGH,     // 9
  ALARM_PEEP_LOW,      // 10
  ALARM_INSP_LOW,      // 11
  ALARM_TIDAL_HIGH,    // 12
  ALARM_TIDAL_LOW,     // 13
  ALARM_O2SENSOR_FAIL  // 14
};

// enumerate alarm priority
enum alarmIntensity {
  HIGH_PRIORITY, // 0
  MED_PRIORITY,   // 1
  LOW_PRIORITY   // 2
};

// Number of distinct alarm categories
const int N_ALARMS = 15;

// 2D array to keep track of alarms and their priority
const int ALARM_PRIORITY[N_ALARMS][2] = {
  {1, HIGH_PRIORITY},  // Apnea
  {2, HIGH_PRIORITY},  // Power supply fail
  {3, HIGH_PRIORITY},  // Inlet gas fail
  {3, HIGH_PRIORITY},  // Inlet O2 fail
  {4, HIGH_PRIORITY},  // Battery low
  {5, HIGH_PRIORITY},  // Pressure sensor fail
  {5, HIGH_PRIORITY},  // Pressure sensor fail
  {5, HIGH_PRIORITY},  // Pressure sensor fail
  {6, HIGH_PRIORITY},  // Insp pressure high
  {6, MED_PRIORITY},   // PEEP high
  {6, MED_PRIORITY},   // PEEP low
  {7, MED_PRIORITY},   // Insp pressure low
  {8, LOW_PRIORITY},   // Tidal volume high
  {8, LOW_PRIORITY},   // Tidal volume low
  {9, LOW_PRIORITY}    // O2 sensor fail
};

class AlarmManager {
  public:
    AlarmManager();                         // constructor
    void activateAlarm(int code);           // activates alarm with specified code
    void deactivateAlarm(int code);         // deactivates alarm with specified code
    bool alarmStatus(int code);             // returns true if specified alarm is on (even if silenced)
    void silence(unsigned int duration);    // silences current alarms for specified time (e.g. 2 minutes)
    bool isSilenced();                      // is an alarm currently active but not audible?
    int topAlarm();                         // returns code of current highest priority active alarm
    void maintainAlarms();                  // call every cycle to perform alarm maintenance & update

  private:  
    bool onPriority(alarmIntensity level);  // determines whether an alarm of specified priority is on
    void quellAlarm(int code);              // cease LED display and tone for this alarm
    void beginAlarm();                      // sets up the variables for alarm production
    bool geqPriority(int code1, int code2);  // determines if code1 is >= than code2
    bool timeIsNow(unsigned long t, unsigned long target);  // checks whether time has arrived

    // alarms initialized as 'OFF'
    bool alarms[N_ALARMS];// = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
    
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
