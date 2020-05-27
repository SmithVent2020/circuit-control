/**
 * Flow.h
 * Calculates and stores the key flow values in the breathing cycle.
 */
 
#ifndef Flow_h
#define Flow_h

#include "Arduino.h"

class Flow {
  public:
    Flow(int pin);
    void read();
    void setPeakFlow();
    void reset();

    void setPeakAndReset() {
      peak_flow_ = current_peak_;
      current_peak_ = 0.0;
    }

    float get() const { return flow_rate_; }
    float peak() const { return peak_flow_; }

  private:
    int sensor_pin_;
    float flow_rate_;
    
    // only needed for pressure support
    float peak_flow_; 
    float current_peak_;
};

#endif
 
