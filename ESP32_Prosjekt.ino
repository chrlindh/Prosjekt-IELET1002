#include <Wire.h>

void setup() 
{
  Serial.begin(9600);
}

void loop()
{
  float batteriProsent;
  while(Serial.available()) {
    delay(1);
    float received = (float)Serial.read();
    batteriProsent = received;
    Serial.print("ESP32: ");
    Serial.println(batteriProsent);
  }
  
  if (batteriProsent == 90){
    Serial.write("p");
  }
  
}
