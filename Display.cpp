#include "Display.h"
#include "Constants.h"
#include "AlarmManager.h"

char buffer[20];
char buffer2[20];

/*
 * Initialize values
 */
Display::Display() {
  settings.o2 = O2;
  settings.sensitivity = SENSITIVITY;
  settings.bpm = BPM;
  settings.ie[0] = IE_INSP;
  settings.ie[1] = IE_EXP; 
  settings.volume = TIDAL_VOLUME;
  settings.inspHold = false;
  settings.peak = PS;
  settings.apnea = APNEA_BACKUP/1000;
  settings.cycleOff = CYCLE_OFF;
  settings.riseTime = RISE_TIME/1000;

  turnOff = false;
  locked = false;
}

void Display::init() {
  nexInit(115200);

  // dbSerial is for debugging screen
  dbSerialPrintln("setup done");

  hold.attachPop(holdPopCallback, &hold);
  lock.attachPop(lockPopCallback, &lock);
  bell.attachPush(bellPushCallback, &bell);

  nex_listen_list[0] = &hold;
  nex_listen_list[1] = &lock;
  nex_listen_list[2] = NULL; 
}

void Display::listen() {
  nexLoop(nex_listen_list);
}

void Display::resetInspHold() {
  settings.inspHold = false;
}

void Display::showAlarm(const char *buffer, int priority) {
  sendCommand("vis 1,1");

  if(priority == 0) {
    banner.Set_background_color_bco(63488);
  } else {
    banner.Set_background_color_bco(65504);
  }

  banner.setText(buffer);
}

void Display::stopAlarm() {
  sendCommand("vis 1,0");
}

// -----------------
// waveforms
// -----------------
void Display::updateFlowWave(float flow) {
  uint8_t val = map(flow, FLOW_RANGE_MIN, FLOW_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrint("Flow waveform value=");
  dbSerialPrintln(val);
  flowWave.addValue(0, flowSmoother.smooth(val));
}

void Display::updatePressureWave(float pressure) {
  uint8_t val = map(pressure, PRESSURE_RANGE_MIN, PRESSURE_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrint("Pressure waveform value=");
  dbSerialPrintln(val);
  pressureWave.addValue(0, pressureSmoother.smooth(val));
}

// -----------------
// patient data
// -----------------
void Display::writePeak(float peak) {
  dtostrf(peak, 4, 1, buffer);
  pip.setText(buffer);
}

void Display::writePlateau(float pressure) {
  dtostrf(pressure, 4, 1, buffer);
  plat.setText(buffer);
} 

void Display::writePeep(float pressure) {
  dtostrf(pressure, 3, 1, buffer);
  peep.setText(buffer);
}

void Display::writeVolumeInsp(float volumeInsp) {
  dtostrf(volumeInsp, 5, 1, buffer);
  VTi.setText(buffer);
}

void Display::writeVolumeExp(float volumeExp) {
  dtostrf(volumeExp, 5, 1, buffer);
  VTe.setText(buffer);
}

void Display::writeMinuteVolume(float minuteVolume) {
  dtostrf(minuteVolume, 4, 1, buffer);
  mv.setText(buffer);
}

void Display::writeBPM(float bpm) {
  dtostrf(bpm, 4, 1, buffer);
  rr.setText(buffer);
}
 
void Display::writeO2(int oxygen) {
  dtostrf(oxygen, 4, 1, buffer);
  o2.setText(buffer);
}

void Display::updateValues() {
  VTText.getText(buffer, sizeof(buffer));
  settings.volume = atoi(buffer);
  dbSerialPrint("Volume setting=");
  dbSerialPrintln(settings.volume);

  RRText.getText(buffer, sizeof(buffer));
  settings.bpm = atoi(buffer);
  dbSerialPrint("BPM setting=");
  dbSerialPrintln(settings.bpm);

  O2Text.getText(buffer, sizeof(buffer));
  settings.o2 = atoi(buffer);
  dbSerialPrint("O2 setting=");
  dbSerialPrintln(settings.o2);

  IEText.getText(buffer, sizeof(buffer));
  sscanf(buffer, "%d:%d", &settings.ie[0], settings.ie[1]);
  dbSerialPrint("O2 setting=");
  dbSerialPrint(settings.ie[0]);
  dbSerialPrint(":");
  dbSerialPrintln(settings.ie[1]);

  SenText.getText(buffer, sizeof(buffer));
  settings.sensitivity = atof(buffer);
  dbSerialPrint("Sensitivity setting=");
  dbSerialPrintln(settings.sensitivity);
}

void holdPopCallback(void *ptr) {
  dbSerialPrintln("Callback");
  dbSerialPrint("ptr=");
  dbSerialPrintln((uint32_t)ptr);

  display.setInspHold();
}

void lockPopCallback(void *ptr) {
  dbSerialPrintln("Callback");
  dbSerialPrint("ptr=");
  dbSerialPrintln((uint32_t)ptr);

  if(display.locked == false) {
    display.locked = true;
    display.updateValues();
  } else {
    display.locked = false;
  }
}

void bellPushCallback(void *ptr) {
  dbSerialPrintln("Callback");
  dbSerialPrint("ptr=");
  dbSerialPrintln((uint32_t)ptr);

  alarmMgr.silence(SILENCE_DURATION);
}


// Display
Display display;
