#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

/* --- Define Macros --- */
#define BTNS 2
#define ON LOW
#define OFF HIGH

/* --- Define Pin Numbers --- */
#define SW_0 2
#define RX 4
#define TX 5
#define SW_1 6

/* --- Define Button values --- */
uint8_t prev_sw[] = {OFF, OFF};
uint8_t sw[] = {OFF, OFF};

/* --- Define Sound variables --- */
SoftwareSerial ss(TX, RX);
DFRobotDFPlayerMini mp3;

void setup() {
  /* ------ Start Serial Communication ------ */
  Serial.begin(9600);

  /* ------ Setup MP3 Player Module ------*/
  ss.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!mp3.begin(ss, true, false)) {  //Use serial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));

  mp3.stop();
  mp3.volume(20);
  mp3.play(1);

  /* ------ Add Pins ------ */
  pinMode(SW_0, INPUT_PULLUP);
  pinMode(SW_1, INPUT_PULLUP);
}

unsigned long last = 0;

void loop() {
  unsigned long now = millis();
  sw[0] = digitalRead(SW_0);
  sw[1] = digitalRead(SW_1);


  prev_sw[0] = sw[0];
  prev_sw[1] = sw[1];
}
