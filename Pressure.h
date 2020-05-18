/**
 * Pressure.h
 * Calculates and stores the key pressure values of the breathing cycle.
 */
 
#ifndef Pressure_h
#define Pressure_h

#include "Arduino.h"

class Pressure {
public:
  Pressure(int pin): 
    sensor_pin_(pin),
    current_(0.0),
    current_peak_(0.0),
    peak_(0.0),
    plateau_(0.0),
    peep_(0.0) {}

  // Get pressure reading
  void read() {
    // read the voltage
    int V = analogRead(sensor_pin_); 

    float Pmin = -163.155;   // pressure max in mbar
    float Pmax = 163.155;    // pressure min in mbar
    float Vmax = 1023;       // max voltage in range from analogRead
    
    // convert to pressure
    float pressure = (5.0 * V/Vmax - 0.5) * (Pmax-Pmin)/4.0 + Pmin; //mmHg

    // convert to cmH20
    pressure *= 1.01972;

    // update peak
    current_peak_ = max(current_peak_, pressure);

    current_ = pressure;
  }

  const float& get() {
    return current_;
  }

  void setPeakAndReset() {
    peak_ = current_peak_;
    current_peak_ = 0.0;
  }

  void setPlateau() {
    plateau_ = get();
  }

  void setPeep() {
    peep_ = get();
  }

  const float& peak() { return peak_; }
  const float& plateau() { return plateau_; }
  const float& peep() { return peep_; }

private:
  int sensor_pin_;
  float current_;
  float current_peak_;
  float peak_, plateau_, peep_;
};

#endif
