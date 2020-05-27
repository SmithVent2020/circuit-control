#include "Flow.h"

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
	int V = analogRead(sensor_pin_);

	const float Fmax = 150;     // max flow in SLPM
	const float Vmax = 1023;    // max voltage in range from analogRead
	const float Vsupply = 5.0;  // voltage supplied
	const float sensorRange = (4.5 - 0.5); // voltage range returned by sensor

	// convert to flow rate at standard temperature and pressure
	// sensor_read(0.5-4.5 V) maps linearly to flow_rate_(0-150 SLPM) with
	float flow_rate_ = Vsupply * V/Vmax * Fmax/sensorRange - 18.75;
}
