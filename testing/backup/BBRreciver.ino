#include <SPI.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24.h>
#include <RF24_config.h>
#include <Servo.h>

Servo motor;

RF24 radio(6, 7); // CE, CSN
const byte address[6] = "00001";

void setup() {
  motor.attach(4);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  motor.writeMicroseconds(1000);

}

void loop() {
  delay(10);
  radio.startListening();
  if (radio.available()) {
    while (radio.available()) {
      int power = 0;
      radio.read(&power, sizeof(int));
      motor.writeMicroseconds(power);
    }
  }
}
