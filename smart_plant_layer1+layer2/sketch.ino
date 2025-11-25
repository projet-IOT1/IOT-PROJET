// =============================================
// PROJECT LINK IN WOKWI: https://wokwi.com/projects/444282871410335745
// =============================================


#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
// #include <ESP32Servo.h>  // COMMENTED OUT - Servo removed
#include "DHT.h"

// =============================================
// CONFIGURATION
// =============================================
const char* ssid = "ORANGE-DIGITAL-CENTER";
const char* password = "Welcome@2023";
const char* MQTT_SERVER = "028e2afc7bff49aa9758e69dfebd7b17.s1.eu.hivemq.cloud";
const int MQTT_PORT = 8883;
const char* MQTT_USER = "hope_231";
const char* MQTT_PASSWORD = "Japodisehell1234";

// Pins
#define RGB_RED_PIN 14
#define RGB_GREEN_PIN 12  
#define RGB_BLUE_PIN 13
// #define SERVO_PIN 27  // COMMENTED OUT - Servo removed
#define SOIL_MOISTURE_PIN 34
#define LDR_PIN 35
#define PUMP_PIN 27 // initialy pin 26
#define DHT_PIN 15
#define DHT_TYPE DHT11

// MQTT Connection Settings - OPTIMIZED
#define MQTT_KEEPALIVE 15
#define MQTT_SOCKET_TIMEOUT 5
#define MQTT_BUFFER_SIZE 512
#define RECONNECT_DELAY 2000
#define MAX_RECONNECT_ATTEMPTS 5

// =============================================
// GLOBALS
// =============================================
String deviceId = "planty_" + String(ESP.getEfuseMac(), HEX);
String telemetryTopic = "planty/" + deviceId + "/telemetry";
String commandTopic = "planty/" + deviceId + "/commands";

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
// Servo fanServo;  // COMMENTED OUT - Servo removed
DHT dht(DHT_PIN, DHT_TYPE);

bool pumpRunning = false;
unsigned long pumpStartTime = 0;
unsigned long pumpDuration = 0;
unsigned long lastReconnectAttempt = 0;
int reconnectAttempts = 0;

// Root CA Certificate
static const char* ROOT_CA PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// =============================================
// SENSOR FUNCTIONS
// =============================================
float readSoilMoisture() {
  int raw = analogRead(SOIL_MOISTURE_PIN);
  Serial.print("Soil RAW: "); Serial.println(raw);
  
  int moisturePercent = map(raw, 4095, 1500, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);
  return moisturePercent;
}

float readLightLevel() {
  int raw = analogRead(LDR_PIN);
  Serial.print("LDR RAW: "); Serial.println(raw);
  
  int lightPercent = map(raw, 4095, 100, 0, 100);
  lightPercent = constrain(lightPercent, 0, 100);
  return lightPercent;
}

void printSensorReadings() {
  Serial.print("Soil:"); Serial.print(readSoilMoisture()); Serial.print("% ");
  Serial.print("Light:"); Serial.print(readLightLevel()); Serial.print("% ");
  Serial.print("Temp:"); Serial.print(dht.readTemperature()); Serial.print("C ");
  Serial.print("Hum:"); Serial.print(dht.readHumidity()); Serial.println("%");
}

// =============================================
// PUMP CONTROL
// =============================================
void updatePump() {
  if (pumpRunning && (millis() - pumpStartTime >= pumpDuration)) {
    digitalWrite(PUMP_PIN, LOW);
    pumpRunning = false;
    Serial.println("Pump auto-stop");
  }
}

void startPump(int duration) {
  digitalWrite(PUMP_PIN, HIGH);
  pumpRunning = true;
  pumpStartTime = millis();
  pumpDuration = duration;
  Serial.println("Pump ON for " + String(duration) + "ms");
}

void stopPump() {
  digitalWrite(PUMP_PIN, LOW);
  pumpRunning = false;
  Serial.println("Pump STOPPED");
}

// =============================================
// DEVICE CONTROL
// =============================================
String ledColor = "OFF";

void setLedColor(String color) {
  ledColor = color;
  digitalWrite(RGB_RED_PIN, LOW);
  digitalWrite(RGB_GREEN_PIN, LOW);
  digitalWrite(RGB_BLUE_PIN, LOW);
  
  if(color == "RED") digitalWrite(RGB_RED_PIN, HIGH);
  else if(color == "GREEN") digitalWrite(RGB_GREEN_PIN, HIGH);
  else if(color == "BLUE") digitalWrite(RGB_BLUE_PIN, HIGH);
  else if(color == "YELLOW") { digitalWrite(RGB_RED_PIN, HIGH); digitalWrite(RGB_GREEN_PIN, HIGH); }
  else if(color == "PURPLE") { digitalWrite(RGB_RED_PIN, HIGH); digitalWrite(RGB_BLUE_PIN, HIGH); }
  else if(color == "CYAN") { digitalWrite(RGB_GREEN_PIN, HIGH); digitalWrite(RGB_BLUE_PIN, HIGH); }
  else if(color == "WHITE") { digitalWrite(RGB_RED_PIN, HIGH); digitalWrite(RGB_GREEN_PIN, HIGH); digitalWrite(RGB_BLUE_PIN, HIGH); }
  
  Serial.println("LED: " + color);
}

// SERVO FUNCTIONS COMMENTED OUT
/*
int fanSpeed = 90;

void setFanSpeed(int speed) {
  if (speed >= 90 && speed <= 180) {
    fanSpeed = speed;
    fanServo.write(speed);
    Serial.println("Fan: " + String(speed));
  }
}

void stopFan() {
  setFanSpeed(90);
  Serial.println("Fan STOPPED");
}
*/

void executeCommand(String cmd) {
  Serial.println("Exec: " + cmd);
  
  if(cmd.startsWith("SET_LED_COLOR:")) setLedColor(cmd.substring(14));
  else if(cmd == "LED_OFF") setLedColor("OFF");
  // SERVO COMMANDS COMMENTED OUT
  // else if(cmd.startsWith("SET_FAN_SPEED:")) setFanSpeed(cmd.substring(14).toInt());
  // else if(cmd == "FAN_STOP") stopFan();
  else if(cmd.startsWith("WATER_PUMP:")) startPump(cmd.substring(11).toInt());
  else if(cmd == "STOP_WATER") stopPump();
  else if(cmd == "STATUS") Serial.println("LED:" + ledColor + " Pump:" + String(pumpRunning));
  else if(cmd == "SENSOR_READINGS") printSensorReadings();
  else Serial.println("Unknown cmd");
}

String getTelemetryJSON() {
  printSensorReadings();
  
  String json = "{";
  json += "\"deviceId\":\"" + deviceId + "\",";
  json += "\"soilMoisture\":" + String(readSoilMoisture()) + ",";
  json += "\"temperature\":" + String(dht.readTemperature()) + ",";
  json += "\"lightLevel\":" + String(readLightLevel()) + ",";
  json += "\"humidity\":" + String(dht.readHumidity()) + ",";
  json += "\"ledStatus\":\"" + ledColor + "\",";
  // json += "\"fanSpeed\":" + String(fanSpeed) + ",";  // COMMENTED OUT
  json += "\"pumpRunning\":" + String(pumpRunning ? "true" : "false") + ",";
  json += "\"timestamp\":" + String(millis());
  json += "}";
  return json;
}

// =============================================
// MQTT - OPTIMIZED CONNECTION
// =============================================
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message;
  message.reserve(length); // Pre-allocate memory
  for(int i = 0; i < length; i++) message += (char)payload[i];
  executeCommand(message);
}

bool connectMQTT() {
  Serial.print("MQTT connecting... ");
  
  // Set clean session for faster reconnection
  if (mqttClient.connect(deviceId.c_str(), MQTT_USER, MQTT_PASSWORD, 0, 1, 0, 0, true)) {
    Serial.println("SUCCESS!");
    
    // Subscribe to command topic
    if(mqttClient.subscribe(commandTopic.c_str(), 1)) {
      Serial.println("Subscribed to: " + commandTopic);
      reconnectAttempts = 0;
      return true;
    } else {
      Serial.println("Subscription failed!");
      return false;
    }
  } else {
    Serial.print("FAILED! State: ");
    Serial.println(mqttClient.state());
    reconnectAttempts++;
    return false;
  }
}

// =============================================
// MAIN
// =============================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n=== PLANTY SYSTEM STARTING ===");
  
  // Initialize hardware
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  
  // SERVO INITIALIZATION COMMENTED OUT
  /*
  ESP32PWM::allocateTimer(0);
  fanServo.setPeriodHertz(50);
  fanServo.attach(SERVO_PIN, 500, 2400);
  fanServo.write(90);
  */
  
  dht.begin();
  
  // WiFi connection with faster timeout
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setAutoReconnect(true);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
    Serial.print("Signal: "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  } else {
    Serial.println("\nWiFi FAILED! Restarting...");
    ESP.restart();
  }
  
  // MQTT Setup - OPTIMIZED
  wifiClient.setCACert(ROOT_CA);
  wifiClient.setTimeout(MQTT_SOCKET_TIMEOUT);
  
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(MQTT_KEEPALIVE);
  mqttClient.setSocketTimeout(MQTT_SOCKET_TIMEOUT);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  
  // Initial MQTT connection
  connectMQTT();
  
  Serial.println("\n=== SYSTEM READY ===");
  Serial.println("Device ID: " + deviceId);
  Serial.println("Telemetry: " + telemetryTopic);
  Serial.println("Commands: " + commandTopic);
  
  // Startup indicator
  setLedColor("GREEN");
  delay(1000);
  setLedColor("OFF");
}

void loop() {
  // Fast MQTT reconnection with backoff
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt >= RECONNECT_DELAY) {
      lastReconnectAttempt = now;
      
      if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        Serial.println("Max reconnect attempts reached. Restarting WiFi...");
        WiFi.disconnect();
        delay(100);
        WiFi.begin(ssid, password);
        reconnectAttempts = 0;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        connectMQTT();
      }
    }
  } else {
    mqttClient.loop();
  }
  
  // Update pump timer
  updatePump();

  // Send telemetry every 10 seconds
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 10000) {
    if (mqttClient.connected()) {
      String telemetry = getTelemetryJSON();
      if(mqttClient.publish(telemetryTopic.c_str(), telemetry.c_str(), false)) {
        Serial.println("✓ Telemetry sent");
      } else {
        Serial.println("✗ Telemetry failed");
      }
    }
    lastSend = millis();
  }

  // Serial commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    executeCommand(cmd);
  }

  delay(50); // Reduced from 100ms for faster response
}
