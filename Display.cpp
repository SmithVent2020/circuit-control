#include "Display.h"
#include "Constants.h"

char buffer[20];
char buffer2[20];

/*
 * Initialize values
 */
Display::Display() {
  settings.o2 = O2_MIN;
  settings.sensitivity = SENSITIVITY_MIN;
  settings.bpm = 20;
  settings.ie[0] = IE_INSP;
  settings.ie[1] = 2; 
  settings.volume = 400;
  settings.inspHold = false;
  settings.peak = PS_MIN;
  settings.apnea = APNEA_BACKUP/1000;
  settings.cycleOff = CYCLE_OFF;
  settings.riseTime = RISE_TIME/1000;

  dbSerialPrintln("---------volume---------");
  dbSerialPrintln(settings.volume);

  dbSerialPrintln("---------RR---------");
  dbSerialPrintln(settings.bpm);

  turnOff = false;
}

void Display::init() {
  nexInit(115200);

  // dbSerial is for debugging screen
  dbSerialPrintln("setup done");

  // show default values for settings
  showVCSettings();
}

void Display::showVCSettings() {
  itoa(settings.volume, buffer, 10); 
  dbSerialPrintln(buffer);
  VTText.setText(buffer);

  itoa(settings.bpm, buffer, 10);
  dbSerialPrintln(buffer);
  RRText.setText(buffer);

  itoa(settings.o2, buffer, 10);
  O2Text.setText(buffer);

  itoa(settings.ie[0], buffer, 10);
  itoa(settings.ie[1], buffer2, 10);
  strcat(buffer, ":");
  strcat(buffer, buffer2);
  IEText.setText(buffer);

  dtostrf(settings.sensitivity, 3, 1, buffer);
  SenText.setText(buffer);
}

void Display::resetInspHold() {
  settings.inspHold = false;
}

// -----------------
// waveforms
// -----------------
void Display::updateFlowWave(float flow) {
  uint8_t val = map(flow, FLOW_RANGE_MIN, FLOW_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrintln("Flow waveform value------");
  dbSerialPrintln(val);
//   flowWave.addValue(0, millis() % 4096 / 16);
  flowWave.addValue(val);
}

void Display::updatePressureWave(float pressure) {
  uint8_t val = map(pressure, PRESSURE_RANGE_MIN, PRESSURE_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrintln("Pressure waveform value------");
  dbSerialPrintln(val);
//   pressureWave.addValue(0, millis() % 4096 / 16);
  flowWave.addValue(val);
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


// Display
Display display;
