#include "O2management.h"
#include "Pressure.h"
#include "Valve.h"
#include "AlarmManager.h"

#include <limits.h> // For ULONG_MAX

static const float UPPER_PRESSURE_THRESHOLD = 1756.67; //cmH2O (25 psi)
static const float LOWER_PRESSURE_THRESHOLD = 703.07;  //cmH2O (10 psi)
static const float LOWER_PRESSURE_LIMIT     = 400;     //cmH2O


void o2Management(int O2target){
  if(reservoirPressureReader.get() < LOWER_PRESSURE_LIMIT){
    alarmMgr.activateAlarm(ALARM_INLET_GAS);
  }
  if(reservoirPressureReader.get()<= LOWER_PRESSURE_THRESHOLD){
    if (O2target == 21) {
      airValve.open();
    } else if (O2target == 60){
      airValve.open();
      oxygenValve.open();
    } else if (O2target == 100){
      oxygenValve.open();
    } else {
      airValve.open();
    }
  }
  else if(reservoirPressureReader.get() >= UPPER_PRESSURE_THRESHOLD){
    airValve.close();
    oxygenValve.close();
  }
  
}
