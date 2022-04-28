#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// SSID & Password - telefon
const char* ssid = "Altibox257862";  // Enter your SSID here
const char* password = "6h5yV6TU";  //Enter your Password here

const char* mqttServer = "192.168.10.160";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "#mqtt1213";

WiFiClient espClient;
PubSubClient client(espClient);


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

    if(client.connect("ESP32Client", mqttUser, mqttPassword)) {
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
  client.loop();
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
    client.publish("homeassistant/esp/Zumo", "Charging...");
  }
}
