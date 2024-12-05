#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//-----------------------
#include "DHT.h"
#define DHTPIN 0 //D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
//-----------------------
#define analogSmoke A0
#define digitizedSmoke 5 //D1
#define digitizedTemperature 4 //D2
#define smokeOuput 14 //D5
#define fireOutput 12 //D6

const char* ssid = "T";
const char* password = "12345678";

const char* mqtt_server = "broker.hivemq.com"; 
const int mqtt_port = 1883;
const char* mqtt_Client = "clientId-YEcoAYpXb4";  //ClientID
const char* mqtt_username = "";  //Token
const char* mqtt_password = ""; //Secret

WiFiClient espClient; 
PubSubClient client(espClient);

long lastMsg = 0;
int value = 0;
char msg[100];
String DataString;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) { //เชื่อมต่อกับ MQTT BROKER
    Serial.println("connected");
    //client.subscribe("bruh/led/status");
  }
  else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println("try again in 5 seconds");
    delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message = message + char(payload[i]);
  }
  Serial.println(message);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(digitizedSmoke, OUTPUT);
  pinMode(digitizedTemperature, OUTPUT);
  pinMode(smokeOuput, INPUT);
  pinMode(fireOutput, INPUT);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); //เชื่อมต่อกับ WIFI
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //เชื่อมต่อกับ WIFI สำเร็จ แสดง IP
  client.setServer(mqtt_server, mqtt_port); //กำหนด MQTT BROKER, PORT ที่ใช้
  client.setCallback(callback); //ตั้งค่าฟังก์ชันที่จะทำงานเมื่อมีข้อมูลเข้ามาผ่านการ Subscribe
  //client.subscribe("gall/led/status");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000) { //จับเวลาส่งข้อมูลทุก ๆ 5 วินาที
    lastMsg = now;
    ++value;

    //MQ-2
    /*if (smokeinput == HIGH){
      send_message();
    }*/
    
    float s = readSensor();
    float t = dht.readTemperature();
    float so = smokeOutput();
    float fo = fireoutput();
    if (t > 25) {
      digitalWrite(digitizedTemperature, HIGH);
    }

    Serial.print("Smoke Sensor : ");
    Serial.println(s);
    Serial.print(" Temperature : ");
    Serial.println(t);
    Serial.print(" Smoke form IC : ");
    Serial.println(so);
    Serial.print(" Fire form IC : ");
    Serial.println(fo);
    Serial.println();
    
    
    DataString =
    "{\"smoke\":"+(String)s
    + ",\"temperature\":"+(String)t
    + ",\"so\":"+(String)so
    + ",\"fo\":"+(String)fo
    + "}";
    DataString.toCharArray(msg, 100);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("gal/dht/temp", msg);
  
  delay(500);
}
}

int readSensor() {
  unsigned int sensorValue = analogRead(analogSmoke);  // Read the analog value from sensor
  unsigned int outputValue = map(sensorValue, 0, 1023, 0, 255); // map the 10-bit data to 8-bit data
  if (outputValue > 65) {
    digitalWrite(digitizedSmoke,HIGH); 
  }
  else {
    digitalWrite(digitizedSmoke,LOW);
  }
  return outputValue;             // Return analog moisture value
}

int smokeOutput() {
  if (digitalRead(smokeOuput) == HIGH){
    return 1;
  }
  else if (digitalRead(smokeOuput) == LOW){
    return 0;
  } else return 0;
}

float fireoutput() {
  if (digitalRead(fireOutput) == HIGH){
    return 1;
  }
  else if (digitalRead(fireOutput) == LOW){
    return 0;
  }  else return 0;
}
