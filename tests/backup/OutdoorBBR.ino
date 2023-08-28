#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Servo.h>
#define low 1000
#define high 2000

LiquidCrystal_I2C lcd(0x27, 16, 2);

//pins
int A;
int B;

//rotation tracking
int encoderPos;
int lastReportedPos;

// debounce management
boolean rotating = false;    
boolean A_set;
boolean B_set;

Servo motor;

//controls precision of each fine controller
int scalar = 10;
int potVal;

int tt = 0;
volatile long curMicros = 0;

void setup() {
  
  //LCD setup
  //initialize lcd screen
  lcd.init();
  // turn on the backlight
  lcd.backlight();

  //pin assignment
  A = 46;
  B = 47;
    
    //pin setup
    //no need for input pullup since pullup resistors are added physically 
  pinMode(A,INPUT_PULLUP);
  pinMode(B,INPUT_PULLUP);

  //pin initialization
  digitalWrite(A, HIGH);
  digitalWrite(B, HIGH);

  //encoder initialization
  encoderPos = 0;
  lastReportedPos = 1;   // change management

  //debounce management
  A_set = false;
  B_set = false;

  //initialize interrupt
  attachInterrupt(digitalPinToInterrupt(A), EncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(B), EncoderB, CHANGE);

  //motor setup
  //pin assignments
  motor.attach(2);

  //initialize each motor
  motor.write(0);

  Serial.begin(9600);
}

void loop() {
  rotating = true;  // reset the debouncer

  potVal = analogRead(A0);
  int power = potVal + scalar * encoderPos;

  if (power < 0) {
    power = 0;
    encoderPos = -potVal / scalar;
  }

  if (power > 1023) {
    power = 1023;
    encoderPos = (1023 - potVal) / scalar;
  }

  power = map(power, 0, 1023, low, high);

  lcd.setCursor(0, 0);
  lcd.print(power/10); 
  
  if (millis() - tt > 500){
    Serial.println(power/10);
    tt = millis();
  }
  noInterrupts();
  motor.write(power);
  interrupts();
}

// Interrupt on A changing state
void EncoderA() {
  // debounce
  curMicros = micros();
  
  if (rotating){
    do{
    }while(micros() - curMicros < 1000);
  }


  // Test transition, did things really change?
  if ( digitalRead(A) != A_set ) { // debounce once more
    A_set = !A_set;

    // adjust counter + if A leads B
    if ( A_set && !B_set )
      encoderPos += 1;

    rotating = false;  // no more debouncing until loop() hits again
  }

  if (lastReportedPos != encoderPos) {
    lastReportedPos = encoderPos;
  }
}

// Interrupt on B changing state, same as A above
void EncoderB() {

  curMicros = micros();
  
  if (rotating){
    do{
    }while(micros() - curMicros < 1000);
  }

  if ( digitalRead(B) != B_set ) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
    if ( B_set && !A_set )
      encoderPos -= 1;

    rotating = false;
  }

  if (lastReportedPos != encoderPos) {
    lastReportedPos = encoderPos;
  }
}
