#include <WiFiManager.h> 
#include <Adafruit_GFX.h>
#include <Firebase_ESP_Client.h>
#include <IRremote.hpp>
#include <RCSwitch.h>

#include <WebServer.h>
#include <WiFiUdp.h>
#include <vector> // Include this header for using string and vector
#include <map>
//senosr libs
#include <Adafruit_SSD1306.h>
#include <BH1750.h>
#include "DHT.h"
#include <FS.h>
#include <SPIFFS.h>

RCSwitch mySwitch = RCSwitch();

WebServer server(80); // HTTP server on port 80
WiFiUDP udp; // UDP communication object

// ESP8266 IP address
IPAddress esp8266IP(192, 168, 4, 10); // Replace with the actual IP of ESP8266
const unsigned int outPort = 7777; // Port to send data to ESP8266

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert Firebase project API Key
#define API_KEY ""
#define DATABASE_URL "" 

const int irReceiverPin = 17;
bool autolight = false;
bool autocooling = false;
String irDataString = "";
#define SENSORS

#ifdef SENSORS
  //sensor defines and pins 
  #define WIRE Wire
  #define DHTTYPE DHT11   // DHT 11
  #define DHTPIN 18

  Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &WIRE);
  BH1750 lightMeter;
  DHT dht(DHTPIN, DHTTYPE);
#endif

// select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN 13
int timeout = 120; // seconds to run for

//Define Firebase Data object and other variables
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//wifi ap credential 
const char* apSsid = "OMNIREMOTE";
const char* apPassword = "12345678";

//global variables
String state = "";
String key = "";
bool pair = false;
#define USE_SERIAL Serial

// Define a vector to store IR data strings
std::vector<String> irDataVector;

// Define a variable to store the timestamp of the last received IR data
unsigned long lastIrDataTimestamp = 0;
unsigned long loopstamp = 0;
unsigned long currloopstamp = 0;
std::map<String, std::vector<String>> irDataMap;


#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

unsigned long previousMillis = 0;
const unsigned long scrollingInterval = 50;  // Adjust this value to control the scrolling speed
int textStart = 0;
String currentText = "";
    int16_t textX, textY;
  uint16_t textWidth, textHeight;
void scrollText(String text) {

  currentText = text;
  display.getTextBounds(currentText, 0, 0, &textX, &textY, &textWidth, &textHeight);
  textStart = display.width();
}

void updateScrollingText() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= scrollingInterval) {
    previousMillis = currentMillis;

    display.clearDisplay();
    display.setCursor(textStart, 10);

    // Print the text from the current start position
    display.println(currentText);

    // Move the start position for the next loop
    textStart -= 4;  // Adjust the scrolling speed here

    if (textStart < (textWidth * -1)) {
      textStart = display.width();
    }

    display.display();
  }
}


// Function to parse the IR data string and transmit
void parseAndTransmitIRData(String irDataString) {
  // Extract protocol, address, command, and raw data
    int firstSpaceIndex = irDataString.indexOf(" ");
  int secondSpaceIndex = irDataString.indexOf(" ", firstSpaceIndex + 1);
    String protocol = irDataString.substring(0, firstSpaceIndex);
    String address = irDataString.substring(firstSpaceIndex + 1, secondSpaceIndex);
    String command = irDataString.substring(secondSpaceIndex + 1);
  Serial.println(protocol + " " + address +  "  "+ command + "  ");
  // Convert hex strings to unsigned long
  scrollText("Transmitting IR data : " + protocol + " " + address +  "  "+ command + "  ");

  unsigned long addressValue = strtoul(address.c_str(), NULL, 16);
  unsigned long commandValue = strtoul(command.c_str(), NULL, 16);
  unsigned long numberOfRepeats = 0;

if (protocol.toInt() == decode_type_t::NEC) {
  IrSender.sendNEC(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::SAMSUNG) {
  IrSender.sendSamsung(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::SAMSUNG48) {
  IrSender.sendSamsung48(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::SAMSUNGLG) {
  IrSender.sendSamsungLG(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::SONY) {
  IrSender.sendSony(addressValue, commandValue, numberOfRepeats, IrReceiver.decodedIRData.numberOfBits);
} else if (protocol.toInt() == decode_type_t::PANASONIC) {
  IrSender.sendPanasonic(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::DENON) {
  IrSender.sendDenon(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::SHARP) {
  IrSender.sendSharp(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::LG) {
  IrSender.sendLG(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::JVC) {
  IrSender.sendJVC((uint8_t)addressValue, (uint8_t)commandValue, numberOfRepeats); // casts are required to specify the right function
} else if (protocol.toInt() == decode_type_t::RC5) {
  IrSender.sendRC5(addressValue, commandValue, numberOfRepeats, 0); // No toggle for repeats
} else if (protocol.toInt() == decode_type_t::RC6) {
  IrSender.sendRC6(addressValue, commandValue, numberOfRepeats, 0); // No toggle for repeats
} else if (protocol.toInt() == decode_type_t::KASEIKYO_JVC) {
  IrSender.sendKaseikyo_JVC(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::KASEIKYO_DENON) {
  IrSender.sendKaseikyo_Denon(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::KASEIKYO_SHARP) {
  IrSender.sendKaseikyo_Sharp(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::KASEIKYO_MITSUBISHI) {
  IrSender.sendKaseikyo_Mitsubishi(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::NEC2) {
  IrSender.sendNEC2(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::ONKYO) {
  IrSender.sendOnkyo(addressValue, commandValue, numberOfRepeats);
} else if (protocol.toInt() == decode_type_t::APPLE) {
  IrSender.sendApple(addressValue, commandValue, numberOfRepeats);
}

  // Add more else-if conditions for other protocols
}

bool saveMapToFile(const std::map<String, std::vector<String>>& irDataMap, const String& fileName) {
  File file = SPIFFS.open("/" + fileName, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }

  for (const auto& pair : irDataMap) {
    file.print(pair.first);
    file.print(":");
    for (const auto& value : pair.second) {
      file.print(value);
      file.print(",");
    }
    file.println();
  }

  file.close();
  return true;
}

bool loadMapFromFile(std::map<String, std::vector<String>>& irDataMap, const String& fileName) {
  File file = SPIFFS.open("/" + fileName, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return false;
  }

  irDataMap.clear();
  String line;
  while (file.available()) {
    line = file.readStringUntil('\n');
    int colonIndex = line.indexOf(':');
    if (colonIndex != -1) {
      String key = line.substring(0, colonIndex);
      String values = line.substring(colonIndex + 1);
      std::vector<String> valueVector;
      int startIndex = 0;
      int commaIndex = values.indexOf(',');
      while (commaIndex != -1) {
        valueVector.push_back(values.substring(startIndex, commaIndex));
        startIndex = commaIndex + 1;
        commaIndex = values.indexOf(',', startIndex);
      }
      valueVector.push_back(values.substring(startIndex));
      irDataMap[key] = valueVector;
    }
  }

  file.close();
  return true;
}


void irReceiverTask(void *parameter) {
  unsigned long currentTime = 0;
  unsigned long pairTimeout = 0; // Variable to keep track of the pair timeout

  for (;;) {
    currentTime = millis();
    if (pair) {
      scrollText("Pairing IR KEY : " + key);
      if (pairTimeout == 0) {
        pairTimeout = currentTime + 5000; // Set the pair timeout to 5 seconds from now
      }

      if (currentTime < pairTimeout) {
        decode_results results;
        if (IrReceiver.decode()) {
          IrReceiver.printIRResultShort(&Serial);
          IrReceiver.resume();
          Serial.println(IrReceiver.decodedIRData.decodedRawData);
          if (IrReceiver.decodedIRData.decodedRawData != 0) {
              // Check if the key exists in the map
              auto it = irDataMap.find(key);
              if (it != irDataMap.end()) {
                  // Remove the key-value pair from the map
                  irDataMap.erase(it);
                  Serial.println("Removed key: " + key);
              } else {
                  Serial.println("Key not found in the map.");
              }
            // Create a string with IR data
            String irDataString = String(IrReceiver.decodedIRData.protocol) + " " + String(IrReceiver.decodedIRData.address, HEX) + " " + String(IrReceiver.decodedIRData.command, HEX);
            Serial.println(irDataString);
            Serial.println(IrReceiver.decodedIRData.decodedRawData);

            // Check if the key already exists in the map
            if (irDataMap.find(key) != irDataMap.end() && !(irDataString == "0 0 0")) {
              // Key exists, check if the value already exists in the vector
              std::vector<String>& valueVector = irDataMap[key];
              bool alreadyExists = false;
              for (const auto& existingData : valueVector) {
                if (irDataString == existingData) {
                  alreadyExists = true;
                  break;
                }
              }

              // If the value doesn't exist, add it to the vector
              if (!alreadyExists) {
                valueVector.push_back(irDataString);
              }
            } else {
              // Key doesn't exist, create a new vector and add the new value
              std::vector<String> newVector;
              newVector.push_back(irDataString);
              irDataMap[key] = newVector;
            }
          }
        }
      } else {
        pair = false;
        server.send(201, "text/plain", "OK");
        Serial.println("Pair Value : " + String(pair));
        pairTimeout = 0; // Reset the pair timeout

        // Print the contents of the irDataMap
        if (!irDataMap.empty()) {
          Serial.println("IR Data Map:");
          for (const auto& pair : irDataMap) {
            Serial.print("Key: ");
            Serial.println(pair.first);
            Serial.print("Values: ");
            for (const auto& value : pair.second) {
              Serial.print(value);
              scrollText("Pairing completed with key : " + pair.first + " with value : " + value);
              Serial.print(", ");
            }
            Serial.println();
          }
        } else {
          Serial.println("IR Data Map is empty");
        }
        if (saveMapToFile(irDataMap, "irdata.txt")) {
          Serial.println("Map data saved to SPIFFS");
        } else {
          Serial.println("Failed to save map data to SPIFFS");
        }
      }
    }
    updateScrollingText();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Non-blocking delay
  }

}


// Function to handle LED control requests
void handleLEDControl() {
  if (server.hasArg("state")) {
    scrollText("Controlling wifi devices with state " + server.arg("state"));
    String state = server.arg("state");
    udp.beginPacket(esp8266IP, outPort);
    udp.print(state);
    udp.endPacket();
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle LED control requests
void handleProjector() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);
    paramName = paramName+"proj";

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    if(paramValue == "pair") {
      scrollText("Button pressed for projector ");
      key = paramName;
      pair = true;
      Serial.println("Pairing....... ");
    } else {
      key = "";
      Serial.print("Pressing value: ");
      if (irDataMap.find(key) != irDataMap.end()) {
          const std::vector<String>& values = irDataMap[paramName];
          for (const String& value : values) {
              Serial.println(value);
              parseAndTransmitIRData(value);
              delay(100);
          }
          Serial.println(); // Print a newline after the loop
      } else {
          Serial.println("Key not found in the map.");
      }
  }
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle LED control requests
void handleTv() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);
    paramName = paramName+"tv";
    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    if(paramValue == "pair") {
      scrollText("Button pressed for projector ");
      key = paramName;
      pair = true;
      Serial.println("Pairing....... ");
    } else {
      key = "";
      Serial.print("Pressing value: ");
      if (irDataMap.find(paramName) != irDataMap.end()) {
          const std::vector<String>& values = irDataMap[paramName];
          for (const String& value : values) {
              Serial.println(value);
              parseAndTransmitIRData(value);
              delay(100);
          }
          Serial.println(); // Print a newline after the loop
      } else {
          Serial.println("Key not found in the map.");
      }
  }
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle LED control requests
void handleAc() {
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);
    paramName = paramName+"ac";

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    if(paramValue == "pair") {
      scrollText("Button pressed for projector ");
      key = paramName;
      pair = true;
      Serial.println("Pairing....... ");
      currentText = "Pairing started";
    } else {
      currentText = "Pressing remote";

      key = "";
      Serial.print("Pressing value: ");
      if (irDataMap.find(paramName) != irDataMap.end()) {
          const std::vector<String>& values = irDataMap[paramName];
          for (const String& value : values) {
              Serial.println(value);
              parseAndTransmitIRData(value);
              delay(100);
          }
          Serial.println(); // Print a newline after the loop
      } else {
          Serial.println("Key not found in the map.");
      }
  }
  }
  server.send(200, "text/plain", "OK");
}

// Function to handle LED control requests
void handleRf() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    scrollText("Controlling RF devices with state : "+paramValue);
    if(paramValue == "LEDON") {
        mySwitch.send(1, 24);
        Serial.print("send value: LEDON");

    }
    else if(paramValue == "LEDOFF") {
        mySwitch.send(2, 24);
    }
    else if(paramValue == "FANON") {
      mySwitch.send(3, 24);
    }
    else if(paramValue == "FANOFF") {
      mySwitch.send(4, 24);

    }
    else if(paramValue == "TUBEON") {
      mySwitch.send(5, 24);      
    }
    else if(paramValue == "TUBEOFF") {
      mySwitch.send(6, 24);
    }

  server.send(200, "text/plain", "OK");
  } 
}

// Function to handle LED control requests
void handleAutocooling() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    if(paramValue == "ON") {
        Serial.print("auto cooling on ");
        autocooling = true;
        scrollText("Auto cooling is turned ON  ");

    }
    else if(paramValue == "OFF") {
        Serial.print("auto cooling on ");
        mySwitch.send(4, 24);
        delay(50);
        mySwitch.send(4, 24);
        autocooling = false;
        scrollText("Auto cooling is turned OFF"  );

    }
  server.send(200, "text/plain", "OK");
  } 
}

// Function to handle LED control requests
void handleAutolight() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    if(paramValue == "ON") {
        Serial.print("auto light on ");
        autolight  = true;
        scrollText("Auto Light is turned ON   ");

    }
    else if(paramValue == "OFF") {
        Serial.print("auto light on ");
        mySwitch.send(2, 24);
        delay(50);
        mySwitch.send(2, 24);
        autolight = false;
        scrollText("Auto Light is turned OFF   ");

    }
  server.send(200, "text/plain", "OK");
  } 
}

// Function to handle LED control requests
void handleBuzzer() {
   // Get the URL of the request
  int argCount = server.args();
  // Iterate over each argument in the request
  for (int i = 0; i < argCount; i++) {
    // Get the name of the parameter at index 'i'
    String paramName = server.argName(i);

    // Get the value of the parameter with the name 'paramName'
    String paramValue = server.arg(paramName);

    // Print the name and value of the parameter
    Serial.print("Parameter name: ");
    Serial.println(paramName);
    Serial.print("Parameter value: ");
    Serial.println(paramValue);
    scrollText("Remote buzzer is Ringing ");
    if(paramValue == "ring") {
      for(int i = 0; i < 10; i++ ) {
      digitalWrite(23,HIGH);
      delay(50);
      digitalWrite(23,LOW);
      delay(50);
      }
    }
  server.send(200, "text/plain", "OK");
  } 
}

void firebaseInit() {
  
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  // Firebase.reconnectWiFi(true);  
  delay(2000);

}

#ifdef SENSORS
  void sensorInits() {
    Wire.begin();
    dht.begin();
    lightMeter.begin();
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
    display.setTextWrap(false);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

  }
#endif


void setup() {
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
}
  USE_SERIAL.setDebugOutput(true);
  mySwitch.enableTransmit(16);
  pinMode(23,OUTPUT);
  #ifdef SENSORS
    sensorInits();
  #endif
    WiFiManager wm;
	// wm.resetSettings();
	bool res = wm.autoConnect(apSsid,apPassword); // password protected ap
	pinMode(TRIGGER_PIN, INPUT_PULLUP);
	if(!res) {
	Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
		firebaseInit();
    }
  #ifdef SENSORS
  currentText = "Ip ready : 192.168.4.1";
  delay(1000);
  #endif
  IrReceiver.begin(irReceiverPin , ENABLE_LED_FEEDBACK);
  IrSender.begin(19); 
  disableLEDFeedback(); // Disable feedback LED at default feedback LED pin

  WiFi.persistent(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSsid, apPassword);
  Serial.println(WiFi.softAPIP());
    // Start Web Server.
  server.on("/led", handleLEDControl); // Handle LED control requests
  server.on("/projector", handleProjector); // Handle sensor data requests
  server.on("/ir", handleTv); // Handle configuration page requests
  server.on("/ac", handleAc); // Handle configuration page requests
  server.on("/rf", handleRf); // Handle configuration page requests
  server.on("/buzzer", handleBuzzer); // Handle configuration page requests
  server.on("/autolight", handleAutolight); // Handle configuration page requests
  server.on("/autocooling", handleAutocooling); // Handle configuration page requests

  server.begin(); // Start HTTP server
  xTaskCreatePinnedToCore(irReceiverTask, "irReceiverTask", 10000, NULL, 1, NULL, 0);
  digitalWrite(23,HIGH);
  delay(1000);
  digitalWrite(23,LOW);
  if (loadMapFromFile(irDataMap, "irdata.txt")) {
    Serial.println("Map data loaded from SPIFFS");
  } else {
    Serial.println("Failed to load map data from SPIFFS");
  }

}

void loop() {
  server.handleClient(); // Handle incoming HTTP requests
      static int a = 1;
      float t;
      float lux;
      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
        if( a % 2 == 0) {
          currentText = "";
        }
        a++;
        sendDataPrevMillis = millis();
        t = dht.readTemperature();
        float h = dht.readHumidity();
        lux = lightMeter.readLightLevel();
        #ifdef CLOUD
          if (Firebase.RTDB.setFloat(&fbdo, "omniremote/Temperature/celcius", t )){
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }
          count++;
          
          if (Firebase.RTDB.setFloat(&fbdo, "omniremote/Humidity/percentage", h)){
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
          }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
          }
          if (Firebase.RTDB.setFloat(&fbdo, "omniremote/Intensity/percentage", lux)){
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
          }
        #endif
        if(lux < 20 && autolight) {
            Serial.println("Auto light on.......");
            mySwitch.send(1, 24);
            delay(50);
            mySwitch.send(1, 24);
        }else if(lux > 20 && autolight) {
            Serial.println("Auto light on.......");
            mySwitch.send(2, 24);
            delay(50);
            mySwitch.send(2, 24);
        }
        
      }
      if(t > 37 && autocooling)
        {
            Serial.println(t);
            Serial.println("Auto fan on.......");
            mySwitch.send(3, 24);
            delay(50);
            mySwitch.send(3, 24);

        }
        else if(t < 34 && autocooling)
        {
            Serial.println("Auto fan off.......");

            mySwitch.send(4, 24);
            delay(50);
            mySwitch.send(4, 24);

        }

  }



