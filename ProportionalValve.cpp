#include "ProportionalValve.h"
#include "PID_v1.h"
#include "Flow.h"

unsigned long nextPID    = 0;

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
void ProportionalValve::move() {
  controller.Compute(); // do a round of inspiratory PID computing
  analogWrite(valve_pin_, pid_output_);      // move according to the position calculated by the PID controller
}

/**
 * Trigger inspiration by starting PID control
 */
void ProportionalValve::beginBreath(float desiredSetpoint) {

  //implement burst to unstick SV3
  analogWrite(SV3_CONTROL, burst_amplitude_);    //set SV3 all the way open
  delay(burst_time_);                      //wait for 15 milliseconds
  analogWrite(SV3_CONTROL, previousPIDOutput); //open SV3 all to desired opening (calculated based on previous breath's opening)

  // set setpoint to desired inspiratory flow rate set tidal volume/ desired inspiratory time
  float pid_setpoint_ = desiredSetpoint;
  controller.Initialize();
  active_memory_ = used_memory_ = 0; //reset PID memory
}

/**
 * Compute PID output and continue moving the valve
 */
void ProportionalValve::maintainBreath(unsigned long cycleTimer) {
  if(millis() - cycleTimer < burst_wait_){
    //wait for initial burst to settle
    Serial.println("waiting for burst to settle"); //@debugging
    analogWrite(valve_pin_, previousPIDOutput); // move according to previous output
  }
  else if(controller.GetMode() == MANUAL){
    //if the controller is turned off, turn it on and move the valve
    Serial.println("turning on and moving insp valve"); //@debugging
    controller.SetMode(AUTOMATIC);
    move();
  }
  else{
    //otherwise continue computing and giving output
    Serial.println("moving insp valve"); //@debugging
    move();
  }

}

/**
 * Trigger expiration
 */
void ProportionalValve::endBreath() {
  previousPIDOutput = pid_output_;  //save successfull position to begin with next loop

  // turn off insp PID computing
  controller.SetMode(MANUAL);    //Turn off PID controller

  analogWrite(valve_pin_, 0);    // close valve by reseting position, should be 0
}

void ProportionalValve::initializePID(double outputMin, double outputMax, int sampleTime){
  controller.SetOutputLimits(outputMin, outputMax);
  controller.SetSampleTime(sampleTime);
}

float ProportionalValve::integrateReadings(){
  PIDMemory[(active_memory_+used_memory_)%memory_length_] = inspFlowReader.get(); //get most recent flow reading in insp line
  //Serial.print("Measurement: ");
  //Serial.println(PIDMemory[(active_memory_+used_memory_)%memory_length_]);
  if (used_memory_ == memory_length_) {
    active_memory_ = (active_memory_+1)%memory_length_;
  } else {
    used_memory_++;
  }
  float sum = 0;
  for (int i = 0; i < used_memory_; i++) {
    sum += PIDMemory[(active_memory_+i)%memory_length_];
    //Serial.print(PIDMemory[(active_memory_+i)%memory_length_]);
    //Serial.print(" ");
  }
  //Serial.println(" <-- Memory");
  return sum/used_memory_;
}
// Inspiration valve
ProportionalValve inspValve(SV3_CONTROL);
