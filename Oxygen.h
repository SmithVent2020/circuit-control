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
    Oxygen(): 
      sensor_pin_(O2_SENSOR) {}

    // Get oxygen concentration reading
    void read() {
      // may want to research a better approach than continuously switching reference
      
      // change analog pin reference voltage to 1.1V and discard first reading
      analogReference(INTERNAL1V1);
      analogRead(sensor_pin_);
    
      // read the voltage
      int V = analogRead(sensor_pin_); 

      float O2Max = 100;    // max oxygen percentage
      float Vmax = 1023;    // max voltage in range from analogRead
      float Vsupply = 1.1;  // supplied voltage 
      float sensorRange = (60 * 1000); // voltage range (0-60 mV) returned from sensor in V
    
      // convert to O2 percentage
      // sensor_read(0.0-60.0 mV) maps linearly to concentration_(0-100%)
      float concentration_ = 1.1 * V/Vmax * O2Max/sensorRange; 
    
      // change analog pin reference voltage back to 5.0 V
      // read and discard again to avoid next wrong reading due to reference change
      analogReference(DEFAULT); 
      analogRead(sensor_pin_);    
    }

    const float& get() {
      return concentration_;
    }

  private:
    int sensor_pin_;
    float concentration_;
};

#endif
