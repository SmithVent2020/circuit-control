/**
 * SmithVent
 * Ventilator Control Circuit
 * 
 * modified on May 7, 2020
 * created by Dan, Farida, and Naylani
 */

 // initialized any global variables
 const int RELAY_ENABLE = 22;
 bool state; // initialized as false by default

void setup() {
  //initialize pin modes with pinMode command
  pinMode(RELAY_ENABLE, OUTPUT);

  Serial.begin(9600);

}

void loop() {
  // Run forever
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
