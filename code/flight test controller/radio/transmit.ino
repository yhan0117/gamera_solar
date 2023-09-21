#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  int potValue = analogRead(A0);
  int PWM = map(potValue, 0, 1023, 0, 255);
  radio.write(&PWM, sizeof(PWM));
}
