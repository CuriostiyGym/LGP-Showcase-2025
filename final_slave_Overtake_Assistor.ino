//slave car harshini

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

void updateLEDs(bool localDetected, bool remoteDetected) {
  if (localDetected || remoteDetected) {
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

  bool localDetected = (digitalRead(IR_SENSOR_1) == LOW || digitalRead(IR_SENSOR_2) == LOW);
  updateLEDs(localDetected, remoteCarDetected);

  server.send(200, "text/plain", "OK");
  Serial.print("Received from Master: ");
  Serial.println(state);
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Master");

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
    http.begin(wifiClient, "http://192.168.4.1/update?state=" + state);
    int code = http.GET();
    http.end();

    Serial.print("Sent to Master: ");
    Serial.println(state);

    updateLEDs(currentIRState, remoteCarDetected);
  }
}