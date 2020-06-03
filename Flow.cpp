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
  const float Fmax       = 150;                     // max flow in SLPM. (Min flow is 0)
  const long Vsupply     = 5000;                    // voltage supplied, mv
  const long sensorMin   = 1023L * 500 / Vsupply;  // Sensor value at 500 mv
  const long sensorMax   = 1023L * 4500 / Vsupply; // Sensor value at 4500 mv
  const long sensorRange = sensorMax - sensorMin;

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
  Serial.print("accum_volume_ before ="); Serial.print("\t"); Serial.println(accum_volume_);
  Serial.print("last_timepoint_ ="); Serial.print("\t"); Serial.println(last_timepoint_);
  unsigned long new_timepoint = millis();
  Serial.print("new_timepoint ="); Serial.print("\t"); Serial.println(new_timepoint);
  unsigned long interval = new_timepoint - last_timepoint_;

  // Convert flow from L/min to ml/ms.
  float flow = get() * LPM_TO_CC_PER_MS;
  Serial.print("flow ="); Serial.print("\t"); Serial.println(flow);

  // Multiply flow by time and add to accumulated volume.
  // Theoretically, we could get slightly better accuracy by choosing the midpoint between
  // the previous flow and the current flow, but that is unlikely to make much difference.
  accum_volume_ += flow * interval;
  Serial.print("accum_volume_ after ="); Serial.print("\t"); Serial.println(accum_volume_);
  last_timepoint_ = new_timepoint;
}

// Flow sensors
Flow inspFlowReader(FLOW_INSP);
Flow expFlowReader(FLOW_EXP);
