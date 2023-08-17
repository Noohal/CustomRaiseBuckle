#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

/* --- Define Macros --- */
#define BTNS 2
#define ON LOW
#define OFF HIGH

/* --- Define Pin Numbers --- */
#define SW_0 2
#define SW_1 6

/* --- Define Button values --- */
uint8_t sw[] = {OFF, OFF};

void setup() {
  /* ------ Start Serial Communication ------ */
  Serial.begin(9600);

  /* ------ Add Pins ------ */
  pinMode(SW_0, INPUT_PULLUP);
  pinMode(SW_1, INPUT_PULLUP);
}

void loop() {
  sw[0] = digitalRead(SW_0);
  sw[1] = digitalRead(SW_1);
}
