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
    void move();
    void  setGains(double kp, double ki, double kd);
    void  beginBreath(float desiredFlow, unsigned long cycleTimer);
    void  maintainBreath(unsigned long cycleTimer);
    void  endBreath();
    float integrateReadings();
    void  initializePID(double outputMin, double outputMax, int sampleTime);
    float previousPIDOutput = 0;   //initial value chosen from previous tests (close to desired opening)(should be global)
    double desiredSetpoint = 0;

    float get() const { return position_; }
    float position() const { return position_; }
    
    

  private:
    int valve_pin_;
    float position_          = 0.0;
    double pid_setpoint_     = 10.0;  //set the setpoint to a lowish flowrate
    double pid_input_        = 0.0;   
    double pid_output_       = 0.0;  
    double kp_ = VKP;
    double ki_ = VKI;
    double kd_ = VKD;
    int active_memory_         = 0;     //number of active memory units
    int used_memory_           = 0;     //number of used memory units
    static const int burst_time_      = 15;    //milliseconds
    static const int burst_amplitude_ = 255;   //amount to open SV3 during burst
    static const int burst_wait_      = 100;   //milliseconds
    static const int memory_length_   = 4;     //length of PID output memory
    float PIDMemory[memory_length_];    //create an arrat of length memory_length_
    
    
    PID controller = PID(&pid_input_, &pid_output_, &pid_setpoint_, kp_, ki_, kd_, DIRECT);
    void move(float increment);
};

// Inspiration valve
extern ProportionalValve inspValve;

#endif
