#include <ESP8266WiFi.h>  // WiFi
#include <WiFiClient.h>   // MQTT
#include <PubSubClient.h> // MQTT
#include <ESP8266mDNS.h>  // O
#include <WiFiUdp.h>      // T
#include <ArduinoOTA.h>   // A
#include <OneWire.h>      // For DallasTemperature.h
#include <DallasTemperature.h>

// Network credentials
const char* ssid = "Xiaomi_1662";
const char* password = "Xiaomi_1662";
// MQTT credentails
const char* deviceName =    "WemosHome2";
const char* willTopic =     "WemosHome2/will";
const char* inTopic =       "WemosHome2/in";
const char* d1Topic =       "WemosHome2/D1";
const char* d2Topic =       "WemosHome2/D2";
const char* d3Topic =       "WemosHome2/D3";
const char* d4Topic =       "WemosHome2/D4";
const char* d5Topic =       "WemosHome2/D5";
const char* d6Topic =       "WemosHome2/D6";
const char* d7Topic =       "WemosHome2/D7"; 
const char* d8Topic =       "WemosHome2/D8";
const char* d9Topic =       "WemosHome2/D9";
const char* a0Topic =       "WemosHome2/A0";
const char* shttTopic =     "WemosHome2/SHTT";
const char* shthTopic =     "WemosHome2/SHTH";
const char* shtdTopic =     "WemosHome2/SHTD";
const char* mqtt_server =   "virsi.mooo.com";
WiFiClient espClient;
PubSubClient client(espClient);

// DallasTemperature sensors
#define ONE_WIRE_BUS 0 //D3
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensorsDS18B20(&oneWire);

// For mqtt_sendmsg()
long lastMsg = 0;
int timeBetweenSendMsg = 5000;

void setup()  {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Booting");
  Serial.println(deviceName);
  setupWiFi();
  setupArduinoOTA();
  setupMQTT();
  setupSensorsDS18B20();
}

void loop() {
  ArduinoOTA.handle();
  MQTTHandle();
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
}

void setupArduinoOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("device ID: ");
  Serial.println(ESP.getChipId(),HEX);
}

void setupMQTT(){
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  
}

void setupSensorsDS18B20(){
  sensorsDS18B20.begin();
  sensorsDS18B20.setResolution(12);  
  Serial.print("Sensors resolution is ");
  Serial.print(sensorsDS18B20.getResolution());
  Serial.println(" bits");
}

void MQTTHandle(){
  if (WiFi.status() == WL_CONNECTED){
    if (!client.connected()) {
      reconnect();
    }
    if (client.connected()) {
      mqtt_sendmsg();
    } 
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(),willTopic,2,false,"-999")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe(inTopic);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("have to turn on");// Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  Serial.println("have to turn off"); }

}

void mqtt_sendmsg(){
client.loop();
 
  
  long now = millis();
  if (now - lastMsg > timeBetweenSendMsg) {
    lastMsg = now;
    
    
    int deviceCount  = sensorsDS18B20.getDeviceCount();
    Serial.print("Device count: ");
    Serial.println(deviceCount);
    if(deviceCount == 0){
      Serial.println("No DS18B20 sensors detected");
    }
    if(deviceCount != 0){
      sensorsDS18B20.requestTemperatures();
      for (uint8_t i = 0; i < deviceCount; i++){
        char buf[20];
        float ds18b20 = (sensorsDS18B20.getTempCByIndex(i)) * 100;
        itoa(ds18b20, buf, 10);
        Serial.print("Temperature ");
        Serial.print(i+1);
        Serial.print(": ");
        Serial.println(ds18b20 / 100);
        
        switch (i){
          case 0:
            client.publish(d1Topic, buf);
            break;
          case 1:
            client.publish(d2Topic, buf);
            break;
          case 2:
            client.publish(d3Topic, buf);
            break;           
        }
      }
    }


//
//    itoa(sensors.getDeviceCount(), buf, 10);
//    client.publish(d9Topic, buf);
//    
//    temperature = -127;
//    humidity = -127;
//    dewpoint = -127;
//    tempSensor.measure(&temperature, &humidity, &dewpoint);
//    itoa(temperature, buf, 10);
//    client.publish(shttTopic, buf);
//    itoa(humidity, buf, 10);
//    client.publish(shthTopic, buf);
//    itoa(dewpoint, buf, 10);
//    client.publish(shtdTopic, buf);
    char buf[20];
    int a0 = analogRead(A0);
    itoa(a0, buf, 10);
    client.publish(a0Topic, buf);
    Serial.print("Analog input: ");
    Serial.println(a0);
  }
}
