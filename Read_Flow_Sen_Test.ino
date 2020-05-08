//SmithVent
//This test code reads inspiratory and expiratory flow sensor analog outputs and prints on the serial monitor

////////////////////////Setup////////////////////////////////
#define flow_sen_in_pin A0  //  Inspiriatory flow sensor analog output connected to analog pin 0
#define flow_sen_ex_pin A1  //  Expiriatory flow sensor analog output connected to analog pin 1

float flow_read_in = 0;  // variable to store the inspiratory flow value read
float flow_read_ex = 0;  // variable to store the inspiratory flow value read



////////////////////////Test Code////////////////////////////
void setup() {
  Serial.begin(9600);  //  setup serial
}

void loop() {
  flow_read_in = Read_Flow_Sen_In();  // read the inspiratory flow sensor
  flow_read_ex = Read_Flow_Sen_Ex();  // read the expiratory flow sensor

  // debug value
  Serial.print("Inspiratory flow: "); Serial.print(flow_read_in);  
  Serial.print(" Expiratory flow: "); Serial.print(flow_read_ex); 
  Serial.println();
  delay(10);
}


///////////////////////////Function//////////////////////////
//
//The Read_Flow_Sen_In() function returns the inspiratory flow sensor value in SLPM. The sensor is attached to the predefined flow_sen_in_pin
//
float Read_Flow_Sen_In(){
  int sensor_read = analogRead(flow_sen_in_pin);
  float sensor_volume = sensor_read*5.0*150/4/1023.0-18.75; //sensor_read(0.5-4.5 V) maps linearly to flow_read(0-150 SLPM)
  return sensor_volume;
}

//
//The Read_Flow_Sen_Ex() function returns the expiratory flow sensor value in SLPM. The sensor is attached to the predefined flow_sen_ex_pin
//
float Read_Flow_Sen_Ex(){
  int sensor_read = analogRead(flow_sen_ex_pin);
  float sensor_volume = sensor_read*5.0*150/4/1023.0-18.75; //sensor_read(0.5-4.5 V) maps linearly to flow_read(0-150 SLPM)
  return sensor_volume;
}
