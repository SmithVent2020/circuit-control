/**
 * SmithVent
 * Ventilator Control Circuit
 *
 * The circuit:
 * Inputs
 *    analog  -- oxygen sensor, and 2 flow sensors (one for each line)
 *    digital -- 3 pressure sensors (inspiratory, expiratory, and reservoir), LCD
 * Outputs:
 *    analog  -- none
 *    digital -- SV1 (air), SV2 (O2), SV3 (reservoir), SV4 (exp)
 */

//------------------Libraries------------------
#include "Constants.h"
#include "Pressure.h"
#include "Flow.h"
#include "Oxygen.h"
#include "Valve.h"
#include "ProportionalValve.h"
#include "O2management.h"
#include "AlarmManager.h"
#include "Display.h"
#include "StateMachine.h"


// Flags
bool DEBUG = false;           // for debugging mode
bool onButton = true;         // should be set to true when user indicates so on screen
VentMode ventMode = VC_MODE;  // set the default ventilation mode to volume control 

// State objects
StateMachine state;           // current state of machine

//--------------Declare Functions--------------
/**
 *  function that reads all sensors and updates values 
 */
void readSensors(){
  //inspiratory sensors
  inspFlowReader.read();                    // inspiratory flow (SLPM)
  inspPressureReader.read();                // inspiratory pressure (cmH2O)
  reservoirPressureReader.readReservoir();  // gas reservoir pressure (cmH2O)

  //expiratory sensors
  expFlowReader.read();                     // expiratory flow (SLPM)
  expPressureReader.read();                 // expiratory pressure (cmH2O)
}


//-------------------Set Up--------------------
void setup() {
  Serial.begin(115200);   // open serial port for debugging

  // initialize screen
  display.init();

  // initialize pins with pinMode command
  pinMode(SV1_CONTROL, OUTPUT);
  pinMode(SV2_CONTROL, OUTPUT);
  pinMode(SV4_CONTROL, OUTPUT);
  pinMode(SV3_CONTROL, OUTPUT);

  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(PRESSURE_RESERVOIR, INPUT);
  pinMode(PRESSURE_INSP, INPUT);
  pinMode(PRESSURE_EXP, INPUT);
  pinMode(O2_SENSOR, INPUT);
  pinMode(FLOW_INSP, INPUT);
  pinMode(FLOW_EXP, INPUT);

  // setup PID controller (for VC mode, the default mode)
  inspValve.initializePID(OUTPUT_MIN, OUTPUT_MAX, SAMPLE_TIME); 
  inspValve.previousPosition = DEFAULT_VALVE_POSITION;     

  // warm up SV3 valve by opening it to unstick it
  analogWrite(SV3_CONTROL, 255);
  delay(35);
  analogWrite(SV3_CONTROL, 0);

  // set to VC_MODE (@FutureWork: ideally this would be indicated through the UI startup sequence)
  ventMode = VC_MODE;   // for testing VC mode only
  expValve.close();     // close exp valve for calibration

  // calibrate flow meters -- seems to change when SV4 closes
  inspFlowReader.calibrateToZero(); // set non-flow analog readings as the 0 in the flow reading functions
  expFlowReader.calibrateToZero();  

  // valve settings to begin
  inspValve.endBreath();      // close the inspiratory valve
  expValve.open();            // keep expiratory valve open for safety (also does not use as much power)

  // @FutureWork: implement startup sequence on display
  // display.start();
}


//-------------------Run Forever--------------------
void loop() {
  display.listen(); // listen for interactions with display

  readSensors(); 

  // @FutureWork: We only alarm after first 5 breaths (this is a "warm up" issue where it takes time to stabilize)
  if (state.breath.cycleCount > 5){  
    state.breath.checkForAlarmConditions();     
    alarmMgr.maintainAlarms();  // maintain any ongoing alarms 
  }
  
  display.updatePressureWave(inspPressureReader.get()); 

  // manage reservoir refilling based on FIO2 concentration set by user on the display
  o2Management(display.oxygen());

  // handle state maintenance & update
  state.update();
}
