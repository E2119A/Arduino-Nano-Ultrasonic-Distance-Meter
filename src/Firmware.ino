// -------OPTIONS-------
// body length in centimeters, to transfer the reference point to the rear of the body
float case_offset = 10.0;


// sonar
#define ECHO 2
#define TRIG 3
#define sensVCC 4

// display
#define dispGND 5
byte DIO = 6;
byte RCLK = 7;
byte SCLK = 8;
#define dispVCC 9

// switcher
#define buttPIN 11
#define buttGND 12

// create distplay
#include <TM74HC595Display.h>
#include <TimerOne.h>
TM74HC595Display disp(SCLK, RCLK, DIO);
unsigned char SYM[47];

// sonar library 
#include <NewPing.h>
NewPing sonar(TRIG, ECHO, 400);

float dist_3[3] = {0.0, 0.0, 0.0};   // array for storing the last three measurements
float middle, dist, dist_filtered;
float k;
byte i, delta;
unsigned long dispIsrTimer, sensTimer;

void setup() {
  Serial.begin(9600);
  symbols();            // create symbols for display

  // pins setting
  pinMode(sensVCC, OUTPUT);
  pinMode(dispGND, OUTPUT);
  pinMode(dispVCC, OUTPUT);
  pinMode(buttPIN, INPUT_PULLUP);
  pinMode(buttGND, OUTPUT);

  // пpins signals
  digitalWrite(sensVCC, 1);
  digitalWrite(dispGND, 0);
  digitalWrite(dispVCC, 1);
  digitalWrite(buttGND, 0);
}

void loop() {
  if (millis() - sensTimer > 50) {                          // change and display every 50 ms
    // counter from 0 to 2
    // each iteration of timer i sequentially takes values ​​0, 1, 2, and so on in a circle
    if (i > 1) i = 0;
    else i++;

    dist_3[i] = (float)sonar.ping() / 57.5;                 // get the distance to the current cell of the array
    if (!digitalRead(buttPIN)) dist_3[i] += case_offset;    // if measurement side switch is on, add case_offset
    dist = middle_of_3(dist_3[0], dist_3[1], dist_3[2]);    // filter by median filter of the last 3 measurements

    delta = abs(dist_filtered - dist);                      // calculation of the change with the previous
    if (delta > 1) k = 0.7;                                 // if large - a sharp coefficient
    else k = 0.1;                                           // if small - smooth coefficient

    dist_filtered = dist * k + dist_filtered * (1 - k);     // "running average" filter

    disp.clear();                                           // clean display
    disp.float_dot(dist_filtered, 1);                       // display
    sensTimer = millis();                                   // reset timer
  }

  if (micros() - dispIsrTimer > 300) {       // timer of dynamic indication
    disp.timerIsr();                         // "kick" the display
    dispIsrTimer = micros();                 // reset timer
  }
}

// median filter of 3 values
float middle_of_3(float a, float b, float c) {
  if ((a <= b) && (a <= c)) {
    middle = (b <= c) ? b : c;
  }
  else {
    if ((b <= a) && (b <= c)) {
      middle = (a <= c) ? a : c;
    }
    else {
      middle = (a <= b) ? a : b;
    }
  }
  return middle;
}

// symbols for display
void symbols() {
  // обычные
  SYM[0] = 0xC0; //0
  SYM[1] = 0xF9; //1
  SYM[2] = 0xA4; //2
  SYM[3] = 0xB0; //3
  SYM[4] = 0x99; //4
  SYM[5] = 0x92; //5
  SYM[6] = 0x82; //6
  SYM[7] = 0xF8; //7
  SYM[8] = 0x80; //8
  SYM[9] = 0x90; //9

  // with point
  SYM[10] = 0b01000000; //0.
  SYM[11] = 0b01111001; //1.
  SYM[12] = 0b00100100; //2.
  SYM[13] = 0b00110000; //3.
  SYM[14] = 0b00011001; //4.
  SYM[15] = 0b00010010; //5.
  SYM[16] = 0b00000010; //6.
  SYM[17] = 0b01111000; //7.
  SYM[18] = 0b00000000; //8.
  SYM[19] = 0b00010000; //9.
}
