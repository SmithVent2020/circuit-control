#include "O2Management.h"
#include "Pressure.h"
#include "Valve.h"

#include <limits.h> // For ULONG_MAX

static const float UPPER_PRESSURE_THRESHOLD = 1756.67; //cmH2O (25 psi)
static const float LOWER_PRESSURE_THRESHOLD = 703.07;  //cmH2O (10 psi)


void o2Management(int O2target){

  if(reservoirPressureReader.get()<= LOWER_PRESSURE_THRESHOLD){
    // we always want to open something, even if we can't hit setting exactly
    if(O2target == 100){
      // pure oxygen
      oxygenValve.open();
    } else if (O2target >= 50){
      // mix air and oxygen => 60% oxygen
      airValve.open();
      oxygenValve.open();
    } else {
      // default to air (21% oxygen)
      airValve.open();
    }
  }
  else if(reservoirPressureReader.get() >= UPPER_PRESSURE_THRESHOLD){
    airValve.close();
    oxygenValve.close();
  }
  
}
//|===================================================================================================================================================================|
//|---------Below this point is the code we would use for finer control of the O2 concentration if we had a working O2 sensor to calebrate it with--------------------|
//|===================================================================================================================================================================|

// During a tank top up, we open one valve at a time.
// If we are using only one gas (O2 or air), then we skip the SECOND_VALVE_OPEN state.
//enum TopUpState {
//  VALVES_CLOSED,
//  FIRST_VALVE_OPEN,
//  SECOND_VALVE_OPEN
//};

/*
 * pressure limits:
 *  - Low pressure limit
 *  - High pressure limit
 *  - Low top-up threshold (start top-up)
 *  - High top-up threshold (stop top-up)
 *
 * When the pressure drops below the Low threshold, begin a top-up cycle.
 * If the pressure is above the High threshold, do not begin a top-up cycle,
 * but do not interrupt a cycle already in progress. A top-up cycle produces
 * the correct O2/N2 mix by using time, so once it begins, it must complete.
 *
 * We want to choose the low threshold such that, if the expiratory valve
 * opens and the pressure drops below threshold, we have time to loop around
 * AND open the inlet valves before the pressure drops below the Low Pressure
 * limit.  Assume about 50 ms to recognize the event and turn the inlet valves
 * on.
 *
 * We want to choose the high threshold such that
 * if we begin a top-up just below the high threshold, we can
 * complete the top-up with the inspiratory valve closed without exceeding the
 * high pressure limit.
 */

// All pressures are in cmH2O above atm
//static const float LOW_PRESSURE_LIMIT  = 703.0;  // 10 psi
//static const float HIGH_PRESSURE_LIMIT = 2109.2; // 30 psi
//static const float LOW_TOPUP_TRESHOLD  = 875.0;  // 160 cmH2O above LOW_PRESSURE_LIMIT plus margin of safety
//static const float HIGH_TOPUP_TRESHOLD = 1500.0; // Half way between LOW_TOPUP_THRESHOLD and HIGH_PRESSURE_LIMIT
//
//static const unsigned long TOPUP_TIME  = 200;    // Time for one topup cycle @TODO: compute real value
//
//static unsigned long topUpTimer = 0;
//static unsigned long topUpSwitchoverTime;  // Time at which to switch from O2 to air or vice-versa
//static unsigned long topUpEndTime;
//static unsigned long O2TimeInterval;
//static unsigned long AirTimeInterval;
//
//static Valve *firstValvePtr = &oxygenValve;  // Pointer to first valve to be opened on this cycle
//static Valve *secondValvePtr = &airValve;    // Pointer to second valve to be opened on this cycle
//
//static TopUpState state = VALVES_CLOSED;

//static void setState(TopUpState newState) {
//  state = newState;
//}
//
//static void computeIntervals(int O2target) {
//  O2TimeInterval  = (O2target - 21) * TOPUP_TIME / (100 - 21);
//  AirTimeInterval = TOPUP_TIME - O2TimeInterval;
//}
//
//// Swap the first valve and second valve
//static void swapValves() {
//  static Valve *temp = firstValvePtr;
//  firstValvePtr = secondValvePtr;
//  secondValvePtr = temp;
//}
//
//static void beginTopUp(unsigned long startTime) {
//  topUpTimer   = startTime;
//  topUpEndTime = topUpTimer + TOPUP_TIME;
//
//  // Choose the switch time interval depending on which gas is going first this time around.
//  unsigned long topUpSwitchInterval = (firstValvePtr == &oxygenValve) ? O2TimeInterval : AirTimeInterval;
//
//  if (topUpSwitchInterval > 0) {
//    topUpSwitchoverTime = topUpTimer + topUpSwitchInterval;
//  }
//  else {
//    // The first valve should be open for zero milliseconds.  Start with other gas and never switch.
//    swapValves();
//    topUpSwitchoverTime = ULONG_MAX;            // We will never switch gasses on this cycle
//  }
//
//  // Open the first valve.
//  firstValvePtr->open();
//  secondValvePtr->close();
//}
//
//static void endTopUp() {
//  oxygenValve.close();
//  airValve.close();
//  swapValves();   // Start with the oposite valve next time
//}
//
//void o2Management(int O2target) {
//
//  unsigned long currentTime = millis();
//
//  switch (state) {
//    case VALVES_CLOSED:
//      if (reservoirPressureReader.get() < LOW_TOPUP_TRESHOLD) {
//        computeIntervals(O2target);
//        beginTopUp(currentTime);
//        setState(FIRST_VALVE_OPEN);
//      }
//      break;
//
//    case FIRST_VALVE_OPEN:
//      if (currentTime >= topUpEndTime) {
//        if (reservoirPressureReader.get() < HIGH_TOPUP_TRESHOLD) {
//          beginTopUp(currentTime);
//          // Stay in this state
//        }
//        else {
//          endTopUp();
//          setState(VALVES_CLOSED);
//        }
//      }
//      else if (currentTime >= topUpSwitchoverTime) {
//        firstValvePtr->close();
//        secondValvePtr->open();
//        setState(SECOND_VALVE_OPEN);
//      }
//      break;
//
//    case SECOND_VALVE_OPEN:
//      if (currentTime >= topUpEndTime) {
//        if (reservoirPressureReader.get() < HIGH_TOPUP_TRESHOLD) {
//          beginTopUp(currentTime);
//          setState(FIRST_VALVE_OPEN);
//        }
//        else {
//          endTopUp();
//          setState(VALVES_CLOSED);
//        }
//      }
//      break;
//  }
//}
