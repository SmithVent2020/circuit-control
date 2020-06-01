#include "Pressure.h"
#include "Constants.h"

/*
 * Initialize values
 */
Pressure::Pressure(int pin) {
  sensor_pin_ = pin;
  current_ = 0.0;
  current_peak_ = 0.0;
  peak_ = 0.0;
  plateau_ = 0.0;
  peep_ = 0.0;
}

/**
 * Get pressure reading
 */
void Pressure::read() {
  // read the voltage
  int V = analogRead(sensor_pin_);

  const float Pmin = -163.155;   // pressure max in mbar
  const float Pmax = 163.155;    // pressure min in mbar
  const float Vmax = 1023;       // max voltage in range from analogRead

  // convert to pressure
  float pressure = (5.0 * V/Vmax - 0.5) * (Pmax-Pmin)/4.0 + Pmin; //mmHg

  // convert to cmH20
  pressure *= 1.01972;

  // update peak and reset
  current_peak_ = max(current_peak_, pressure);
  current_ = pressure;
}

void Pressure::readReservoir(){
  int V = analogRead(sensor_pin_);
  float pressure = 70.307*100*(5.0*V/1023-0.25)/4.5;  // in cmH20 sensorRead(0.5-4.5 V) maps linearly to flow_read(+-1053.6 cmH2O)
  current_ = pressure;
}

// Known pressure sensors
Pressure inspPressureReader(PRESSURE_INSP);
Pressure expPressureReader(PRESSURE_EXP);
Pressure reservoirPressureReader(PRESSURE_RESERVOIR);
