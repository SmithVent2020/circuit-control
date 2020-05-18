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
  Valve(ValveInfo valveInfo): 
    valve_pin_(valveInfo.pin),
    is_normally_open_(valveInfo.normallyOpen),
    state_(false) {}

  void open() {  
    is_normally_open_ ? digitalWrite(valve_pin_, LOW) : digitalWrite(valve_pin_, HIGH);
  }

  void close() {
    is_normally_open_ ? digitalWrite(valve_pin_, HIGH) : digitalWrite(valve_pin_, LOW);
  }

  const float& get() { return state_; }

private:
  int valve_pin_;
  bool is_normally_open_;
  bool state_; 
};

#endif
