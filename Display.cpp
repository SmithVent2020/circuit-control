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
  locked = false;
}

void Display::init() {
  nexInit(115200);

  // dbSerial is for debugging screen
  dbSerialPrintln("setup done");

  hold.attachPop(holdPopCallback, &hold);
  lock.attachPop(lockPopCallback, &lock);

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

// -----------------
// waveforms
// -----------------
void Display::updateFlowWave(float flow) {
  uint8_t val = map(flow, FLOW_RANGE_MIN, FLOW_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrintln("Flow waveform value------");
  dbSerialPrintln(val);
  flowWave.addValue(0, flowSmoother.smooth(val));
}

void Display::updatePressureWave(float pressure) {
  uint8_t val = map(pressure, PRESSURE_RANGE_MIN, PRESSURE_RANGE_MAX, GRAPH_MIN, GRAPH_MAX);
  dbSerialPrintln("Pressure waveform value------");
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
  dbSerialPrintln(settings.volume);

  RRText.getText(buffer, sizeof(buffer));
  settings.bpm = atoi(buffer);
  dbSerialPrintln(settings.bpm);

  O2Text.getText(buffer, sizeof(buffer));
  settings.o2 = atoi(buffer);
  dbSerialPrintln(settings.o2);

  IEText.getText(buffer, sizeof(buffer));
  sscanf(buffer, "%d, %d", &settings.ie[0], settings.ie[1]);

  SenText.getText(buffer, sizeof(buffer));
  settings.sensitivity = atof(buffer);
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


// Display
Display display;
