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
 *  Returns the priority associated with the given alarm code
 * 
 *  @param code:  alarm we want the priority of
 *  @returns: the alarms priority (high, medium, low)
 */
alarmPriority AlarmManager::getAlarmPriority(alarmCode code) {
  if (code <= ALARM_MAX_HIGH_PRIORITY) {
    return HIGH_PRIORITY;
  } else if (code <= ALARM_MAX_MED_PRIORITY) {
    return MED_PRIORITY;
  } else if (code <= ALARM_MAX_LOW_PRIORITY) {
    return LOW_PRIORITY;
  } else {
    return NO_ALARM;
  }
}

/*
 * Returns code (index) of highest priority alarm currently active
 * or NORMAL_STATE if no alarm is currently set
 */
 alarmCode AlarmManager::topAlarm() {
   alarmCode result = NORMAL_STATE;
   for (int i = 0; i < N_ALARMS; i++) {
     if (alarms[i] == true) {
       result = (alarmCode)i;
       break; 
     }
   }
   return result;
 }

/*
 * Returns true if any alarm are set at given level, false otherwise
 */
 bool AlarmManager::onPriority(alarmPriority level) {
   for (int i = 0; i < N_ALARMS; i++) {
     if ((alarms[i])&&(getAlarmPriority(i)==level)) {
       return true;
     }
   }
   return false;
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
void AlarmManager::activateAlarm(alarmCode code) {
  if ((code>=0)&&(code<N_ALARMS)) {
    alarms[code] = true;
    alarmCode top = topAlarm();
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
void AlarmManager::deactivateAlarm(alarmCode code) {
  if (alarmStatus(code) == true) {
    alarms[code] = false;
    alarmCode top = topAlarm();  // check for new top alarm
    if (top == NORMAL_STATE) {
      // no alarms remain
      quellAlarm(code);  // cease LED display and tone for deactivated alarm
      alarmSounding = alarmLED = false;
      alarmRearm = 0xffffffff;
    } else if (top > code) {   // recall that lowest code is highest priority!
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
bool AlarmManager::alarmStatus(alarmCode code) {
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
void AlarmManager::quellAlarm(alarmCode code) {
  // stop sound
  noTone(BUZZER);

  // stop LED
  switch (getAlarmPriority(code)) {
    case HIGH_PRIORITY:
      digitalWrite(RED_LED, LOW);              
      break;
    case MED_PRIORITY:
      digitalWrite(YELLOW_LED, LOW);              
      break;
    case LOW_PRIORITY:
      digitalWrite(YELLOW_LED, LOW);              
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
        switch (getAlarmPriority(top)) {
          case HIGH_PRIORITY:
            // pattern is beep beep beep rest beep beep rest rest rest rest x2
            // each beat is 125 ms; beeps last for 75 ms at 880 Hz (A5)
            // ten beep pattern is followed by 6000 ms interval
            switch (alarmPhase) {
              case 0:
              case 1:
              case 3:
              case 5:
              case 6:
              case 8:
                tone(BUZZER,880,75);
                alarmPhase += 1;
                alarmNextTone = t+125;
              break;
              case 2:
              case 7:
                tone(BUZZER,880,75);
                alarmPhase += 1;
                alarmNextTone = t+250;
              break;
              case 4:
                tone(BUZZER,880,75);
                alarmPhase += 1;
                alarmNextTone = t+625;
              break;
              case 9:
                tone(BUZZER,880,75);
                alarmPhase = 0;
                alarmNextTone = t+6125;
              break;
            }
            break;
          case MED_PRIORITY:
            // three beeps followed by 12000 ms of silence
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
              alarmNextTone = t+12250;
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
        // high priority blinks red LED at 2 Hz
        if (onPriority(HIGH_PRIORITY)) {
            digitalWrite(RED_LED, 1-alarmPhaseLED%2);   
        } else {
            digitalWrite(RED_LED, LOW);   
        }
        // medium priority blinks yellow LED at 0.8 Hz
        if (onPriority(MED_PRIORITY)) {
            digitalWrite(YELLOW_LED, 1-alarmPhaseLED/5);   
        } else if (onPriority(LOW_PRIORITY)) {
            // low priority leaves yellow LED on solid
            digitalWrite(YELLOW_LED, HIGH);   
        } else {
            digitalWrite(YELLOW_LED, LOW);   
        }
        alarmPhaseLED = (alarmPhaseLED+1)%10;     
        alarmNextBlink = t+250;
      }
    } else {
      // no alarm detected -- reaching this point is probably a bug
      // because alarmLED should only be true when there is an alarm set
      Serial.println("ERROR:  alarmLED set but no alarm active.");
      alarmLED = false;
    }
  }
}
