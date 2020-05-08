/**
 * SmithVent
 * Ventilator Control Circuit
 * 
 * modified on May 7, 2020
 * created by Dan, Farida, and Naylani
 */

 // initialized any global variables
//Valve input and output pins
 const int RELAY_ENABLE = 22;
 const int VALVE_READ = 28; //Reading pin from same valve (not in circuit)
 const int PROP_VALVE = 41; //proportional valve pin (?)

//valve state variables
 bool state; // initialized as false by default
 bool read_state; //initialized as false
float current_pos; //Of proportional valve
float prop_valve_increment; //Of proportional valve

//-----------------------------------------------SET UP & MAIN LOOP-------------------------------------------------

void setup() {
  //initialize pin modes with pinMode command
  pinMode(RELAY_ENABLE, OUTPUT);
  pinMode(VALVE_READ, INPUT);
  pinMode (PROP_VALVE, OUTPUT);

  Serial.begin(9600);

}

void loop() {
  // Run forever
 //Call Valve_Status
  read_state = Valve_Status(VALVE_READ); //Calling the function for test. Would be replaced in main code with relevant function
  Serial.println (read_state); //Test return from function
 
 //Call actuate_valve
  actuate_valve(state); //**Pin number input here so same function for all valves?
  state = !state;
  
 //Call Move_Insp_Valve
  current_pos = Move_Insp_Valve(prop_valve_increment, current_pos); //Assuming in mm
  Serial.println (current_pos); //Test return from function
 
  delay(1000);
}

//------------------------------------------VALVE FUNCTIONS-------------------------------------------------------
void actuate_valve(bool state) {
  // actuates valve based on boolean state
  if (state) {
    Serial.println("Valve ON");
    digitalWrite(RELAY_ENABLE, LOW); // relay is active low, so it turns on when we set pin to LOW
  } else {
    Serial.println("Valve OFF");
    digitalWrite(RELAY_ENABLE, HIGH);
  }

  // more concise 
  // state ? digitalWrite(VALVE_PIN, LOW) : digitalWrite(VALVE_PIN, HIGH);
}

bool Valve_Status(int pin){     
 //Returns True if valve is open, False if valve is closed
if (digitalRead(pin)==HIGH){  //Valve should be open when power is supplied. If circuit is reversed, reverse logic
  Serial.println("valve is open");
  return true; //open
}
else if (digitalRead(pin)==LOW){   
  Serial.println("valve is closed");
  return false; //closed
}
}

float Move_Insp_Valve(float increment, float pos){  
 //Opens or closes the proportional inspiration valve by an increment 
 //(increment is positive if opening, negative if closing)and returns new position
 float pos_v;
 
  pos = pos+increment; //******assuming in mm
  pos_v = (5/(9.5-2))*pos; //assuming 0-5V input signal, orifice range from datasheet 
  pos_v = (255/5)*pos_v; //convert to analog. Remove if component below is used
  analogWrite(PROP_VALVE, pos_v); //moves to new position
  return (pos); //store new position as current position in main code
 
 // ***NOTE: Likely to need this component: https://tameson.com/valves/solenoid-valve/proportional/controller/316529-proportional-pwm-controller-12-24vdc-din-a-pg-burkert-316529.html
 //for PWM conversion of voltage signal, or possibly a voltage divider in circuit, but unsure about that
}
