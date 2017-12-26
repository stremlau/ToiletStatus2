#include <SPI.h>
#include "RF24.h"

RF24 radio(4, 15); // CS, CS

// same address across all nodes, can be random, max 5 chars
byte address[6] = {"CjTS2"}; // Campusjaeger Toilet-Sensor 2

void setup() {
  Serial.begin(9600);
  radio.begin();

  radio.setPALevel(RF24_PA_LOW);
  
  radio.openReadingPipe(1, address);
  radio.startListening();
}

void loop() {
  byte msg;
  
  if (radio.available()) {
    while (radio.available()) {
      radio.read(&msg, sizeof(byte));
    }

    byte open = msg % 2;
    byte nodeId = msg / 2;

    Serial.print("Node ");
    Serial.print(nodeId);
    Serial.print(": ");
    Serial.println(open);
  }
}

