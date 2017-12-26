#include <SPI.h>
#include "RF24.h"

#include <avr/sleep.h>
#include <avr/power.h>

RF24 radio(9, 10); // CS, CS

// same address across all nodes, can be random, max 5 chars
byte address[6] = {"CjTS2"}; // Campusjaeger Toilet-Sensor 2

byte nodeId = 0; // should be unique

volatile bool sleep = false;

void setup() {
  pinMode(2, INPUT); // switch
  pinMode(6, OUTPUT); // led
  
  radio.begin();
  
  radio.setPALevel(RF24_PA_LOW); // adjust when necessary
  
  radio.openWritingPipe(address);
}

void loop() {
  digitalWrite(6, HIGH); // led on

  byte msg = nodeId * 2 + digitalRead(2); // read switch state
  
  radio.write(&msg, sizeof(byte));

  delay(100); // debouncing and time to see led

  digitalWrite(6, LOW); // led off
  enterSleep();
}

void enterSleep(void)
{
  sleep = true;
  radio.powerDown();
  
  attachInterrupt(digitalPinToInterrupt(2), isr, CHANGE); // interrupt, when pin 2 changes state
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // power down as much as possible
  cli(); // disable all interrupts
  if (sleep) // when interrupt already triggered do not sleep
  {
    sleep_enable();
    sei(); // sei() must be called directly before sleep_cpu() (to avoid an interrupt in between)
    sleep_cpu(); // sleep now
    sleep_disable();
  }
  sei(); // ensure enabled interrupts

  radio.powerUp();
}

void isr(void) {
  detachInterrupt(digitalPinToInterrupt(2)); // detach interrupt to avoid retrigger
  sleep = false; // ignore sleep call (used to prevent race condition)
}

