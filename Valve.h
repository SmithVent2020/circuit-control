/**
 * Pressure.h
 * Calculates and stores the key pressure values of the breathing cycle.
 */

#ifndef Valve_h
#define Valve_h

#include "Arduino.h"
#include "Constants.h"

class Valve {
public:
  Valve(int pin, bool normallyOpen)
    : valve_pin_(pin)
    , is_normally_open_(normallyOpen)
    , state_(false) {}

  void open() {
    is_normally_open_ ? digitalWrite(valve_pin_, LOW) : digitalWrite(valve_pin_, HIGH);
  }

  void close() {
    is_normally_open_ ? digitalWrite(valve_pin_, HIGH) : digitalWrite(valve_pin_, LOW);
  }

  float get() const { return state_; }

private:
  int valve_pin_;
  bool is_normally_open_;
  bool state_;
};

// Valves
extern Valve oxygenValve;
extern Valve airValve;
extern Valve expValve;

#endif
