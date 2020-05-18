#include "AlarmManager.h"

/*
 *  Initializes with all alarms off
 */
AlarmManager::AlarmManager() {
  for (int i = 0; i < N_ALARMS; i++) {
    alarms[i] = false;
  }
  alarmSounding = alarmLED = false;
  alarmPhase = alarmPhaseLED = alarmNextTone = alarmNextBlink = 0;
}

/*
 * Returns code (index) of highest priority alarm currently active
 * or -1 if no alarm is currently set
 */
 int AlarmManager::topAlarm() {
   int result = -1;
   for (int i = 0; i < N_ALARMS; i++) {
     if (alarms[i] == true) {
       result = i;
       break; 
     }
   }
   return result;
 }

/*
 * Returns true if any alarm are set at given level, false otherwise
 */
 bool AlarmManager::onPriority(alarmIntensity level) {
   for (int i = 0; i < N_ALARMS; i++) {
     if ((alarms[i])&&(ALARM_PRIORITY[i][1]==level)) {
       return true;
     }
   }
   return false;
 }
     
bool AlarmManager::geqPriority(int code1, int code2) {
  // highest priority alarms have lowest numbers
    return (ALARM_PRIORITY[code1][1] <= ALARM_PRIORITY[code2][1]);
}

/*
 * Makes specified alarm active
 * 
 * Sets alarm status to true and unsilences alarm.
 * If this is the top priority alarm, sets onset timestamp & next tone
 * 
 * No effect if invalid code is supplied
 * 
 * @params code -- alarm index
 */
void AlarmManager::activateAlarm(int code) {
  if ((code>=0)&&(code<N_ALARMS)) {
    alarms[code] = true;
    int top = topAlarm();
    alarmLED = true;
    if (top == code) {
      alarmSounding = true;
      beginAlarm();
    }
  }
}

/*
 * Makes specified alarm inactive
 * 
 * Sets the specified alarm status to false.
 * Checks for remaining alarms.  If higher priority, no further action required.  
 * If lower priority alarm remains active, sets onset timestamp & next tone.
 * If no alarms remain active, sets alarmSounding and alarmLED to false.
 * 
 * No effect if alarm is already off or invalid code is supplied
 * 
 * @params code -- alarm index
 */
void AlarmManager::deactivateAlarm(int code) {
  if (alarmStatus(code) == true) {
    alarms[code] = false;
    int top = topAlarm();  // check for new top alarm
    if (top == -1) {
      // no alarms remain
      quellAlarm(code);  // cease LED display and tone for deactivated alarm
      alarmSounding = alarmLED = false;
      alarmRearm = 0xffffffff;
    } else if (geqPriority(top,code)) { 
      // lower priority alarm remains
      quellAlarm(code);  // cease LED display and tone for deactivated alarm
      beginAlarm();
    }  // else allow higher-priority alarm to continue
  }
}
     
/*
 * Returns status of specified alarm (true if on)
 * Always returns false for invalid code
 * 
 * @params code -- alarm index
 */
bool AlarmManager::alarmStatus(int code) {
  return  (code>=0)&&(code<N_ALARMS)&&alarms[code];
}

/*
 * Silences all current alarms for specified duration.
 * New alarm will override silence if they are higher priority than
 * any current.
 */
void AlarmManager::silence(unsigned int duration) {
  alarmSounding = false;
  noTone(BUZZER);  // stop auditory alarm but leave LED on
  unsigned long t = millis();
  alarmRearm = t+duration;
}

/*
 * Checks whether there is a currently active alarm that is silent
 */
bool AlarmManager::isSilenced() {
  // note that this will return TRUE for a low-priority alarm after the initial tone
  // not sure this is the desired behavior...
  return (alarmSounding = false)&&(topAlarm()!=-1);
}

/* 
 * Resets the alarm production variables
 */
void AlarmManager::beginAlarm() {
  alarmSounding = alarmLED = true;
  alarmNextTone = alarmNextBlink = millis();  // reset for new alarm
  alarmPhase = alarmPhaseLED = 0;
  alarmRearm = 0xffffffff;     
}

/*
 * Cease LED display and tone for this alarm.
 * No effect on alarm status.
 * 
 * @params code -- alarm index
 */
void AlarmManager::quellAlarm(int code) {
  // stop sound
  noTone(BUZZER);

  // stop LED
  switch (ALARM_PRIORITY[code][1]) {
    case HIGH_PRIORITY:
      digitalWrite(RED_LED, LOW);              
      break;
    case MED_PRIORITY:
      digitalWrite(YELLOW_LED, LOW);              
      break;
    case LOW_PRIORITY:
      break;
  }

 }

/**
 * Checks whether a specified time has elapsed.
 * 
 * Assuming that very large difference between time and target 
 * is result of rollover.
 * 
 * R/O t: current time
 * R/O targetTime:  target time
 */
bool AlarmManager::timeIsNow(unsigned long t, unsigned long targetTime) {
  return ((t >= targetTime)&&(t-targetTime < 0xf0000000));
}

/**
 * Updates alarm LED/buzzer status as necessary based on current top alarm and phase
 * 
 * Each priority level has a hard-coded pattern of sound & LED display
 * Additional patterns may be added as necessary
 * Could make this more general and express the display patterns as data
 * but this seems good enough for a small number of possibilities
 */
void AlarmManager::maintainAlarms() {
  unsigned long t = millis();

  // first check if there are silenced alarms that need to be reactivated
  if (timeIsNow(t,alarmRearm)) {
    beginAlarm();
  }
  if (alarmSounding == true) {
    int top = topAlarm();
    if (top > -1) {
      // audible alarms (current highest only):
      if (timeIsNow(t,alarmNextTone)) {
        switch (ALARM_PRIORITY[top][1]) {
          case HIGH_PRIORITY:
            // pattern is beep beep beep rest beep beep rest rest rest rest
            // each segment is 250 ms; beeps last for 150 ms at 880 Hz (A5)
            if (alarmPhase <= 2) {  // three beeps
              tone(BUZZER,880,150);
              alarmPhase += 1;
              alarmNextTone = t+250;
            } else if (alarmPhase == 3) {  // rest
              alarmPhase += 1;
              alarmNextTone = t+250;
            } else if (alarmPhase == 4) {  // first of beep pair
              tone(BUZZER,880,150);
              alarmPhase += 1;
              alarmNextTone = t+250;
            } else {  // second of beep pair
              tone(BUZZER,880,150);
              alarmPhase = 0;
              alarmNextTone = t+1250;
            }
            break;
          case MED_PRIORITY:
            // three beeps followed by four beats of silence
            // each segment is 250 ms; beeps last for 150 ms at 660 Hz (E5)
            if (alarmPhase == 0) {
              tone(BUZZER,660,150);
              alarmPhase = 1;
              alarmNextTone = t+250;
            } else if (alarmPhase == 1) {
              tone(BUZZER,660,150);
              alarmPhase = 2;
              alarmNextTone = t+250;
            } else {
              tone(BUZZER,660,150);
              alarmPhase = 0;
              alarmNextTone = t+1250;
            }
            break;
          case LOW_PRIORITY:
            // one 2000 ms tone, not repeated
            tone(BUZZER,440,2000);  // D3
            alarmNextTone = 0xffffffff;
            alarmSounding = false;
            break;
        }
      }
    } else {
      // no alarm detected -- reaching this point is probably a bug
      // because alarmSounding should only be true when there is an alarm set
      Serial.println("ERROR:  alarmSounding set but no alarm active.");
      alarmSounding = false;
    }
  if (alarmLED == true) {
      // visible alarms (both LED can blink if appropriate)
      if (timeIsNow(t,alarmNextBlink)) {
        // toggle active LED on/off every 500 ms
        if (onPriority(HIGH_PRIORITY)) {
            digitalWrite(RED_LED, alarmPhaseLED);   
        } else {
            digitalWrite(RED_LED, LOW);   
        }
        if (onPriority(MED_PRIORITY)) {
            digitalWrite(YELLOW_LED, alarmPhaseLED);   
        } else {
            digitalWrite(YELLOW_LED, LOW);   
        }
        alarmPhaseLED = !alarmPhaseLED;     
        alarmNextBlink = t+500;
      }
    } else {
      // no alarm detected -- reaching this point is probably a bug
      // because alarmLED should only be true when there is an alarm set
      Serial.println("ERROR:  alarmLED set but no alarm active.");
      alarmLED = false;
    }
  }
}
