#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

/* --- Define Pin Numbers --- */
#define SW_0 2
#define BUSY 3
#define RX 4
#define TX 5
#define SW_1 6
#define SW_2 7

/* --- Define Macros --- */
#define BTNS 2
#define ON LOW
#define OFF HIGH

#define STATE_IDLE 1
#define STATE_KEY_PRESSED 2
#define STATE_ACTIVE_SOUND 3
#define STATE_RECORD_SCRATCH 4

/* --- Define Button Constants --- */
uint8_t OFF_OFF[] = {OFF, OFF};
uint8_t ON_OFF[] = {ON, OFF};
uint8_t OFF_ON[] = {OFF, ON};
uint8_t ON_ON[] = {ON, ON};

/* --- Define Button values --- */
uint8_t prev_sw[] = {OFF, OFF};
uint8_t sw[] = {OFF, OFF};

uint8_t prev_busy = OFF;
uint8_t busy = OFF;

uint8_t prev_state = STATE_IDLE;
uint8_t state = STATE_IDLE;
uint8_t last_sound_state = STATE_IDLE;

unsigned long sound_waiting_time = 0;

/* --- Define Sound variables --- */
SoftwareSerial ss(TX, RX);
DFRobotDFPlayerMini mp3;

const uint8_t KEY_SOUND = 1;
const uint8_t RECORD_SCRATCH = 2;

const uint8_t START_OFFSET = 2;
const uint8_t NUM_SOUNDS = 20;
uint8_t currentSound = START_OFFSET;

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
  /* ------ Add Pins ------ */
  //pinMode(SW_0, INPUT_PULLUP);
  pinMode(SW_1, INPUT_PULLUP);
  pinMode(SW_2, INPUT_PULLUP);
  pinMode(BUSY, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();
  sw[0] = digitalRead(SW_1);
  sw[1] = digitalRead(SW_2);
  
  Serial.println(state); Serial.print(sw[0]); Serial.println(sw[1]);

  switch(state) {
    case STATE_IDLE:
      if(memcmp(prev_sw, OFF_OFF, BTNS) == 0) {
        if(memcmp(sw, ON_OFF, BTNS) == 0) {
          state = STATE_RECORD_SCRATCH;
        } else if(memcmp(sw, OFF_ON, BTNS) == 0) {
          state = STATE_KEY_PRESSED;
          Serial.print("KEY PRESSED: ");
          Serial.print("PRESS ");
          mp3.play(KEY_SOUND);
        }
      }
      break;
    case STATE_KEY_PRESSED:
      if(memcmp(prev_sw, OFF_ON, BTNS) == 0) {
        if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          state = STATE_IDLE;
          unsigned long last = millis();
          unsigned long now_ms = millis();
          while(now_ms - last <= 800) {
            Serial.println(now_ms - last);
            now_ms = millis();
          }

          Serial.println("DONE");
          currentSound += 1;
          if(currentSound % (NUM_SOUNDS + START_OFFSET + 1) == 0) {
            currentSound = START_OFFSET + 1;
          }
          mp3.loop(currentSound);
          printDetail(mp3.readType(), mp3.read());
        }
      }
      break;
    case STATE_RECORD_SCRATCH:
      mp3.pause();
      mp3.play(RECORD_SCRATCH);
      Serial.println("Wakawaka");
      state = STATE_IDLE;
    default:
      break;
  }

  /*
  if(memcmp(prev_sw, OFF_OFF, BTNS) == 0) {
    if(memcmp(sw, ON_OFF, BTNS) == 0) {
      mp3.pause();
      currentSound += 1;
      if(currentSound % (NUM_SOUNDS + 1) == 0) 
        currentSound = 1;
      //Serial.print("Playing Sound: "); Serial.println(currentSound);
      mp3.loop(currentSound);
    }
  }
  */
  prev_sw[0] = sw[0];
  prev_sw[1] = sw[1];
  prev_state = state;
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}