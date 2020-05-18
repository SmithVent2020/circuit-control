/**
 * Pressure.h
 * Calculates and stores the key pressure values of the breathing cycle.
 */
 
#ifndef Proportional_Valve_h
#define Proportional_Valve_h

#include "Arduino.h"
#include "Constants.h"
#include "PID_v1.h"

class ProportionalValve {
  public:
    void setGains(double kp, double ki, double kd);
    void move(float increment);
    void beginBreath(float tInsp, float setVT);
    void endBreath(float tInsp, float setVT);
    
    const float& get() { return position_; }
    const float& position() { return position_; }
    const float& pidOutput() { return pid_output_; }

  private:
    int valve_pin_ = SV3_CONTROL;
    float position_ = 0.0;
    double pid_setpoint_;
    double pid_input_;
    double pid_output_;
    double kp_ = VKP;
    double ki_ = VKI;
    double kd_ = VKD;
    PID pid_control_ = PID(&pid_input_, &pid_output_, &pid_setpoint_, kp_, ki_, kd_, DIRECT);
};

#endif
