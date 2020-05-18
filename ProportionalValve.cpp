#include "ProportionalValve.h"
#include "PID_v1.h"


void ProportionalValve::setGains(double kp, double ki, double kd) {
  kp_ = kp;
  ki_ = ki;
  kd_ = kd;
}

// move proportional valve (in mm)
void ProportionalValve::move(float increment) {
  // increment position
  position_ += increment;

  float valveRange = 5;      // (0-5 V) signal to valve
  float orificeSize = 16.51; // size for iQ valves 700048 in mm

  // calculate voltage and move to new position
  float voltage = (valveRange/orificeSize) * position_ ; 
  analogWrite(valve_pin_, voltage);       
}

/**
 * Trigger inspiration 
 */ 
void ProportionalValve::beginBreath(float tInsp, float setVT) {
  // set setpoint to desired inspiratory flow rate ()
  float pid_setpoint_ = setVT/tInsp;
  
  // turn on PID to open valves
  pid_control_.SetMode(AUTOMATIC);

  pid_control_.Compute(); // do a round of inspiratory PID computing
  move(pid_output_);      // move according to the position calculated by the PID controller
}

/**
 * Trigger expiration
 */ 
void ProportionalValve::endBreath(float tInsp, float setVT) {
  // set setpoint to desired inspiratory flow rate 
  // not sure if this needs to be here
  float pid_setpoint_ = setVT/tInsp;
  
  // turn off insp PID computing
  pid_control_.SetMode(MANUAL);

  move(-1 * position()); // close valve by reseting position, should be 0
}
