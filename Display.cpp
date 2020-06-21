#include "Display.h"
#include "Constants.h"
#include "AlarmManager.h"

char buffer[20];
char buffer2[20];

/*
 * Initialize setting values
 */
Display::Display() {
  settings.o2 = O2;
  settings.sensitivity = SENSITIVITY;
  settings.bpm = BPM;
  settings.ie[0] = IE_INSP;
  settings.ie[1] = IE_EXP; 
  settings.volume = TIDAL_VOLUME;
  settings.inspHold = false;

  turnOff = false;
  locked = false;
}

/**
 * Initialize display callback and register buttons
 */ 
void Display::init() {
  nexInit(115200); // set baud rate to 115200

  hold.attachPop(holdPopCallback, &hold);
  lock.attachPop(lockPopCallback, &lock);
  bell.attachPush(bellPushCallback, &bell);

  nex_listen_list[0] = &hold;
  nex_listen_list[1] = &lock;
  nex_listen_list[2] = NULL; 
}

/**
 * Listen to callback events
 */  
void Display::listen() {
  nexLoop(nex_listen_list);
}

/**
 * Reset inspiratory hold once the system registers it
 */  
void Display::resetInspHold() {
  settings.inspHold = false;
}

/**
 * Show alarm banner with color based on priority
 */ 
void Display::showAlarm(const char *buffer, int priority) {
  sendCommand("vis 1,1");

  priority == 0 ? banner.Set_background_color_bco(63488) : banner.Set_background_color_bco(65504);
  banner.setText(buffer);
}

/**
 * Hide alarm banner when values return to normal
 */ 
void Display::stopAlarm() {
  sendCommand("vis 1,0");
}

// -----------------
// waveforms
// -----------------
void Display::updateFlowWave(float flow) {
  uint8_t val = map(flow, FLOW_RANGE_MIN, FLOW_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  flowWave.addValue(0, flowSmoother.smooth(val));
}

void Display::updatePressureWave(float pressure) {
  uint8_t val = map(pressure, PRESSURE_RANGE_MIN, PRESSURE_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
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

// Update setting values based on user input
void Display::updateValues() {
  VTText.getText(buffer, sizeof(buffer));
  settings.volume = atoi(buffer);

  RRText.getText(buffer, sizeof(buffer));
  settings.bpm = atoi(buffer);

  O2Text.getText(buffer, sizeof(buffer));
  settings.o2 = atoi(buffer);

  IEText.getText(buffer, sizeof(buffer));
  sscanf(buffer, "%d:%d", &settings.ie[0], settings.ie[1]);

  SenText.getText(buffer, sizeof(buffer));
  settings.sensitivity = atof(buffer);
}

// -----------------
// Button callbacks
// -----------------
void holdPopCallback(void *ptr) {
  display.setInspHold();
}

void lockPopCallback(void *ptr) {
  if (display.locked == false) {
    display.locked = true;
    display.updateValues();
  } else {
    display.locked = false;
  }
}

void bellPushCallback(void *ptr) {
  alarmMgr.silence(SILENCE_DURATION);
}

// Display
Display display;
