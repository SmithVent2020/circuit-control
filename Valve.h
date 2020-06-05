/**
 * Pressure.h
 * Calculates and stores the key pressure values of the breathing cycle.
 */

#ifndef Valve_h
#define Valve_h

#include "Arduino.h"
#include "Constants.h"

enum ValveState {
  CLOSED,   // 0
  OPEN  // 1
};

class Valve {
public:
  Valve(int pin, bool normallyOpen)
    : valve_pin_(pin)
    , is_normally_open_(normallyOpen)
    , state_(normallyOpen ? OPEN:CLOSED) {}

  void open() {
    //Serial.print("opeing on/off valve");
    state_ = OPEN;
    is_normally_open_ ? digitalWrite(valve_pin_, LOW) : digitalWrite(valve_pin_, HIGH);
  }

  void close() {
    //Serial.print("closing onn/off valve");
    state_ = CLOSED;
    is_normally_open_ ? digitalWrite(valve_pin_, HIGH) : digitalWrite(valve_pin_, LOW);
  }

  ValveState get() const { return state_; }

private:
  int valve_pin_;
  bool is_normally_open_;
  ValveState state_;
};

// Valves
extern Valve oxygenValve;
extern Valve airValve;
extern Valve expValve;

#endif
