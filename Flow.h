/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 */
 
#ifndef Flow_h
#define Flow_h

#include "Arduino.h"
#include "Constants.h"

class Flow {
public:
  Flow(int pin): 
    sensor_pin_(pin),
    flow_rate_(0.0) {}

  // Get flow reading
  void read() {
    // read the voltage
    int V = analogRead(sensor_pin_); 

    float Fmax = 150;     // max flow in SLPM
    float Vmax = 1023;    // max voltage in range from analogRead
    float Vsupply = 5.0;  // voltage supplied
    float sensorRange = (4.5 - 0.5); // voltage range returned by sensor
  
    // convert to flow rate at standard temperature and pressure
    // sensor_read(0.5-4.5 V) maps linearly to flow_rate_(0-150 SLPM)
    float flow_rate_ = Vsupply * V/Vmax * Fmax/sensorRange - 18.75; 
  }

  const float& get() {
    return flow_rate_;
  }

private:
  int sensor_pin_;
  float flow_rate_;
};

#endif
 
