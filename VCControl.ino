/*This file contains flow control algorithms for volume control mode
 * 
 */
#include <PID_v1.h>
//define variables that should be passed to it or be global:
bool breathStatus = false;
float loopTime = 0; //the time it takes to exicute one loop
float inspVPos = 0; //inspiratory valve position
float platP; //plateau pressure

int const O2ValvePin = 3; //this is INCORRECT just for convenience, pin of O2 sensor
int const airValvePin = 2; //air valve pin INCORRECT just for convenience
int const expValvePin = 1; // expiration valve pin INCORRECT just for convenience

//PID set-up
//Define Variables we'll be connecting to
double inspPIDSetpnt, inspPIDInpt, inspPIDOutpt; //define the inspiratory PID variables

//Specify the links and initial tuning parameters
PID inspPID(&inspPIDInpt, &inspPIDOutpt, &inspPIDSetpnt,2,5,1, DIRECT); //creat inspiratory PID controller

//----------------------------------------Functions--------------------------------------------------------------------------------
void beginBreath(float breathTimer, float inspTimer, float setIE, float setBpm, float setTV){
  //this function switches from expiration to inspiration
  //it closes the O2, air and expiratory valves 
  //and turns on the PID control for inspiratory valves
  //it also resets the breath timer to zero

  //close valves
  //actuate_valve(false, O2ValvePin); //close O2 valve
  //actuate_valve(false, airValvePin); //close air valve pin
  //actuate_valve(false, expValvePin); // close expiration valve

  //calculate desired inspiratory flow rate
  float desiredInspTime = setIE/(setBpm*(setIE+1));
  float desiredInspFlow = setTV/desiredInspTime; //THIS ASSUMES VOLUMETRIC FLOW Rate

  inspPIDSetpnt = desiredInspFlow; //set the PID setpoint to the desired inspiratory flow rate
  inspPID.SetMode(AUTOMATIC); //Turn on inspiratory PID to open valves

  inspPID.Compute(); //to a round of inspiratory PID computing
  Move_Insp_Valve(inspPIDOutpt, inspVPos); //move the inspiratory valve according to the increment calculated by the PID controler

  breathTimer = 0; //reset breath counter
  inspTimer = 0; //reset inspiratory time counter
}

float endBreath(float inspVPos){
  breathStatus = false; //set breath to expiratory
  inspPID.SetMode(MANUAL); // turn off inspiratory PID control
  Move_Insp_Valve(-inspVPos, inspVPos); //close inspiratory valve 
  
  delay(200); // pause between insp and exp THIS NEEDS TO BE CHANGED AFTER CONSULTING WITH SYSTEM REQUIREMENTS TEAM
  float platP = Read_P_Sen_In(); //read plateau pressure
  
  //actuate_valve(true, expValvePin); //open expiratory valve

  return platP;
}
float breathPause(){
  //pauses between inspiration and expiration and returns the plateau pressure
  delay(200); // pause between insp and exp THIS NEEDS TO BE CHANGED AFTER CONSULTING WITH SYSTEM REQUIREMENTS TEAM
  float platP = Read_P_Sen_In();
  return platP;
  
}

void volumeControl(float inspP, float expP, float inspFlow, float setTV, float setIE, float setBpm, float minExpP, float maxInspP){
  //This function controles the valves to provide volume control functionality
  //this fuction takes inspiratory and expiratory pressure readings, set tidal volume and bpm,
  //and the minimum expiratory pressure (lower than PEEP) to indicate when the patient is trying to breath in
  
  //define static variables for this function
  static float breathTimer = 0; //breath timer that should be updated each loop
  static float inspTimer = 0; //timer of how long it takes to inhale 
  static float expTimer = 0; //timer of how long it takes to exhale
  static float breathVol =0; //ongoing breath volume

  const float flowMargin = 0.1; //the margin of error allowed int the insp flow rate (L/min)
  const float inspPMargin = 0.1;// margin of error allowd in max inspiratory pressure
  if(breathStatus ==false){
    //if the patient is currently exhaling
    float maxBreathTime = (1/setBpm)*60000; //maximum milliseconds per breath based on set bpm

    if(expP < minExpP ||breathTimer >=maxBreathTime){
      //if the patient is trying to breath in (expP goes down below minimum)
      //or the breath timer indicates that the next breath should start
      beginBreath(breathTimer, inspTimer, setIE, setBpm, setTV); //begin a breath
      
    }else {
      //the patient is still exhaling
      expTimer += loopTime; //increment the expiratory timer
    }
  }else if(breathStatus == true){
    //if the patient is currently inhaling
    if(breathVol >= setTV){
      //if the volume of air inhaled is greater than or equal to the desired tidal volume
      platP = endBreath(inspVPos); //end inspiration and record plateau pressure
    
    }else if(inspP >= maxInspP-inspPMargin){
      //if pressure is
      platP = endBreath(inspVPos); //end inspiration and record plateau pressure

      //ADD TV NOT MET ALARM HERE
      
    }else{
      //patient is still inhaling
      //calculate PID control
      inspPID.Compute(); //compute PID control value
      Move_Insp_Valve(inspPIDOutpt, inspVPos); //adjust the inspiratory valve according to the increment calculated by the PID controler
      breathVol += inspFlow*loopTime; //update loop time
      inspTimer += loopTime; //update inspiratory timer
      }
  }
  breathTimer += loopTime; //update breath timer
}
