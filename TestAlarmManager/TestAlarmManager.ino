#include "AlarmManager.h"

AlarmManager amgr;

int test = 0;

void setup() {
  delay(10000);
}

void loop() {
  // put your main code here, to run repeatedly:
  amgr.maintainAlarms();

  // More thorough testing wanted but this is a start...
  switch (test) {
    // Begin with individual alarm tests:
    case 0:  // turn on low tidal alarm at 1 seconds
      if (millis()>10000) {
        amgr.activateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
    case 1:  // turn off low tidal alarm at 5 seconds
      if (millis()>15000) {
        amgr.deactivateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
    case 2:  // turn on low insp pressure alarm at 6 seconds
      if (millis()>16000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 3:  // turn off low insp pressure alarm at 25 seconds
      if (millis()>35000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 4:  // turn on apnea alarm at 26 seconds
      if (millis()>36000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 5:  // turn off apnea alarm at 54 seconds
      if (millis()>54000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;

    // Now try two alarms at once:
    case 6:  // turn on low insp pressure alarm at 56 seconds
      if (millis()>56000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 7:  // turn on apnea alarm at 57 seconds
      if (millis()>57000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 8:  // turn off apnea alarm at 66 seconds
      if (millis()>66000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 9:  // turn off low insp pressure alarm at 70 seconds
      if (millis()>70000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 10:  // turn on apnea alarm at 72 seconds
      if (millis()>72000) {
        amgr.activateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 11:  // turn on low insp pressure alarm at 73 seconds
      if (millis()>73000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 12:  // turn off low insp pressure alarm at 78 seconds
      if (millis()>78000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 13:  // turn off apnea alarm at 85 seconds
      if (millis()>85000) {
        amgr.deactivateAlarm(ALARM_APNEA);
        test++;
      }
      break;
    case 14:  // turn on low tidal alarm at 86 seconds
      if (millis()>86000) {
        amgr.activateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
    case 15:  // turn on low insp pressure alarm at 87 seconds
      if (millis()>87000) {
        amgr.activateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 16:  // turn off low insp pressure alarm at 90 seconds
      if (millis()>90000) {
        amgr.deactivateAlarm(ALARM_INSP_LOW);
        test++;
      }
      break;
    case 17:  // turn off low tidal alarm at 95 seconds
      if (millis()>95000) {
        amgr.deactivateAlarm(ALARM_TIDAL_LOW);
        test++;
      }
      break;
  }
}
