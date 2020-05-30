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
    ProportionalValve(int pin) : valve_pin_(pin) { }

    void setGains(double kp, double ki, double kd);
    void beginBreath(float tInsp, float setVT);
    void maintainBreath();
    void endBreath();

    float get() const { return position_; }
    float position() const { return position_; }

  private:
    int valve_pin_;
    float position_ = 0.0;
    double pid_setpoint_ = 0.0;
    double pid_input_ = 0.0;
    double pid_output_ = 0.0;
    double kp_ = VKP;
    double ki_ = VKI;
    double kd_ = VKD;
    PID controller = PID(&pid_input_, &pid_output_, &pid_setpoint_, kp_, ki_, kd_, DIRECT);

    void move(float increment);
};

// Inspiration valve
extern ProportionalValve inspValve;

#endif
