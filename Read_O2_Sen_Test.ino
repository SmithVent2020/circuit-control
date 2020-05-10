//SmithVent
//This test code reads O2 sensor analog output and prints on the serial monitor

////////////////////////Setup////////////////////////////////
#define O2_sen_pin A8  //  O2 sensor analog output connected to analog pin 8

float O2_read = 0;  // variable to store the O2 value read



////////////////////////Test Code////////////////////////////
void setup() {
  Serial.begin(9600);  //  setup serial
}

void loop() {
  O2_read = Read_O2_Sen();  // read the O2 sensor

  // debug value
  Serial.print("O2 concentration: "); Serial.print(O2_read);  
  Serial.println();
  delay(10);
}


///////////////////////////Function//////////////////////////
//
//The Read_O2_Sen_() function returns the O2 sensor value in %. The sensor is attached to the predefined O2_sen_pin
//
float Read_O2_Sen(){
  analogReference(INTERNAL1V1);  //change analog pin reference voltage to 1.1V
  int sensor_read = analogRead(O2_sen_pin);
  sensor_read = analogRead(O2_sen_pin);  //discard the first reading after changing the reference voltage
  float sensor_O2 = sensor_read*1.1*10000/6.0/1023.0; //sensor_read(0.0-60.0 mV) maps linearly to O2_read(0-100%)
  return sensor_O2;
  analogReference(DEFAULT);  //change analog pin reference voltage back to 5.0V
  sensor_read = analogRead(O2_sen_pin);  //read again to avoid next wrong reading due to change of reference voltage
}
