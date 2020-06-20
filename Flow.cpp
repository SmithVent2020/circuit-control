#include "Flow.h"
#include "Constants.h"

/*
 * Initialize values
 */
Flow::Flow(int pin) {
  sensor_pin_ = pin;
  flow_rate_ = 0.0;
  accum_volume_ = 0.0;
  zero_flow_offset_ = 0;
  last_timepoint_ = millis();
}

/**
 * Get flow readings
 */
void Flow::read() {
  // read the voltage
  long R = analogRead(sensor_pin_);

  // sensor_read(0.5-4.5 V) maps linearly to flow_rate_ (0-150 SLPM)
  const float Fmax       = 150;                     // max flow in SLPM. (Min flow is 0)
  const long Vsupply     = 5000;                    // voltage supplied, mv
  const long sensorMin   = 1023L*500 / Vsupply;     // Sensor value at 500 mv
  const long sensorRange = 1023L * 4000 / Vsupply;  // 4000 mv range (regardless of calibration?)
  
  // Convert analog reading to flow rate at standard temperature and pressure
  // offset is from calibration during zero-flow initialization
  flow_rate_ = (R - sensorMin) * (Fmax / sensorRange)- zero_flow_offset_;  
}

/**
 * Start/restart volume integration.
 */
void Flow::resetVolume() {
  last_timepoint_ = millis();
  accum_volume_ = 0;
}

/**
 * Add the flow during the recent time interval to the accumulated volume
 * 
 * Theoretically we could get slightly better accuracy by choosing 
 * the midpoint betweenthe previous flow and the current flow, but that 
 * is unlikely to make much difference.
 */
void Flow::updateVolume() {
  unsigned long new_timepoint = millis();
  unsigned long interval = new_timepoint - last_timepoint_;

  // Convert flow from L/min to ml/ms.
  float flow = get() * LPM_TO_CC_PER_MS;

  accum_volume_ += flow * interval;
  last_timepoint_ = new_timepoint;
}

void Flow::calibrateToZero() {
  // set offset to zero
  zero_flow_offset_ = 0; 
  float fm[5];
  for (int i = 0; i < 5; i++) {
    read();
    fm[i] = flow_rate_;
    delay(100);
  }

  // average last three readings
  zero_flow_offset_ = (fm[2]+fm[3]+fm[4])/3;  
}

// Flow sensors
Flow inspFlowReader(FLOW_INSP);
Flow expFlowReader(FLOW_EXP);
