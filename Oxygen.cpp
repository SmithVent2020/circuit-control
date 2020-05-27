#include "Oxygen.h"

/*
 * Initialize values
 */
Oxygen::Oxygen() {
  sensor_pin_ = O2_SENSOR;
}

/**
 * Get oxygen concentration reading (may want to research a better approach)
 */
void Oxygen::read() {
	// change analog pin reference voltage to 1.1V and discard first reading
	analogReference(INTERNAL1V1);
	analogRead(sensor_pin_);

	int V = analogRead(sensor_pin_); // map linearly to concentration

	const float O2Max = 100;    // max oxygen percentage
	const float Vmax = 1023;    // max voltage in range from analogRead
	const float Vsupply = 1.1;  // supplied voltage
	const float sensorRange = (60 * 1000); // voltage range (0-60 mV) returned from sensor in V

	float concentration_ = Vsupply * V/Vmax * O2Max/sensorRange;

	// change analog pin reference voltage back to 5.0 V and discard first reading again
	analogReference(DEFAULT);
	analogRead(sensor_pin_);
}
