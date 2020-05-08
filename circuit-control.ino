/**
 * SmithVent
 * Ventilator Control Circuit
 * 
 * modified on May 7, 2020
 * created by Dan, Farida, and Naylani
 */

 // initialized any global variables
//Valve input and putput pins
 const int RELAY_ENABLE = 22;
 const int VALVE_READ = 21; //Reading pin from same valve

 bool state; // initialized as false by default
 bool read_state; //initialized as false

void setup() {
  //initialize pin modes with pinMode command
  pinMode(RELAY_ENABLE, OUTPUT);
 pinMode(VALVE_READ, INPUT);

  Serial.begin(9600);

}

void loop() {
  // Run forever
  read_state = Valve_Status(VALVE_READ); //Calling the function for test. Would be replaced in main code with relevant function
  Serial.println (read_state); //Test return from function
 
  actuate_valve(state);
  state = !state;
  
  delay(1000);
}

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
if (digitalRead(pin)==HIGH){  //Valve should be open when power is supplied. If circuit is reversed, reverse logic
  Serial.println("valve is open");
  return true; //open
}
else if (digitalRead(pin)==LOW){   
  Serial.println("valve is closed");
  return false; //closed
}
}
