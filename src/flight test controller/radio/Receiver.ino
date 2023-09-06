#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if ( radio.available()) {
    while (radio.available()) {
      int PWM = 0;
      radio.read(&PWM, sizeof(PWM));
      Serial.println(PWM);
      analogWrite(3, PWM);
    }
  }
  delay(500);
}