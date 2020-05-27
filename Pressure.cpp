#include "Pressure.h"

/*
 * Initialize values
 */
Pressure::Pressure(int pin) {
  sensor_pin_ = pin;
	current_ = 0.0;
	current_peak_ = 0.0;
	peak_ = 0.0;
	plateau_ = 0.0;
	peep_ = 0.0;
}

/**
 * Get pressure reading
 */
void Pressure::read() {
	// read the voltage
	int V = analogRead(sensor_pin_);

	const float Pmin = -163.155;   // pressure max in mbar
	const float Pmax = 163.155;    // pressure min in mbar
	const float Vmax = 1023;       // max voltage in range from analogRead

	// convert to pressure
	float pressure = (5.0 * V/Vmax - 0.5) * (Pmax-Pmin)/4.0 + Pmin; //mmHg

	// convert to cmH20
	pressure *= 1.01972;

	// update peak and reset
	current_peak_ = max(current_peak_, pressure);
	current_ = pressure;
}
