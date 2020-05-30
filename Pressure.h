/**
 * Pressure.h
 * Calculates and stores the key pressure values of the breathing cycle.
 */

#ifndef Pressure_h
#define Pressure_h

#include "Arduino.h"

class Pressure {
public:
  Pressure(int pin);

  void read();

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

  float get() const { return current_; }
  float peak() const { return peak_; }
  float plateau() const { return plateau_; }
  float peep() const { return peep_; }

private:
  int sensor_pin_;
  float current_;
  float current_peak_;
  float peak_, plateau_, peep_;
};

// Known pressure sensors;
extern Pressure inspPressureReader;
extern Pressure expPressureReader;
extern Pressure reservoirPressureReader;

#endif
