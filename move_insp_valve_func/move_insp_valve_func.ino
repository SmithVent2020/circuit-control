//Move inspiratory line function for SmithVent
//Opens or closes the proportional inspiration valve by an increment (increment is positive if opening, negative if closing)and returns new position

//Direct queries/clarifications to Mandira (mandira.marambe@gmail.com)
//The function is written at the end; pins will have to be initialized in main code
//Compiled on Arduino 1.8.8

//----------------------------------------------------TEST CODE---------------------------------------------------------------

const int PROP_VALVE = 41; //proportional valve pin (?)
float current_pos;
float prop_valve_increment;

void setup() {
Serial.begin(9600); //This is just for output from testing. Not needed for main code
pinMode (PROP_VALVE, OUTPUT);
}

void loop() {
  current_pos = Move_Insp_Valve(prop_valve_increment, current_pos); //Calling the function for test. Would be replaced in main code with relevant function
  Serial.println (current_pos); //Test return from function

}

//-----------------------------------------------------------FUNCTION---------------------------------------------------------


float Move_Insp_Valve(float increment, float pos){  
 //Opens or closes the proportional inspiration valve by an increment 
 //(increment is positive if opening, negative if closing)and returns new position
 float pos_v;
 
  pos = pos+increment; //******assuming in mm
  pos_v = (5/(9.5-2))*pos; //assuming 0-5V input signal, orifice range from datasheet 
  pos_v = (255/5)*pos_v; //convert to analog. remove if component below is used
  analogWrite(PROP_VALVE, pos_v); //moves to new position
  return (pos); //store new position as current position in main code
 
 // ***NOTE: Likely to need this component: https://tameson.com/valves/solenoid-valve/proportional/controller/316529-proportional-pwm-controller-12-24vdc-din-a-pg-burkert-316529.html
 //for PWM conversion of voltage signal, or possibly a voltage divider in circuit, but unsure about that
}
