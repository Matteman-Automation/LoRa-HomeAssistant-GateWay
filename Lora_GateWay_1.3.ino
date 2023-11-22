/*
 *     M      M     AA    TTTTTTT  TTTTTTT  EEEEEEE M      M     AA     NN    N
 *     MM    MM    A  A      T        T     E       MM    MM    A  A    NN    N
 *     M M  M M   A    A     T        T     E       M M  M M   A    A   N  N  N
 *     M  MM  M   AAAAAA     T        T     EEEE    M  MM  M   AAAAAA   N  N  N - AUTOMATION
 *     M      M  A      A    T        T     E       M      M  A      A  N   N N 
 *     M      M  A      A    T        T     E       M      M  A      A  N    NN  
 *     M      M  A      A    T        T     EEEEEEE M      M  A      A  N    NN  
 *     
 *     
 *     Project    : Lora GateWay
 *     Versie     : 1.3
 *     Datum      : 11-2023
 *     Schrijver  : Ap Matteman
 *
 *     Versie 1.0 : basic GateWay (receiver)
 *     Versie 1.1 : Destination toegevoegd als herkenning van eigen LoRa commando's
 *     Versie 1.2 : ConnectMQTT toegevoegd voor het ontvangen van MQTT en verzenden via LoRa  
 *     Versie 1.3 : MQTT aangepast ivbm ontvangen MQTT
 *     
 */    

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>                // communicate with SPI devices
#include <Wire.h>               // For OLED display
#include <Adafruit_SSD1306.h>   // For OLED display
#include <Adafruit_GFX.h>       // For OLED display
#include <LoRa.h>               // LoRa module
#include "arduino_secrets.h"    // My Passwords

// WiFi
const char *ssid = YourSSID; // Enter your Wi-Fi name
const char *password = YourWiFiPassWord;  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = YourMQTTserver;
const char *mqtt_username = YourMQTTuser;
const char *mqtt_password = YourMQTTpassword;
const int mqtt_port = 1883;
int WiFiTry = 0;                // nr of times the WiFi is not available
int MQTTtry = 0;                // nr of times the MQTT is not available
WiFiClient espClient;
PubSubClient MQTTclient(espClient);


//define LORA
#define ss 5
#define rst 14
#define dio0 2
const String MyAddress = "4Me!";  // LoRa identifier 

// Declaration for an SSD1306 OLED display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128        // OLED display width, in pixels
#define SCREEN_HEIGHT 64        // OLED display height, in pixels
#define OLED_RESET     4        // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
    
  // setup OLED Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //Connect to WiFi and MQTT
  WiFi.begin(ssid, password);
  Connect2WiFi();
  MQTTclient.setServer(mqtt_broker, mqtt_port);
  MQTTclient.setCallback(callback);
  Connect2MQTT();

  display.clearDisplay(); 
  display.setCursor(0,0); display.println("WiFi connected"); 
  display.setCursor(0,10); display.println("MQTT connected"); 
  display.setCursor(0,20); display.println("Connecting LoRa");
  display.display();

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  if (!LoRa.begin(866E6)) {
    Serial.println("Starting LoRa failed!");
    display.clearDisplay(); 
    display.setCursor(0,0); display.println("WiFi connected"); 
    display.setCursor(0,10); display.println("MQTT connected"); 
    display.setCursor(0,20); display.println("LoRa ERROR");
    display.display();
    sleep(5000);
    while (1);
  }
  else {
    display.clearDisplay(); 
    display.setCursor(0,0); display.println("WiFi connected"); 
    display.setCursor(0,10); display.println("MQTT connected"); 
    display.setCursor(0,20); display.println("LoRa connected");
    display.display();
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  // Convert Char* to String
  String StrTopic = topic;    
  payload[length] = 0;   String recv_payload = String(( char *) payload);
  
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
      Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  display.clearDisplay();
  display.setCursor(0,0); display.println("Receive MQTT");
  display.setCursor(0,10); display.println(StrTopic.substring(0,20));
  display.setCursor(0,20); display.println(recv_payload);
  display.display();  

  //Send message at the LoRa network
  String SendPackage;
  SendPackage = StrTopic;
  SendPackage = SendPackage + "                                        ";   // adding spaces to the end                  
  SendPackage = SendPackage.substring(0,40);  // Limit MQTT Topic = max 40 long
  SendPackage = SendPackage + recv_payload;
  SendPackage = MyAddress + SendPackage; 
  LoRa.beginPacket();   
  LoRa.print(SendPackage); // Max 256 Bytes long  
  LoRa.endPacket();

}

void Connect2WiFi() {
  //Connect with WiFi network
  display.clearDisplay(); 
  display.setCursor(0,0); display.println("Connecting WiFi"); 
  display.display();
  WiFiTry = 0;
   WiFi.begin(ssid, password);
   Serial.println("Connecting to WiFi");
  //Try to connect to WiFi for 11 times
  while (WiFi.status() != WL_CONNECTED && WiFiTry < 11) {

    ++WiFiTry;
    // Serial.print("WiFi status: ");; Serial.println(WiFi.status());
    // Serial.print("WiFiTry: ");; Serial.println(WiFiTry);
    delay(1000);
  }
  Serial.println("");
  Serial.print("WiFiTry: ");; Serial.println(WiFiTry);
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

}

void Connect2MQTT(){
  //connecting to a mqtt broker

  display.clearDisplay(); 
  display.setCursor(0,0); display.println("WiFi connected"); 
  display.setCursor(0,10); display.println("Connecting MQTT"); 
  display.display();
  MQTTtry = 0;
  while ((!MQTTclient.connected() && MQTTtry < 16)) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (MQTTclient.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
          break;
      } else {
          Serial.print("failed with state ");
          Serial.print(MQTTclient.state());
          delay(2000);
      } 
      ++MQTTtry;
      Serial.print("#");
  }
   // Subscribe, voor iedere Topic een regel. Wildcards werken (momenteel) niet :(
  MQTTclient.subscribe("lora/testlights/power");
  Serial.println("MQTT = Connected");
}

void loop() {
  // Receive MQTT
  if (!MQTTclient.connected()) { //MQTT connection lost :(
    if(WiFi.status() != WL_CONNECTED){  //Also a problem with WiFi
      Connect2WiFi();
    }
    Connect2MQTT();
    if(MQTTtry > 15) {  // problem, reconnect WiFi and MQTT
      WiFi.disconnect();
      Connect2WiFi();
      Connect2MQTT();
    }
  }
  MQTTclient.loop();

  // Receive LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {     //received a LoRa packet
    String LoRaData = LoRa.readString(); Serial.print("LoRaData: "); Serial.println(LoRaData);
    String ToAddress = LoRaData.substring(0,4); Serial.print("ToAddress: "); Serial.println(ToAddress);
    if (ToAddress == MyAddress) {      // Package is for me    
      String mqttTopic = LoRaData.substring(4,44);
      float mqttValue = LoRaData.substring(44).toFloat();
      float rssiValue = LoRa.packetRssi();
      mqttTopic.trim();
      //Send to Serial port
      Serial.print("mqttTopic: "); Serial.print(mqttTopic); Serial.println(".");
      Serial.print("mqttValue: "); Serial.println(mqttValue); 
      Serial.print("RSSI: "); Serial.println(rssiValue); 
      
      //Send to OLED Display
      display.clearDisplay();
      display.setCursor(0,0); display.println("Receive LoRa");
      display.setCursor(0,10); display.println(mqttTopic.substring(0,20));
      display.setCursor(0,20); display.println(mqttValue);
      display.setCursor(0,40); display.print("RSSI: "); display.println(rssiValue);
      display.display();  
     
      // Send to Home Assistant
      MQTTclient.publish(mqttTopic.c_str(), String(mqttValue).c_str());
      mqttTopic = mqttTopic + "/RSSI";
      MQTTclient.publish(mqttTopic.c_str(), String(rssiValue).c_str());
      Serial.println("Published to Home Assistant");    
      Serial.println("...............................................");
    }
  }
}