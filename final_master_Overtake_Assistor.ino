//master car harshini

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define IR_SENSOR_1 D1
#define IR_SENSOR_2 D3
#define RED_LED D5
#define GREEN_LED D6

const char* ssid = "CG";
const char* password = "robotics1234";

ESP8266WebServer server(80);
WiFiClient wifiClient;

bool lastIRState = HIGH;
bool remoteCarDetected = false;

void updateLEDs(bool localDetected) {
  if (localDetected) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
  }
}

void handleUpdate() {
  String state = server.arg("state");
  remoteCarDetected = (state == "ON");

  // Only local detection affects master LEDs
  bool localDetected = (digitalRead(IR_SENSOR_1) == LOW || digitalRead(IR_SENSOR_2) == LOW);
  updateLEDs(localDetected);

  server.send(200, "text/plain", "OK");
  Serial.print("Received from Slave: ");
  Serial.println(state);
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);  // Start with green ON

  WiFi.softAP(ssid, password);
  delay(100);  // Wait for AP to start
  Serial.println("AP started at " + WiFi.softAPIP().toString());

  server.on("/update", handleUpdate);
  server.begin();
}

void loop() {
  server.handleClient();

  bool currentIRState = (digitalRead(IR_SENSOR_1) == LOW || digitalRead(IR_SENSOR_2) == LOW);
  if (currentIRState != lastIRState) {
    lastIRState = currentIRState;

    String state = currentIRState ? "ON" : "OFF";

    HTTPClient http;
    String url = "http://192.168.4.2/update?state=" + state;
    http.begin(wifiClient, url);
    int code = http.GET();
    http.end();

    Serial.print("Sent to Slave: ");
    Serial.println(state);

    updateLEDs(currentIRState);  // Only local matters
  }
}