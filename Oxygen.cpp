#include "Oxygen.h"

/**
 * Get oxygen concentration reading (may want to research a better approach)
 */
void Oxygen::read() {
  // change analog pin reference voltage to 1.1V and discard first reading
  analogReference(INTERNAL1V1);
  analogRead(sensor_pin_);

  unsigned R = analogRead(sensor_pin_); // map linearly to concentration

  const unsigned O2Max = 100;                      // max oxygen percentage
  const unsigned Vref = 1100;                      // reference voltage (mv)
  const unsigned sensorVMax = 60;                  // voltage range (0-60 mV) returned from sensor
  const unsigned Rmax = 1023U * sensorVMax / Vref; // Max sensor reading (corresponding to 60mv)

  concentration_ = R * O2Max / Rmax;  // Concentration in percent

  // change analog pin reference voltage back to 5.0 V and discard first reading again
  analogReference(DEFAULT);
  analogRead(sensor_pin_);
}

// The oxygen reader
Oxygen oxygenReader(O2_SENSOR);
