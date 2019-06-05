#include <Stepper.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <Wire.h>

// defines pins numbers
#define IN1  3
#define IN2  4
#define IN3  5
#define IN4  6

#define RST_PIN   9
#define SS_PIN   10

#define StandartTimeToCloseDoor 5000

// Create instances
MFRC522 mfrc522(SS_PIN, RST_PIN);

//Tag 1 = 1445dc73
//Tag 2 = 705bdba4

char* GarageTag[1] = {};
char* HouseTag[1] = {};
byte readCard[4];
String tagID = "";
boolean successRead = false;

const int trigPin = 8;
const int echoPin = 7;
const int stepsPerRevolution = 100;

// defines variables
long duration;
int distance;
double wallDistance;

int stepCount = 0;  // number of steps the motor has taken
int motorSpeed =100;
int UpDownTimes = 5800;
int x=0,cstep=0;
bool isDoorOpen = false;

unsigned long timeDoorOpened;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  
  Wire.begin();
  
  Serial.begin(9600); // Starts the serial communication
  wallDistance = getDistance();

  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 

  // Initiating
  SPI.begin();        // SPI bus
  mfrc522.PCD_Init(); //  MFRC522
  // Prints the initial message
  Wire.beginTransmission(1);
  Wire.write("Scan Home Tag");
  Wire.endTransmission(false);
  // Waits until a master card is scanned
  while (!successRead) {
    successRead = getID();
    if ( successRead == true) {
      HouseTag[0] = strdup(tagID.c_str()); // Sets the master tag into position 0 in the array
      Wire.beginTransmission(1);   Wire.write("Home Tag Set!");   Wire.endTransmission(false);
    }else{
      Wire.beginTransmission(1);   Wire.write("Home Tag fail!");   Wire.endTransmission(false);
    }
  }
  successRead = false;
  Wire.beginTransmission(1);   Wire.write("Scan Garage Tag");   Wire.endTransmission(false);
  while (!successRead) {
    successRead = getID();
    if ( successRead == true) {
      if(strcmp(HouseTag[0],strdup(tagID.c_str())) != 0 ){
        GarageTag[0] = strdup(tagID.c_str()); // Sets the master tag into position 0 in the array
        Wire.beginTransmission(1);   Wire.write("Garage Tag Set!");   Wire.endTransmission(false);
      }else{
        Wire.beginTransmission(1);   Wire.write("Try Other Tag!");   Wire.endTransmission(false);
        delay(500);
        Wire.beginTransmission(1);   Wire.write("Scan Garage Tag");   Wire.endTransmission(false);
        successRead = false;
      }
    }
  }
  successRead = false;
  printNormalModeMessage();
}

void loop() {
  double distance = getDistance();  
  //Success Scan a Tag card
  if(getID()){
    //TagID is for the House door
    String houseTag = (String)HouseTag[0];
    String garageTag = (String)GarageTag[0];
    if(houseTag.equals(tagID)){
      Wire.beginTransmission(1);   Wire.write("Open Door");   Wire.endTransmission(false);
      delay(1000);
      printNormalModeMessage();
      //TagID is for the Garage Door
    }else if(garageTag.equals(tagID)){
      //The door is closed
      if(!isDoorOpen){
        if(!isDoorOpen && distance <= wallDistance){
          while(x<UpDownTimes){
            OpenGarageDoor();
            delayMicroseconds(900);
            x++;
          }
          Wire.beginTransmission(1);   Wire.write("Garage is Open");   Wire.endTransmission(false);
          x=0;
          isDoorOpen = true;
          delay(1000);
        }
        
        timeDoorOpened = millis();
        isDoorOpen = true;
      //The door is Open
      }else{
        Wire.beginTransmission(1);   Wire.write("Already Opened");   Wire.endTransmission(false);
      }
    }else{
      Wire.beginTransmission(1);   Wire.write("Access Denied!");   Wire.endTransmission(false);
      delay(1000);
      printNormalModeMessage();
    }
  }
  //The door is Open
  if(isDoorOpen){
    if((millis() - timeDoorOpened) >= StandartTimeToCloseDoor ){
      //The is NO Obstacle between the wall and the sensor
      if(distance == wallDistance){
        
        while(x<UpDownTimes){
          CloseGarageDoor();
          delayMicroseconds(900);
          x++; 
        }
        isDoorOpen = false;
        x=0;
        delay(1000);
        printNormalModeMessage(); 
      }else{
        
        Wire.beginTransmission(1);   ("Obstacle in: ");   Wire.endTransmission(false);
        Wire.beginTransmission(1);   Wire.write(int(distance)+" cm");   Wire.endTransmission(false); 
      }
    }
  }
}

uint8_t getID() {
  tagID = "";
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  tagID = "";
  for ( uint8_t i = 0; i < 4; i++) {  // The MIFARE PICCs that we use have 4 byte UID
    readCard[i] = mfrc522.uid.uidByte[i];
    tagID.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tagID.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}


void printNormalModeMessage() {
  delay(1500);
  Wire.beginTransmission(1);   Wire.write(" Scan Your Tag!");   Wire.endTransmission(false);
}


double getDistance(){
// Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance= duration*0.034/2;  
  return distance;
}

void CloseGarageDoor(){

  //stepp
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
  

void OpenGarageDoor()
{
  //stepp
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
