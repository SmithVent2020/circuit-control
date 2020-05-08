/*This file contains the functions to turn the alarms on and off
 * to turn the alarm on you need to pass the Activate_Alarm function a list of alarm messages
 * as strings and a list of the priorities coresponding to those alarm messages, where
 * true means high priority and false means medium priority
 * the part where the alarm messages are displayed on the LCD needs to be added,
 * the LCD updating is partially coded in commented code in the functions dfd 
 */
int yellow_LED_pin = 4;
int red_LED_pin = 3;
int buz_pin = 2;

void Activate_Alarm(String alarm_messages[], bool priorities[]){
  //This function activates the alarm and LEDs according to alarm priority

  //turn on LEDs according to priority of messages
  for(int i=0; i<(sizeof(priorities)/sizeof(priorities[0]))-1; i+=1){
    if(priorities[i]== true){
      digitalWrite(red_LED_pin, HIGH);
    }
    else if(priorities[i] == false){
      digitalWrite(yellow_LED_pin, HIGH);
    }
  }
  //  for(int i=0; i<sizeof(alarm_messages)/sizeof(alarm_messages[0]); i+=1){
  //    //print alarm message to LCD screen
  //  }
  
  //turn on buzzer
  tone(buz_pin,261,500);

  digitalWrite(red_LED_pin, LOW);
  digitalWrite(yellow_LED_pin, LOW);
  noTone(buz_pin);
  
}

void Deactivate_Alarm(){
  digitalWrite(red_LED_pin, LOW);
  digitalWrite(yellow_LED_pin, LOW);
  noTone(buz_pin);
  //remove alarm messages from the LCD screen
  
}
void setup() {
  // put your setup code here, to run once:
  pinMode(red_LED_pin, OUTPUT);
  pinMode(yellow_LED_pin, OUTPUT);
  pinMode(buz_pin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  String alarm_messages[] = { "a", "b", "c"};
  bool priorities[] = {true, false, false}; //high priority is true, medium priority is false 
  
  Activate_Alarm(alarm_messages, priorities);
  
  

}
