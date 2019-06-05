#include <BH1750FVI.h>
#include <timer.h>
#include "dht.h"

#define dht_apin 2 // Analog Pin sensor is connected to
#define defaultTemp 25.00

dht DHT;
int sensor_pin = A0; // Soil Sensor input at Analog PIN A0
int output_value ;
int luxMap;
double temperature;
boolean ReadMoisture = false;
boolean ReadLight = false;
boolean ReadTemp = false;

// Create the Lightsensor instance
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
auto timer = timer_create_default();

void setup() {
  // put your setup code here, to run once:
  pinMode(4,OUTPUT);
  Serial.begin(9600);
  LightSensor.begin(); 
  timer.every(6000, GetSensorLightIntensity); 
  timer.every(5000, GetMoisture); 
  timer.every(5000, GetTemp);
  Serial.println("Arduino is active!");
  delay(2000);
}
 
void loop() {
  timer.tick();
  if(ReadMoisture){
    if(output_value < 0){
      digitalWrite(4,HIGH);
      delay(2000);
      digitalWrite(4,LOW);
    }
    else{
      digitalWrite(4,LOW);
   }
   ReadMoisture = false;
  }
  if(ReadLight){
     if(luxMap < 50)
        Serial.println("Open lights!");
     else
        Serial.println("Close lights!");
    ReadLight = false;
  }
  if(ReadTemp){
     if(temperature >= defaultTemp){
        Serial.println("Open Fan!");
     }else{
        Serial.println("Close Fan!");
     }
     ReadTemp = false;
  }
}

void GetTemp(){
  DHT.read11(dht_apin);
  temperature = DHT.temperature;
  Serial.print("temperature = ");
  Serial.print(temperature); 
  Serial.println("C  ");
  ReadTemp = true;
}

void GetMoisture(){
   output_value= analogRead(sensor_pin);
 
   output_value = map(output_value,550,10,0,100);
   Serial.print("Mositure : ");
 
   Serial.print(output_value);
 
   Serial.println("%");
   
   ReadMoisture = true;
}

void GetSensorLightIntensity(){
  uint16_t lux = LightSensor.GetLightIntensity();
  luxMap = map(lux, 0, 1023, 0, 100);

  // in case the sensor value is outside the range seen during calibration
  luxMap = constrain(luxMap, 0, 100);
  Serial.print("Light: ");
  Serial.print(luxMap);
  Serial.println(" %");
  ReadLight = true;
}
