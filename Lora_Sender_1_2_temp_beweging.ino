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
 *     Project    : Lora Sender
 *     Versie     : 1.2
 *     Datum      : 10-2023
 *     Schrijver  : Ap Matteman
 *     
 *     Versie 1.0 : basic sender
 *     Versie 1.1 : Destination toegevoegd als herkenning van eigen LoRa commando's
 *     Versie 1.2 : Deep Sleep voor energiebesparing voor Accu monitoring
 */    



#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
#define uS_TO_S 1000000 //micro seconds to seconds
#define DHTPIN 4     // pin voor DHT22 sensor
#define DHTTYPE DHT22 // DHT22 (AM2302)
const int PIRPIN = 12; // ping voor bewegingssensor
const String Destination = "4Me!";

int pirValue;  // de waarde van de PIR

// zorg ervoor dat de temperatuur en vochtigheid niet te vaak worden verstuurd.
unsigned long interval = 15000UL;
unsigned long StartTijd = 0UL;
unsigned long HuidigeTijd = 0UL;

DHT dht(DHTPIN, DHTTYPE);


RTC_DATA_ATTR float Volt = 10.0;  // Fictieve waarde om te testen. Moet vervangen worden door een sensor!
                                  // place it in RTC memory to store data during Sleeo

void setup() {
  delay(500);   // 500ms after startup 

  // PIR
  pinMode(PIRPIN, INPUT);
  // DHT22
  pinMode(DHTPIN, INPUT);
  dht.begin();

  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  if (!LoRa.begin(866E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
}

//Send Lora Packet
void SendLora(String LoraPayload) {
  String LoRaData;
  LoRaData = Destination + LoraPayload;
  LoRa.beginPacket();   
  LoRa.print(LoRaData); // Max 256 Bytes long  
  LoRa.endPacket();

  // Serial.print("Send Destination: "); Serial.println(Destination);
  // Serial.print("Send LoraPayload: "); Serial.println(LoraPayload);
}


void loop() {
  String SendPackage;

  int pirStatus = digitalRead(PIRPIN);
  if(pirStatus != pirValue)
  {
    // van geen beweging naar beweging of van geen beweging naar beweging
    pirValue = pirStatus;
     Serial.println("...............................................");
    SendPackage = "sensor.schuur/beweging";
    SendPackage = SendPackage + "                                        ";   // adding spaces to the end                  
    SendPackage = SendPackage.substring(0,40);  // Limit MQTT Topic = max 40 long
    SendPackage = SendPackage + pirValue;
    Serial.print("SendPackage: "); Serial.println(SendPackage);
    SendLora(SendPackage);  //Send LoRa packet to receiver
    delay(500);  //Nodig om de GateWay de gegevens te laten versturen via MQTT
    Serial.println("// End Send");
  }
  
  HuidigeTijd = millis();

  if(HuidigeTijd - StartTijd > interval)
  {
    //Stuur alle waarden door
    StartTijd = millis();

    Serial.println("...............................................");
    SendPackage = "sensor.schuur/beweging";
    SendPackage = SendPackage + "                                        ";   // adding spaces to the end                  
    SendPackage = SendPackage.substring(0,40);  // Limit MQTT Topic = max 40 long
    SendPackage = SendPackage + pirValue;
    Serial.print("SendPackage: "); Serial.println(SendPackage);
    SendLora(SendPackage);  //Send LoRa packet to receiver

    delay(500);  //Nodig om de GateWay de gegevens te laten versturen via MQTT


    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    SendPackage = "sensor.schuur/temperatuur";
    SendPackage = SendPackage + "                                        ";   // adding spaces to the end                  
    SendPackage = SendPackage.substring(0,40);  // Limit MQTT Topic = max 40 long
    SendPackage = SendPackage + temp;
    Serial.print("SendPackage: "); Serial.println(SendPackage);
    SendLora(SendPackage);  //Send LoRa packet to receiver

    delay(500);  //Nodig om de GateWay de gegevens te laten versturen via MQTT

    SendPackage = "sensor.schuur/vochtigheid";
    SendPackage = SendPackage + "                                        ";   // adding spaces to the end                  
    SendPackage = SendPackage.substring(0,40);  // Limit MQTT Topic = max 40 long
    SendPackage = SendPackage + hum;
    Serial.print("SendPackage: "); Serial.println(SendPackage);
    SendLora(SendPackage);  //Send LoRa packet to receiver

    Serial.println("// End Send");
  }
  
}
