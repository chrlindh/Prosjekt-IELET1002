#include <Wire.h>

void setup() 
{
  Serial.begin(9600);
}

void loop()
{
  String batteriProsent;
  while(Serial.available()) {
    delay(5);
    float received = (float)Serial.read();
    batteriProsent = received;
    Serial.println(batteriProsent);
  }
  //Serial.println(batteriProsent);
  delay(1000);


  
//  do {
//    distanceTraveled = (counts/8000);
//    
//  } while(counts < 80); 
//  postTimeTraveled = millis();
//  timeTraveled = (postTimeTraveled - preTimeTraveled)/10;
//  centimeterCount++;
//  batteriProsent = map(centimeterCount, 0, 2000, 100, 0); 
}
