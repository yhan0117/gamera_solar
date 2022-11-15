#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#define low 1000
#define high 2000

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

//struct storing all relevant data of an encoder-motor pair
typedef struct {
  //pins
  int A;
  int B;
  //rotation tracking
  int pos;
  //total power delivery
  int power;
  // debounce management
  boolean rotating;
  boolean A_set;
  boolean B_set;

  //interrupt timer
  volatile long curMicros;
} encoder;

Servo front;
Servo back;
Servo left;
Servo right;

//4 individual fine controllers
encoder control[4];
/*index and motor correspondence
  0: front
  1: back
  2: left
  3: right
*/

//enumerate encoder number
enum EncoderAssignments { f,
                          b,
                          l,
                          r };

//controls precision of each fine controller
int scalar = 10;
//master controller
int potVal;
int i;
long tt = 0;


void setup() {

  //LCD setup
  //initialize lcd screen
  // turn on the backlight
  lcd.init();

  lcd.backlight();


  //controls setup
  //set up each control
  //pin numbers
  /*
   L: 10,9
   R: 8,7
   F: 5,4
   B: 3,2
   */
  control[f].A = 46;
  control[f].B = 47;
  control[b].A = 48;
  control[b].B = 49;
  control[l].A = 50;
  control[l].B = 51;
  control[r].A = 52;
  control[r].B = 53;


  for (i = f; i <= r; i++) {

    //pin setup
    //no need for input pullup since pullup resistors are added physically
    pinMode(control[i].A, INPUT);
    pinMode(control[i].B, INPUT);

    //pin initialization
    digitalWrite(control[i].A, HIGH);
    digitalWrite(control[i].B, HIGH);

    //encoder initialization
    control[i].pos = 0;

    //debounce management
    control[i].A_set = false;
    control[i].B_set = false;
  }

  //initialize interrupt
  //need 8 fxns since interrupt routine cannot pass parameters
  attachInterrupt(digitalPinToInterrupt(control[f].A), fEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[f].B), fEncoderB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[b].A), bEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[b].B), bEncoderB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[l].A), lEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[l].B), lEncoderB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[r].A), rEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(control[r].B), rEncoderB, CHANGE);

  //motor setup
  //pin assignments
  front.attach(3);
  back.attach(6);
  left.attach(9);
  right.attach(10);

  //initialize each motor
  front.writeMicroseconds(low);
  back.writeMicroseconds(low);
  left.writeMicroseconds(low);
  right.writeMicroseconds(low);

  Serial.begin(9600);
}

void loop() {
  // reset the debouncer
  for (i = f; i <= r; i++) {
    control[i].rotating = true;    
  }

  //read in master controller value
  potVal = analogRead(A0);

  for (i = f; i <= r; i++) {
    control[i].power = potVal + scalar * control[i].pos;

    if (control[i].power < 0) {
      control[i].power = 0;
      control[i].pos = -potVal / scalar;
    }

    if (control[i].power > 1023) {
      control[i].power = 1023;
      control[i].pos = (1023 - potVal) / scalar;
    }

    control[i].power = map(control[i].power, 0, 1023, low, high);
  }
  
  //temporarilyt disable interrupts so motors do not stutter
  //interrupt during signal delivery to motors create issues
  noInterrupts();

  //output PWM power signal to motor
  front.writeMicroseconds(control[f].power);
  back.writeMicroseconds(control[b].power);
  left.writeMicroseconds(control[l].power);
  right.writeMicroseconds(control[r].power);

  //reenable interrupts once time sensitive signals are sent
  interrupts();

  //display output on lcd
  //last digit is omitted to save space on display
  lcd.setCursor(0, 0);
  lcd.print("f ");
  lcd.print(control[f].power/10);

  lcd.setCursor(8, 0);
  lcd.print("b ");
  lcd.print(control[b].power/10);

  lcd.setCursor(0, 1);
  lcd.print("l ");
  lcd.print(control[l].power/10);

  lcd.setCursor(8, 1);
  lcd.print("r ");
  lcd.print(control[r].power/10);
}

//front
//Interrupt on A changing state
void fEncoderA() {
  //set debounce timer
  control[f].curMicros = micros(); 

  // debounce
  if (control[f].rotating){
    do{
    }while(micros() - control[f].curMicros < 1000);
  }  
  // wait a little until the bouncing is done
  // do not use delay or millis. they dont work because the function class already includes interrupt and Duue does not support twice interrupt 
  
  // Test transition, did things really change?
  if (digitalRead(control[f].A) != control[f].A_set) {  // debounce once more
    control[f].A_set = !control[f].A_set;

    // adjust counter + if A leads B
    if (control[f].A_set && !control[f].B_set)
      control[f].pos++;

    control[f].rotating = false;  // no more debouncing until loop() hits again
  }
}

// Interrupt on B changing state
void fEncoderB() {

  control[f].curMicros = micros(); 

  if (control[f].rotating){
    do{
    }while(micros() - control[f].curMicros < 1000);
  }  

  if (digitalRead(control[f].B) != control[f].B_set) {
    control[f].B_set = !control[f].B_set;

    //  adjust counter - 1 if B leads A
    if (control[f].B_set && !control[f].A_set)
      control[f].pos--;

    control[f].rotating = false;
  }
}

//back
void bEncoderA() {

  control[b].curMicros = micros(); 

  if (control[b].rotating){
    do{
    }while(micros() - control[b].curMicros < 1000);
  }  

  if (digitalRead(control[b].A) != control[b].A_set) {
    control[b].A_set = !control[b].A_set;

    if (control[b].A_set && !control[b].B_set)
      control[b].pos++;

    control[b].rotating = false;
  }
}

void bEncoderB() {

  control[b].curMicros = micros(); 

  if (control[b].rotating){
    do{
    }while(micros() - control[b].curMicros < 1000);
  }  

  if (digitalRead(control[b].B) != control[b].B_set) {
    control[b].B_set = !control[b].B_set;

    if (control[b].B_set && !control[b].A_set)
      control[b].pos--;

    control[b].rotating = false;
  }
}

//left
void lEncoderA() {

  control[l].curMicros = micros(); 

  if (control[l].rotating){
    do{
    }while(micros() - control[l].curMicros < 1000);
  }  

  if (digitalRead(control[l].A) != control[l].A_set) {
    control[l].A_set = !control[l].A_set;

    if (control[l].A_set && !control[l].B_set)
      control[l].pos++;

    control[l].rotating = false;
  }
}

void lEncoderB() {

  control[l].curMicros = micros(); 

  if (control[l].rotating){
    do{
    }while(micros() - control[l].curMicros < 1000);
  }  

  if (digitalRead(control[l].B) != control[l].B_set) {
    control[l].B_set = !control[l].B_set;

    if (control[l].B_set && !control[l].A_set)
      control[l].pos--;

    control[l].rotating = false;
  }
}

//right
void rEncoderA() {

  control[r].curMicros = micros(); 

  if (control[r].rotating){
    do{
    }while(micros() - control[r].curMicros < 1000);
  }  

  if (digitalRead(control[r].A) != control[r].A_set) {
    control[r].A_set = !control[r].A_set;

    if (control[r].A_set && !control[r].B_set)
      control[r].pos++;

    control[r].rotating = false;
  }
}

void rEncoderB() {

  control[r].curMicros = micros(); 

  if (control[r].rotating){
    do{
    }while(micros() - control[r].curMicros < 1000);
  }  

  if (digitalRead(control[r].B) != control[r].B_set) {
    control[r].B_set = !control[r].B_set;

    if (control[r].B_set && !control[r].A_set)
      control[r].pos--;

    control[r].rotating = false;
  }
}
 
