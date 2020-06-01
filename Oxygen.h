/**
 * Oxygen.h
 * Calculates and stores the oxygen concentration value given to patient
 */

#ifndef Oxygen_h
#define Oxygen_h

#include "Arduino.h"
#include "Constants.h"

class Oxygen {
  public:
    Oxygen(int pin) : sensor_pin_(pin) { }
    int get() const { return concentration_; }
    void read();

  private:
    int sensor_pin_;
    int concentration_;
};

// The oxygen reader
extern Oxygen oxygenReader;

#endif
