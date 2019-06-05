 #include <Wire.h>
#include <LiquidCrystal.h> 
#include <timer.h>
#include <EEPROM.h>

#define calibrationTime 10

#define Contrast 75

#define IN1  8
#define IN2  9
#define IN3  10
#define IN4  13

#define pirPin 7
#define ledPin A1

int StandartTimeToCloseDoor = 5000; //5 sec

auto timer = timer_create_default();

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  
boolean line1IsSet = false;
boolean line2IsSet = false;

const byte slaveId = 1;

String activity = "";
String action = "";

unsigned long timeDoorOpened;
boolean IsOpen = false;

int cstep = 0;

//the time when the sensor outputs a low impulse
long unsigned int lowIn; 
 
//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 500; 

boolean lockLow = true;
boolean takeLowTime;  

int eeAddress = 0; 

void setup() {
  Serial.begin(9600);
  
  analogWrite(6,Contrast);
  lcd.begin(16, 2);
  
  Serial.println("Ready to Receive!");
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 

  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);

  Wire.begin(slaveId);
  Wire.onReceive(receiveEvent);
  
  if( EEPROM.get(eeAddress, IsOpen)){
    Serial.println(EEPROM.get(eeAddress, IsOpen),3);
    EEPROM.put(eeAddress, IsOpen);
  }else{
    Serial.println(EEPROM.get(eeAddress, IsOpen),3);
    EEPROM.get(eeAddress, IsOpen); 
    if(IsOpen){
      for(int i=0;i<700;i++){
        Close();
        delayMicroseconds(900);
      }
      IsOpen = false;
      EEPROM.put(eeAddress, IsOpen);                                                                                                                                                                                                                                                                                     
    }
  }
  delay(50); 
}

void loop() {
  delay(100);
  if(IsOpen){
    if((millis() - timeDoorOpened) >= StandartTimeToCloseDoor ){
      for(int i=0;i<700;i++){
        Close();
        delayMicroseconds(900);
      }
      IsOpen = false;
      EEPROM.put(eeAddress, IsOpen);
    }
  }
  if(digitalRead(pirPin) == HIGH){
    OpenEverything();
  }
  if(digitalRead(pirPin) == LOW){  
    CloseEverything();
  }
}


void receiveEvent(int howMany) {

  String s = "";
  while (1 <= Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    s += c;
  }
  
  activity = s;
  if(activity.length() < 10){
    
    if(!IsOpen){
      OpenHouseDoor();
      delay(1000);
      return;
    }else{
      if((millis() - timeDoorOpened) >= StandartTimeToCloseDoor ){
        for(int i=0;i<700;i++){
          Close();
          delayMicroseconds(900);
        }
        IsOpen = false;
        EEPROM.put(eeAddress, IsOpen);
        printMessage("Door Closed");
      }
      delay(1000);
      return;           
    }
  }else{
    printMessage(s);
  }

  Serial.println(s);
  int x = Wire.read();    // receive byte as an integer
}

void printMessage(String msg){
 
  if(line2IsSet){
    line2IsSet = false;
    ClearLCD();
    lcd.setCursor(0, 0);
    lcd.print(msg);
  }
  if(!line1IsSet){
    lcd.setCursor(0, 0);
    lcd.print(msg);
    line1IsSet = true;
  }else if(!line2IsSet){
    lcd.setCursor(0, 1);
    lcd.print(msg);
    line2IsSet = true;
  }
}

void OpenHouseDoor(){
  for(int i=0;i<700;i++){
    Open();
    delayMicroseconds(900);
  }
  timeDoorOpened = millis();
  IsOpen = true;
  EEPROM.put(eeAddress, IsOpen);
  printMessage("Welcome Home!");
  delay(1000);
  for(int i=0;i<700;i++){
    Close();
    delayMicroseconds(900);
  }
  IsOpen = false;
  EEPROM.put(eeAddress, IsOpen);
  printMessage("Door Closed");
}

//if(digitalRead(pirPin) == HIGH)
void OpenEverything(){
  
    digitalWrite(ledPin, HIGH);   //the led visualizes the sensors output pin state
    if(lockLow){  
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;            
      String msg = "Movement Start";
      printMessage(msg);
    }         
    takeLowTime = true;
  
}

//if(digitalRead(pirPin) == LOW) 
void CloseEverything(){  
    digitalWrite(ledPin, LOW);  //the led visualizes the sensors output pin state
    if(takeLowTime){
      lowIn = millis();          //save the time of the transition from high to LOW
      takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause, 
    //we assume that no more motion is going to happen
    if(!lockLow && millis() - lowIn > pause){  
      //makes sure this block of code is only executed again after 
      //a new motion sequence has been detected
      lockLow = true;        
      String msg = "Movement End";
      printMessage(msg);              
    }
}

void Open()
{
  switch(cstep)
  {
   case 0:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   case 1:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, HIGH);
   break; 
   case 2:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 3:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 4:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 5:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
     case 6:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 7:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   default:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
  }
   
   cstep=cstep+1;
   if(cstep==8)
     {cstep=0;}
}

void Close(){
  switch(cstep)
    {
      case 0:
        digitalWrite(IN1, HIGH); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break; 
      case 1:
        digitalWrite(IN1, HIGH); 
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break; 
      case 2:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break; 
      case 3:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, HIGH);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break; 
      case 4:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
        break; 
      case 5:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, HIGH);
        break; 
      case 6:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break; 
     case 7:
        digitalWrite(IN1, HIGH); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
        break; 
     default:
        digitalWrite(IN1, LOW); 
        digitalWrite(IN2, LOW);
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, LOW);
        break; 
    }
   
    cstep=cstep+1;
    if(cstep==8)
     {cstep=0;}
}

void ClearLCD(){
  printMessage("                ");
  delay(50);
  printMessage("                ");
}
