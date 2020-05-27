#include "ProportionalValve.h"
#include "PID_v1.h"

/**
 * Initializes PID control with constant gains
 */
// ProportionalValve::ProportionalValve() {
//   position_ = 0.0;
//   valve_pin_ = SV3_CONTROL;
// }

/**
 * Set PID gains to tuned kp, ki, and kd values
 */ 
void ProportionalValve::setGains(double kp, double ki, double kd) {
  kp_ = kp;
  ki_ = ki;
  kd_ = kd;
}

/**
 * Moves proportional valve given an increment in mm
 */ 
void ProportionalValve::move(float increment) {
  // increment position
  position_ += increment;

  float valveRange = 5;      // (0-5 V) signal to valve
  float orificeSize = 16.51; // size for iQ valves 700048 in mm

  // calculate voltage and move to new position
  float voltage = (valveRange/orificeSize) * position_ ;

  // analogWrite takes a value between 0-255, so does this need to change?
  analogWrite(valve_pin_, voltage);       
}

/**
 * Trigger inspiration by starting PID control
 */ 
void ProportionalValve::beginBreath(float tInsp, float setVT) {
  // set setpoint to desired inspiratory flow rate ()
  float pid_setpoint_ = setVT/tInsp;
  
  // turn on PID to open valves
  controller.SetMode(AUTOMATIC);

  // first valve opening
  maintainBreath();
}

/**
 * Compute PID output and continue moving the valve
 */ 
void ProportionalValve::maintainBreath() {
  controller.Compute(); // do a round of inspiratory PID computing
  move(pid_output_);      // move according to the position calculated by the PID controller
}

/**
 * Trigger expiration
 */ 
void ProportionalValve::endBreath() {  
  // turn off insp PID computing
  controller.SetMode(MANUAL);

  move(-1 * position()); // close valve by reseting position, should be 0
}
