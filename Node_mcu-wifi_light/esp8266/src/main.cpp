#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// AP credentials
const char* ssid = "OMNIREMOTE";
const char* password = "12345678";
int lastWifiConnectAttempt = 0;
// Set your Static IP address
IPAddress local_IP(192, 168, 4, 10);
// Set your Gateway IP address
IPAddress gateway(192, 168, 4, 1);

IPAddress subnet(255, 255, 255, 0);


WiFiUDP udp; // UDP communication object
const unsigned int localPort = 7777; // Port to listen for data

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set to station mode
  if (!WiFi.config(local_IP, gateway, subnet)) {
  Serial.println("STA Failed to configure");
}
  WiFi.begin(ssid, password); // Connect to the ESP32 AP

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
  udp.begin(localPort); // Start listening on port 7777
  Serial.println(WiFi.localIP());
  pinMode(D7, OUTPUT); // Set IO2 (pin 2) as output
  digitalWrite(D7, LOW);


}

void loop() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char buffer[256];
    int len = udp.read(buffer, 255);
    if (len > 0) {
      buffer[len] = '\0';
      String command = String(buffer);
      // Process the command and control the LED accordingly
      digitalWrite(D7, command == "ON" ? HIGH : LOW);
      Serial.println("Received command: " + command);
    }
  }
    // Check WiFi connection and reconnect if necessary
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiConnectAttempt > 5000) {
    Serial.println("Connection lost, reconnecting...");
    WiFi.begin(ssid, password);
    lastWifiConnectAttempt = millis();
  }
}