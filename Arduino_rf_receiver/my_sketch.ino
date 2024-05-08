#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(9600);
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  Serial.println("start ");
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(6,OUTPUT);

  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);

  Serial.println("success ");


}

void loop() {
  if (mySwitch.available()) {
    
    Serial.println("Received ");
    Serial.println( mySwitch.getReceivedValue() );
    int paramValue = mySwitch.getReceivedValue();
    if(paramValue == 1) {
      digitalWrite(4, LOW);
      Serial.println("3 HIGH");

    }
    else if(paramValue == 2) {
      digitalWrite(4, HIGH);
      Serial.println("3 HIGH");

    }
    else if(paramValue == 3) {
      digitalWrite(5, LOW);
      Serial.println("4 HIGH");

    }
    else if(paramValue == 4) {
      digitalWrite(5, HIGH);
      Serial.println("4 HIGH");

    }
    else if(paramValue == 5) {
      digitalWrite(6, LOW);
      Serial.println("5 HIGH");

    }
    else if(paramValue == 6) {
      digitalWrite(6, HIGH);
      Serial.println("5 HIGH");

    }
    else if(paramValue == 7) {
      digitalWrite(7, LOW);
      Serial.println("7 HIGH");

    }
    else if(paramValue == 8) {
      digitalWrite(7, HIGH);
      Serial.println("7 HIGH");

    }
    else {
          Serial.println("no match found");
    }
    Serial.println(" / ");
    Serial.println( mySwitch.getReceivedBitlength() );
    Serial.println("bit ");
    Serial.println("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );

    mySwitch.resetAvailable();
  }
}