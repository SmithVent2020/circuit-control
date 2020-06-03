#include "Flow.h"
#include "Constants.h"

/*
 * Initialize values
 */
Flow::Flow(int pin) {
  sensor_pin_ = pin;
  flow_rate_ = 0.0;
  peak_flow_ = 0.0;
  current_peak_ = 0.0;
}

/**
 * Get flow readings
 */
void Flow::read() {
  // read the voltage
  unsigned long R = analogRead(sensor_pin_);

  // sensor_read(0.5-4.5 V) maps linearly to flow_rate_ (0-150 SLPM)
  const float Fmax                = 150;                     // max flow in SLPM. (Min flow is 0)
  const unsigned long Vsupply     = 5000;                    // voltage supplied, mv
  const unsigned long sensorMin   = 1023UL * 500 / Vsupply;  // Sensor value at 500 mv
  const unsigned long sensorMax   = 1023UL * 4500 / Vsupply; // Sensor value at 4500 mv
  const unsigned long sensorRange = sensorMax - sensorMin;

  // Convert analog reading to flow rate at standard temperature and pressure
  flow_rate_ = (R - sensorMin) * (Fmax / sensorRange);
}

/**
 * Start/restart volume integration.
 */
void Flow::resetVolume() {
  last_timepoint_ = millis();
  accum_volume_ = 0;
}

/**
 * Add the flow during the recent time interval to the accumulated value.
 */
void Flow::updateVolume() {
  unsigned long new_timepoint = millis();
  unsigned long interval = new_timepoint - last_timepoint_;

  // Convert flow from L/min to ml/ms.
  float flow = get() * LPM_TO_CC_PER_MS;

  // Multiply flow by time and add to accumulated volume.
  // Theoretically, we could get slightly better accuracy by choosing the midpoint between
  // the previous flow and the current flow, but that is unlikely to make much difference.
  accum_volume_ += flow * interval;
  last_timepoint_ = new_timepoint;
}

// Flow sensors
Flow inspFlowReader(FLOW_INSP);
Flow expFlowReader(FLOW_EXP);
