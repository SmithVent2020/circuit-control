
//Valve status function for SmithVent
//Returns True if valve is open, False if valve is closed

//Direct queries/clarifications to Mandira (mandira.marambe@gmail.com)
//The function is written at the end; pins will have to be initialized in main code
//Compiled on Arduino 1.8.8

//----------------------------------------------------TEST CODE---------------------------------------------------------------

const int valve_pin = 2; //arbitrary pin number; depends on remaining pins used on board

void setup() {
Serial.begin(9600); //This is just for output from testing. Not needed for main code
}

void loop() {
  bool stat = Valve_Status(valve_pin); //Calling the function for test. Would be replaced in main code with relevant function
  Serial.println (stat); //Test return from function

}

//-----------------------------------------------------------FUNCTION---------------------------------------------------------

bool Valve_Status(int pin){     
if (digitalRead(pin)==HIGH){  //Valve should be open when power is supplied. If circuit is reversed, reverse logic
  Serial.println("valve is open");
  return true; //open
}
else if (digitalRead(pin)==LOW){   //can be made analog if necessary to read extent of open/closed valve
  Serial.println("valve is closed");
  return false; //closed
}else {
    Serial.println("Not a boolean output; Debug code") ;//Just to make sure :)
  }
}
