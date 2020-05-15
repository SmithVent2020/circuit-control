/**
 * SmithVent
 * Ventilator Control Circuit
 * 
 * The circuit:
 * Inputs 
 *    analog  -- oxygen sensor, and 2 flow sensors (one for each line)
 *    digital -- 3 pressure sensors (inspiratory, expiratory, and chamber mixing), LCD
 * Outputs: 
 *    analog  -- none
 *    digital -- SV1, SV2, SV3, SV4
 * 
 * modified on May 11, 2020
 * created by Dan, Farida, Naylani, and Mandira
 */

//-------------------------------------------------LIBRARIES--------------------------------------------------------
#include <math.h>
#include <LiquidCrystal.h>


//----------------------------------------------------PINS----------------------------------------------------------
// alarm related pins
const int BUZZER     = 2;
const int YELLOW_LED = 3;
const int RED_LED    = 4;

// valve input and output pins
const int SV1_CONTROL = 22; // relay pin that controls SV1
const int SV2_CONTROL = 24; // relay pin that controls SV2
const int SV4_CONTROL = 26; // relay pin that controls SV4
const int SV3_CONTROL = 5;  // proportional valve pin (?)

// flow sensor input pins
const int FLOW_IN  = A0;
const int FLOW_OUT = A1;

// pressure sensor input pins
const int PRESSURE_CHAMBER = A5; 
const int PRESSURE_INSP    = A6;
const int PRESSURE_EXP     = A7; 

// oxygen sensor input pin
const int O2_SENSOR = A8;


//-----------------------------------------------GLOBAL VARIABLES---------------------------------------------------
// valve state variables 
float inspValvePosition = 0;  // initialize inspiratory valve position to closed
float expValvePosition = 0;   // initialize expiratory valve position to closed
float currentPosition = 0;    // of the proportional valve
float propValveIncrement = 0; // of the proportional valve

// pressure sensor variables
float pressureMixing = 0;  // variable to store the mixing chamber pressure value read
float pressureInsp    = 0;  // variable to store the inspiratory pressure value read
float pressureExp     = 0;  // variable to store the exspiratory pressure value read

// oxygen sensor variable
float oxygenConcentration = 0;  // variable to store the O2 value read

// flow sensor variables
float flowIn = 0;  // variable to store the flow sensor value read on inspiratory line
float flowOut = 0;  // variable to store the flow sensor value read on expiratory line

bool alarmStatus;

// enumerate alarm priority
enum alarmPriority {
  HIGH_PRIORITY, // 0
  MED_PRIORITY   // 1
};

// 2D array to keep track of alarms and their priority, initialized as 'OFF'
// HIGH -- apnea -> power supply fail -> inlet gas fail -> inlet O2 fail -> battery low -> pressure sensor fail x3
// LOW  -- PEEP high -> PEEP low -> insp pressure high -> insp pressure low -> tidal volume high -> tidal volume low -> o2 sensor fail
bool alarms[2][8] = {{false, false, false, false, false, false, false, false},
                     {false, false, false, false, false, false, false, false}};


// enumerate modes
enum ventilatorMode {
  VC_MODE, // 0
  PS_MODE  // 1
};

// user variables
int ventMode = PS_MODE; // set to VolumeControl by default
bool ventilatorOn;
bool breathStatus;
float breathTimer = 0; // time since beginning of last breath
float expiratoryTime = 0;
float maxBreathTime = 10000; // assuming PS_MODE (in ms)

// these default values needed to be configured (can then be overriden by user)
float setO2Concentration = 20; // 20% default O2 concentration
float pressureInspError = 0; // acceptable margin of error in inspiratory pressure
float pressureInspMin = 10; // minimum inspiratory pressure
float pressureInspMax = 100; // maximum inspiratory pressure
float setTidalVolume = 0;
float setBpm = 0; // breaths per minute
float setIERatio = 0;

// should have a width of 6? to correspond to each of the sensors
int sensorRanges[6] = {1, 1, 1, 1, 1, 1};



//-----------------------------------------------SET UP & MAIN LOOP-------------------------------------------------
void setup() {
  //initialize pins with pinMode command
  pinMode(SV1_CONTROL, OUTPUT);
  pinMode(SV2_CONTROL, OUTPUT);
  pinMode(SV3_CONTROL, OUTPUT);
  pinMode(SV4_CONTROL, OUTPUT);

  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(PRESSURE_CHAMBER, INPUT);
  pinMode(PRESSURE_INSP, INPUT);
  pinMode(PRESSURE_EXP, INPUT);
  
  pinMode(O2_SENSOR, INPUT);
  pinMode(FLOW_IN, INPUT);
  pinMode(FLOW_OUT, INPUT);

  Serial.begin(9600); // open serial port for debugging
}

// Run forever
void loop() {
  
  // --- 1. check if ventilator is actively 'ON' and get any new user input
  getUserInputs();

  if(ventilatorOn == false) {
    return;
  }
  
  // --- 2. check sensors and store new values
  oxygenConcentration = readOxygenSensor();
  
  pressureMixing = readPressureSensor(PRESSURE_CHAMBER);
  pressureInsp   = readPressureSensor(PRESSURE_INSP);
  pressureExp    = readPressureSensor(PRESSURE_EXP);

  flowIn  = readFlowSensor(FLOW_IN);
  flowOut = readFlowSensor(FLOW_OUT);
  
  // --- 3. check if sensor readings are within acceptable parameters
  checkSensors(sensorRanges);

  // loop over ranges to check for LOW or HIGH flags
  for(int i=0; i < (sizeof(sensorRanges)/sizeof(sensorRanges[0]))-1; i++) {
    // set corresponding value in alarms array and set alarm status to true
  }
  
  // --- 4. breathing mode (VC or PS)
  if (ventMode == VC_MODE) {
    volumeControl();
  } else {
    pressureSupport();
  }
  
  // --- 5. activate alarms (if any) and update display
  if(alarmStatus == true) {
    activateAlarm(alarms);
  }
  
  updateDisplay();

  // --- 6. regulate how long the loop takes to run
  delay(10); // should be based on time elapsed vs. how long the loop should take
}

//------------------------------------------VENT ALGORITHMS------------------------------------------------------
/**
 * Pressure Support algorithm
 */
void pressureSupport() {
  // check if status is inspiring
  if (breathStatus == true) {
    // inspiring logic
  } else { 
    // expiring logic
  }
}

/**
 * Volume Control algorithm
 */
void volumeControl() {
   // check if status is inspiring
  if (breathStatus == true) {
    // inspiring logic
  } else { 
    // expiring logic
  }
}

//---------------------------------------------DISPLAY------------------------------------------------------------
/**
 * gets latest user input on display and updates user variables
 */
void getUserInputs() {
  // get ventMode and update variable
  // set various value thresholds
}

/**
 * updates display with any new values and alarms (if any)
 */
void updateDisplay() {
}

//------------------------------------------VALVE FUNCTIONS-------------------------------------------------------
/**
 * Actuates valve based on boolean state given
 * 
 * @param state - flag to open/close valve
 * @param pin - pin number to specify which valve
 * @returns
 */
void actuateValve(bool valveState, int pin) {
  if (valveState == true) {
    digitalWrite(pin, LOW); // relay is active low, so it turns on when we set pin to LOW
  } else {
    digitalWrite(pin, HIGH);
  }
}

/**
 * Opens or closes the proportional inspiration valve by an increment to current position
 * Increment is positive if opening, negative if closing
 * 
 * NOTE by @Mandira:
 * 
 * @param increment - increment assuming in mm
 * @param increment - current position assuming in mm
 * @returns new position in mm 
 * Change 16.51 below to 0.65 to compute in inches 
 */
float moveInspiratoryValve(float increment, float pos) {  
 
  float posVoltage = (5/16.51) * (pos + increment) ; // outputs a 0-5V input signal to the solenoid valve controller, orifice size 16.51mm (0.65") for iQ valves 700048
  analogWrite(SV3_CONTROL, posVoltage); // moves to new position, <pos> mm
  
  return(pos + increment); 
}


//------------------------------------------SENSOR FUNCTIONS-------------------------------------------------------
/**
 * Used to read pressure sensor values in 3 different areas:
 * 1. Mixing Chamber
 * 2. Inspiratory line
 * 3. Expiratory line
 * 
 * @returns pressureValue - the pressure in cmH2O
 */
float readPressureSensor(int pin) {
  int sensorRead = analogRead(pin);
  float pressureValue = (sensorRead * 5.0/1023.0 - 0.5) * 326.31/4.0 - 163.155; // sensorRead(0.5-4.5 V) maps linearly to flow_read(+-163.155 cmH2O)
  return pressureValue;
}

/**
 * Reads the oxygen sensor to detect percentage of O2 
 * 
 * @returns oxygenValue - the percentage of O2 detected by sensor
 */
float readOxygenSensor() {
  analogReference(INTERNAL1V1);  // change analog pin reference voltage to 1.1V
  analogRead(O2_SENSOR); // discard the first reading after changing the reference voltage
  
  int sensorRead = analogRead(O2_SENSOR); 
  float oxygenConcentration = sensorRead * 1.1 * 10000/6.0/1023.0; // sensorRead(0.0-60.0 mV) maps linearly to oxygenConcentration(0-100%)
  
  analogReference(DEFAULT); // change analog pin reference voltage back to 5.0V
  analogRead(O2_SENSOR);    // read again to avoid next wrong reading due to change of reference voltage
  
  return oxygenConcentration;
}

/**
 * Used to read the flow in and out 
 * 
 * @param pin to specify flow sensor to read
 * @returns flowRate - flow rate (0-150 SPLM)
 */
float readFlowSensor(int pin){
  int sensorRead = analogRead(pin);
  float flowRate = sensorRead * 5.0 * 150/4/1023.0 - 18.75; // sensorRead(0.5-4.5 V) maps linearly to flowRate(0-150 SLPM)
  return flowRate;
}


//------------------------------------------ALARM FUNCTIONS-------------------------------------------------------
/**
 * Checks sensor ranges
 * Returns an array of values that correspond to each of the sensors
 * 0 - LOW 
 * 1 - IN RANGE
 * 2 - HIGH
 * 
 * for now checking just inspiratory pressure, but other values need to be checked as well
 */
void checkSensors(int sensorRanges[8]) {
  
  if(pressureInsp > pressureInspMax) {
    sensorRanges[0] = 2;
  } else if(pressureInsp < pressureInspMin) {
    sensorRanges[0] = 0;
  }
}

/**
 * Activates alarm buzzer and LEDs based on priorities given
 * Tested on TinkerCad by @Farida 
 * 
 * @param alarms - 2D array of bool values in priority order
 */
void activateAlarm(bool alarms[][8]) {

  // check if any high priority alarms are on  
  for(int i=0; i<sizeof(alarms[HIGH_PRIORITY]); i++) {
    if(alarms[HIGH_PRIORITY][i] == true) {
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 100, 1000); // turn on buzzer for 1000 ms at 100 Hz
      alarmStatus = true;
      break;
    }
  }

  // if no high alarm, check if any medium priority alarms are on
  if(!alarmStatus) {
    for(int i=0; i<sizeof(alarms[MED_PRIORITY]); i++) {
      if(alarms[MED_PRIORITY][i] == true) {
        digitalWrite(YELLOW_LED, HIGH);
        tone(BUZZER, 261, 500); //turn on buzzer for 500 ms at 26 Hz
        alarmStatus = true;
        break;
      }
    }
  }
}

/**
 * Turns off LEDs and buzzer
 */
void deactivateAlarm() {
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  noTone(BUZZER);
}
