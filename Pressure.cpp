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
  int R = analogRead(sensor_pin_);

  static const float mBarTocmH2O = 1.01972;

  static const float Pmin = -163.155 * mBarTocmH2O;   // pressure max in cmH2O
  static const float Pmax = 163.155 * mBarTocmH2O;    // pressure min in cmH2O
  static const float Prange = Pmax - Pmin;
  const unsigned long Vsupply     = 5000;                    // voltage supplied, mv
  const unsigned long sensorMin   = 1023UL * 500 / Vsupply;  // Sensor value at 500 mv
  const unsigned long sensorMax   = 1023UL * 4500 / Vsupply; // Sensor value at 4500 mv
  const unsigned long sensorRange = sensorMax - sensorMin;

  // convert to pressure
  float pressure = (R - sensorMin) * (Prange / sensorRange) + Pmin;

  // update peak and reset
  current_peak_ = max(current_peak_, pressure);
  Serial.print("pressure ="); //@debugging
  Serial.print("\t");
  Serial.println(pressure);
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
