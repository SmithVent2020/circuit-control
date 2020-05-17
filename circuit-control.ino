#include <PID_v1.h>

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
const int SV1_CONTROL = 22; // relay pin that controls SV1 (air?)
const int SV2_CONTROL = 24; // relay pin that controls SV2 (O2?)
const int SV4_CONTROL = 26; // relay pin that controls SV4 (exp?)
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
float breathVolume; //ongoing breath volume counter

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
float breathBeginTime = millis(); //the time stamp when the last inspirtion began
float expiratoryTime = 0;//INITIALIZE THIS TO SOME VALUE based on IE ratio to avoid dividing by 0 in the first loop
float inspiratoryTime = 0; //INITIALIZE THIS TO SOME VALUE based on IE ratio to avoid calculating 0 in the first loop for actual IE ratio
float maxBreathTime = 10000; // assuming PS_MODE (in ms) <- I think this should be set within the PS function (Naylani)
float loopTimer =0; //to time how long the last loop was (used for updating volume of current breath)

// these default values needed to be configured (can then be overriden by user)
float setO2Concentration = 20; // 20% default O2 concentration
float pressureInspError = 0; // acceptable margin of error in inspiratory pressure
float pressureInspMin = 10; // minimum inspiratory pressure
float pressureExpMin = 2; //minimum pressure in expiratory line below which a breath is triggered
float pressureInspMax = 100; // maximum inspiratory pressure
float setTidalVolume = 0;
float tidalVolumeError = 0.1; //PLACEHOLDER VALUE acceptable tidal volume errors
float setBpm = 0; // breaths per minute
float setIERatio = 0;
float flowInspError = 0.1; //acceptable margin of error in inspiratory flow rate (calculated based on IE ratio)

//Measured Patient variables to be displayed
float actualIERatio = 0; //the actual IE ratio calculated using inspiratoryTime and expiratoryTime
float pressurePlateau = 0; //measured plateau pressure
float pressurePeak = 0; //measured peak pressure

// should have a width of 6? to correspond to each of the sensors
int sensorRanges[6] = {1, 1, 1, 1, 1, 1};

//PID set-up
//Define Variables we'll be connecting to
double inspiratoyPIDSetpoint, inspiratoryPIDInput, inspiratoryPIDOutput; //define the inspiratory PID variables
double O2PIDSetpoint, O2PIDInput, O2PIDOutput; //define O2 concentration PID variables

//Specify the links and initial tuning parameters
PID inspPID(&inspiratoryPIDInput, &inspiratoryPIDOutput, &inspiratoyPIDSetpoint,2,5,1, DIRECT); //creat inspiratory PID controller
PID O2ConPID(&O2PIDInput,&O2PIDOutput, &O2PIDSetpoint, 1,2,3, DIRECT); // create O2 concentraion management PID controller  


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
  if(pin == SV4_CONTROL){
    //if it is the normally open valve
    if (valveState == true) {
      digitalWrite(pin, HIGH); //write HIGH to close
    } else {
      digitalWrite(pin, LOW); //write LOW to open
    }
  }else{ //otherwise if the valve is noramally open:
    if (valveState == true) {
      digitalWrite(pin, LOW); // relay is active low, so it turns on when we set pin to LOW
    } else {
      digitalWrite(pin, HIGH);
    }
}
}
/**
 * Opens or closes the proportional inspiration valve by an increment to current position
 * Increment is positive if opening, negative if closing
 * 
 * NOTE by @Mandira:
 * Likely to need this component: https://tameson.com/valves/solenoid-valve/proportional/controller/316529-proportional-pwm-controller-12-24vdc-din-a-pg-burkert-316529.html
 * for PWM conversion of voltage signal, or possibly a voltage divider in circuit, but unsure about that
 * 
 * @param increment - increment assuming in mm
 * @param increment - current position assuming in mm
 * @returns new position
 */
float moveInspiratoryValve(float increment, float pos) {  
 
  float posVoltage = (5/(9.5-2)) * (pos + increment) ; // assuming 0-5V input signal, orifice range from datasheet 
  posVoltage = (255/5) * posVoltage; // convert to analog. Remove if component below is used
  analogWrite(SV3_CONTROL, posVoltage); // moves to new position
  
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
float readOxygenSensor(){
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
 * Volume Control algorithm-----------------------------------------------------------------------------------------------------
 */
void volumeControl(){
  //This function controles the valves to provide volume control functionality
  //and the minimum expiratory pressure (lower than PEEP) to indicate when the patient is trying to breath in

  //calculate desired inspiratory flow rate
  float desiredInspTime = setIERatio/(setBpm*(setIERatio+1));
  float desiredInspFlow = setTidalVolume/desiredInspTime; //THIS ASSUMES VOLUMETRIC FLOW Rate
  
  if(breathStatus ==false){
    //if the patient is currently exhaling
    float maxBreathTime = (1/setBpm)*60000; //maximum milliseconds per breath based on set bpm

    if(pressureExp < pressureExpMin ||(millis() - breathTimer )>=maxBreathTime){
      //if the patient is trying to breath in (pressureExp goes down below minimum)
      //or the breath timer indicates that the next breath should start
      beginBreath(desiredInspFlow); //begin a breath
      
    }else {
      //the patient is still exhaling
    }
    
  }else if(breathStatus == true){
    //if the patient is currently inhaling
    
    if(breathVolume >= setTidalVolume){
      //if the volume of air inhaled is greater than or equal to the desired tidal volume
      endBreath(); //end inspiration and record plateau pressure
    
    }else if(pressureInsp >= pressureInspMax-pressureInspError){
      //if inspiratory pressure is exceeding acceptable limits (low compliance lung)
      endBreath(); //end inspiration and record plateau pressure
      
    }else{
      //patient is still inhaling
      //calculate PID control;
      inspiratoyPIDSetpoint = desiredInspFlow; //set desired inspiratory flow rate as the setpoint to the PID controller
      inspiratoryPIDInput = flowIn;
      inspPID.Compute(); //compute PID control value
      moveInspiratoryValve(inspiratoryPIDOutput, inspValvePosition); //adjust the inspiratory valve according to the increment calculated by the PID controler
      loopTimer = millis() - loopTimer; // calculate time of last loop
      breathVolume += flowIn *loopTimer; //update breath volume based on current volumetric flow rate
      loopTimer = millis(); // reset loop timer 
      }
  }
}

/*functions used in the colume control and Pressure support algorithms
 * 
 */
 void beginBreath(float desiredInspFlow){
  /*
   * this function switches from expiration to inspiration
   * it closes the O2, air and expiratory valves
   * turns off PID control for the O2 concentration,
   * and turns on the PID control for inspiratory flow
   * it also resets the breath timer to zero
   */

  breathStatus = true; //set breath to inspiratory
  
  //subtract time at beginning of expiration from current time to get duration of expiration
  expiratoryTime = millis() - expiratoryTime;

  actualIERatio = inspiratoryTime/(expiratoryTime); // calculate actual IE ratio from the previous breath cycle

  O2ConPID.SetMode(MANUAL); //turn off O2 concentration PID control
  
  //close valves
  actuateValve(false, SV2_CONTROL); //close O2 valve
  actuateValve(false, SV1_CONTROL); //close air valve pin
  actuateValve(false, SV4_CONTROL); // close expiration valve
  
  inspiratoyPIDSetpoint = desiredInspFlow; //set the PID setpoint to the desired inspiratory flow rate
  inspPID.SetMode(AUTOMATIC); //Turn on inspiratory PID to open valves
  
  inspiratoryPIDInput = flowIn; //set the inspiratory flow rate as the input
  inspPID.Compute(); //do a round of inspiratory PID computing
  moveInspiratoryValve(inspiratoryPIDOutput, inspValvePosition); //move the inspiratory valve according to the increment calculated by the PID controler

  breathTimer = millis(); //reset breath counter
  //beginBreath = millis();
  inspiratoryTime = millis(); //reset inspiratory time counter
  breathVolume = 0; //reset breath volume counter 
}

void endBreath(){
  /*this function switches from expiration to inspiration
   * it closes the inspiratory valve,
   * pauses for air retention and measures the plateau pressure
   * then opens the expiratory valve
   * turns on O2 concentration PID control
   * NOTE: O2 concentration management needs to be added
   */
  breathStatus = false; //set breath to expiratory
  
  //subtract the time at beginning of inspiration to current time 
  //to calculate inspiration length
  inspiratoryTime = millis() - inspiratoryTime; 
  
  inspPID.SetMode(MANUAL); // turn off inspiratory PID control
  moveInspiratoryValve(-inspValvePosition, inspValvePosition); //close inspiratory valve 
  
  delay(200); // pause between insp and exp THIS NEEDS TO BE CHANGED AFTER CONSULTING WITH SYSTEM REQUIREMENTS TEAM
  pressurePlateau = readPressureSensor(PRESSURE_INSP); //read plateau pressure
  
  actuateValve(true, SV4_CONTROL); // open expiration valve

  O2PIDSetpoint = setO2Concentration; //set the desired O2 concentration as the setpoint for the O2 PID controller
  O2ConPID.SetMode(AUTOMATIC); //turn on PID control for O2 concentration 
  //Do something with the O2ConPID output once O2 concentration control method has been decided on

  actuateValve(true, SV2_CONTROL); //open O2 valve
  actuateValve(true, SV1_CONTROL); //open air valve pin
  
  //display actual IE ratio
  expiratoryTime = millis(); //reset expiratory breath counter

  //decide if the tidal volume not met alarm should be activated
  if(breathVolume < setTidalVolume - tidalVolumeError){
    //alarm.activateAlarm(alarmCodes.ALARM_TIDAL_LOW)
  }
  
}
//-------------------------------------------------------------------Main Loop-------------------------------------------------------------
// Run forever
void loop(){
  
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
