//SmithVent
//This test code reads mixing chamber, inspiratory and expiratory pressure sensor analog outputs and prints on the serial monitor

////////////////////////Setup////////////////////////////////
#define p_sen_chamber_pin A5  //  Mixing chamber pressure sensor analog output connected to analog pin 5
#define p_sen_in_pin A6  //  Inspiriatory pressure sensor analog output connected to analog pin 6
#define p_sen_ex_pin A7  //  Expiriatory pressure sensor analog output connected to analog pin 7

float p_read_chamber = 0;  // variable to store the mixing chamber pressure value read
float p_read_in = 0;  // variable to store the inspiratory pressure value read
float p_read_ex = 0;  // variable to store the exspiratory pressure value read



////////////////////////Test Code////////////////////////////
void setup() {
  Serial.begin(9600);  //  setup serial
}

void loop() {
  p_read_chamber = Read_P_Sen_Chamber();  // read the mixing chamber pressure sensor
  p_read_in = Read_P_Sen_In();  // read the inspiratory pressure sensor
  p_read_ex = Read_P_Sen_Ex();  // read the expiratory pressure sensor

  // debug value
  Serial.print("Mixing chamber pressure: "); Serial.print(p_read_chamber);  
  Serial.print(" Inspiratory flow: "); Serial.print(p_read_in);  
  Serial.print(" Expiratory flow: "); Serial.print(p_read_ex); 
  Serial.println();
  delay(10);
}


///////////////////////////Function//////////////////////////
//
//The Read_P_Sen_Chamber() function returns the mixing chamber pressure sensor value in cmH2O. The sensor is attached to the predefined p_sen_chamber_pin
//
float Read_P_Sen_Chamber(){
  int sensor_read = analogRead(p_sen_chamber_pin);
  float sensor_pressure = (sensor_read*5.0/1023.0-0.5)*326.31/4.0-163.155; //sensor_read(0.5-4.5 V) maps linearly to flow_read(+-163.155 cmH2O)
  return sensor_pressure;
}

//
//The Read_P_Sen_In() function returns the inspiratory pressure sensor value in cmH2O. The sensor is attached to the predefined p_sen_in_pin
//
float Read_P_Sen_In(){
  int sensor_read = analogRead(p_sen_in_pin);
  float sensor_pressure = (sensor_read*5.0/1023.0-0.5)*326.31/4.0-163.155; //sensor_read(0.5-4.5 V) maps linearly to flow_read(+-163.155 cmH2O)
  return sensor_pressure;
}

//
//The Read_P_Sen_Ex() function returns the expiratory pressure sensor value in cmH2O. The sensor is attached to the predefined p_sen_ex_pin
//
float Read_P_Sen_Ex(){
  int sensor_read = analogRead(p_sen_ex_pin);
  float sensor_pressure = (sensor_read*5.0/1023.0-0.5)*326.31/4.0-163.155; //sensor_read(0.5-4.5 V) maps linearly to flow_read(+-163.155 cmH2O)
  return sensor_pressure;
}
