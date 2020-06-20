#include "ProportionalValve.h"
#include "PID_v1.h"
#include "Flow.h"

unsigned long nextPID = 0;

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
void ProportionalValve::move() {
  pid_input_ = inspFlowReader.get();
  controller.Compute();           // do a round of inspiratory PID computing
  position_ = (int)pid_output_;   // move based on PID output 
  analogWrite(valve_pin_, position_); 

}

/**
 * Trigger inspiration by starting PID control
 */
void ProportionalValve::beginBreath(float desiredSetpoint) {
  //implement burst to unstick SV3
  position_ = burst_amplitude_;
  analogWrite(SV3_CONTROL, burst_amplitude_);    // set SV3 all the way open
  delay(burst_time_);                            // wait for 15 milliseconds
  position_ = previousPosition;
  analogWrite(SV3_CONTROL, previousPosition);    // open SV3 to desired opening (calculated based on previous breath's opening)

  // set setpoint to desired inspiratory flow rate set tidal volume / desired inspiratory time
  pid_setpoint_ = desiredSetpoint;
  controller.SetMode(AUTOMATIC);
  active_memory_ = used_memory_ = 0; //reset PID memory
}

/**
 * Compute PID output and continue moving the valve
 */
void ProportionalValve::maintainBreath(unsigned long cycleTimer) {
  if (millis() - cycleTimer < burst_wait_) {
    // wait for initial burst to settle
    position_ = previousPosition;
    analogWrite(valve_pin_, position_); // move according to previous output
  } else if (controller.GetMode() == MANUAL) {
    // if the controller is turned off, turn it on and move the valve
    controller.SetMode(AUTOMATIC);
    move();
  } else {
    move();
  }

}

/**
 * Trigger expiration
 */
void ProportionalValve::endBreath() {
  previousPosition = pid_output_;  // save successfull position to begin with next loop

  // turn off insp PID computing and close valve
  controller.SetMode(MANUAL);    
  analogWrite(valve_pin_, 0);    
}

void ProportionalValve::initializePID(double outputMin, double outputMax, int sampleTime){
  controller.SetOutputLimits(outputMin, outputMax);
  controller.SetSampleTime(sampleTime);
}

float ProportionalValve::integrateReadings() {
  // get most recent flow reading in insp line
  PIDMemory[(active_memory_+used_memory_)%memory_length_] = inspFlowReader.get(); 
  if (used_memory_ == memory_length_) {
    active_memory_ = (active_memory_+1)%memory_length_;
  } else {
    used_memory_++;
  }
  float sum = 0;
  for (int i = 0; i < used_memory_; i++) {
    sum += PIDMemory[(active_memory_+i)%memory_length_];
  }
  return sum/used_memory_;
}

// Inspiration valve
ProportionalValve inspValve(SV3_CONTROL);
