#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// SSID & Password - Ruter
const char* ssid = "Altibox257862";  // Enter your SSID here
const char* password = "6h5yV6TU";  //Enter your Password here

const char* mqttServer = "192.168.10.160";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "#mqtt1213";


int batteryPercentReceived;
int chargingStationReceived;
int speedAverageReceived;
int speedNowReceived;
int meterCountReceived;
int received;

int averageSpeed;

WiFiClient esp2Client;
PubSubClient client(esp2Client);


void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED)  {
   delay(500);
   Serial.println("Connecting to WiFi..."); 
  }
  Serial.println("WiFi connected.");

  client.setServer(mqttServer, mqttPort);

  while(!client.connected())  {
    Serial.println("Connecting to MQTT...");

    if(client.connect("ESP32Client_Zumo", mqttUser, mqttPassword)) {
      Serial.println("Connected");   
    }

    else  {
      Serial.print("Failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void loop() {
  while(Serial.available()) {
    unsigned long timeTraveled = millis();
    delay(1);
    received = (int)Serial.read();
    //Serial.print("Main input: ");
    //Serial.println(received);
    if ((received <= 100) && (received >= 0)) {
      batteryPercentReceived = received;
      //Serial.print("ESP32: ");
      //Serial.println(batteryPercentReceived);
      char buffer1[64];
      int batteryReturn = snprintf(buffer1, sizeof buffer1, "%d", batteryPercentReceived);
      client.publish("homeassistant/esp/battery", buffer1);
    }
    else if (received == 101) {
      chargingStationReceived = received;
      char buffer2[64];
      int requestReturn = snprintf(buffer2, sizeof buffer2, "%d", chargingStationReceived);
      client.publish("homeassistant/esp/chargerequest", buffer2);
    }
//    else if ((received >= 150) && (received <= 152)) {
//      speedAverageReceived = (((received) - 150)/10);
//      Serial.println(speedAverageReceived);
//      char buffer3[64];
//      int speedAvReturn = snprintf(buffer3, sizeof buffer3, "%d", speedAverageReceived);
//      client.publish("homeassistant/esp/speedaverage", buffer3);
//    }
    else if ((received >= 180) && (received <= 182)) {
      speedNowReceived = received - 180;
      Serial.print("Speed now: ");
      Serial.println(speedNowReceived);
      char buffer4[64];
      int speedNowReturn = snprintf(buffer4, sizeof buffer4, "%d", speedNowReceived);
      client.publish("homeassistant/esp/speednow", buffer4);
    }
    else if (received >= 200) {
      meterCountReceived = received - 200;
      Serial.print("meter count: ");
      Serial.println(meterCountReceived);
      char buffer5[64];
      int meterCountReturn = snprintf(buffer5, sizeof buffer5, "%d", meterCountReceived);
      client.publish("homeassistant/esp/metercount", buffer5);
    }
    averageSpeed = meterCountReceived/(timeTraveled/1000);
    //Serial.print("Average speed: ");
    //Serial.println(averageSpeed);

  }






  
  if (received == 101){
    client.publish("homeassistant/esp/chargerequest", "Dog at charging station, please charge him");
  }
    
//  if (batteriProsent == 90){
//    Serial.write("p");
//    client.publish("homeassistant/esp/Zumo", "Low Battery, Request Charging. Cost: 200NOK");
//  }
//  else if (batteriProsent <= 5 && batteriProsent >=0) {
//    Serial.write("q");
//    client.publish("homeassistant/esp/Zumo", "Critical Low Battery! Request Charging. Cost: 200NOK");
//  }


  
//    char buffer1[64];
//    int ret = snprintf(buffer1, sizeof buffer1, "%d", 100);
//    client.publish("homeassistant/esp/battery", buffer1);
  
}
