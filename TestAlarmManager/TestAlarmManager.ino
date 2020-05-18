#include "AlarmManager.h"

AlarmManager amgr;

int test = 0;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  amgr.maintainAlarms();

  // More thorough testing wanted but this is a start...
  switch (test) {
    // Begin with individual alarm tests:
    case 0:  // turn on low tidal alarm at 1 seconds
      if (millis()>1000) {
        amgr.activateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
    case 1:  // turn off low tidal alarm at 4 seconds
      if (millis()>4000) {
        amgr.deactivateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
    case 2:  // turn on low insp pressure alarm at 5 seconds
      if (millis()>5000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 3:  // turn off low insp pressure alarm at 15 seconds
      if (millis()>15000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 4:  // turn on apnea alarm at 18 seconds
      if (millis()>18000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 5:  // turn off apnea alarm at 24 seconds
      if (millis()>24000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;

    // Now try two alarms at once:
    case 6:  // turn on low insp pressure alarm at 25 seconds
      if (millis()>25000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 7:  // turn on apnea alarm at 26 seconds
      if (millis()>26000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 8:  // turn off apnea alarm at 28 seconds
      if (millis()>28000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 9:  // turn off low insp pressure alarm at 30 seconds
      if (millis()>30000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 10:  // turn on apnea alarm at 32 seconds
      if (millis()>32000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 11:  // turn on low insp pressure alarm at 33 seconds
      if (millis()>33000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 12:  // turn off low insp pressure alarm at 34 seconds
      if (millis()>34000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 13:  // turn off apnea alarm at 35 seconds
      if (millis()>35000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;
  }
}
