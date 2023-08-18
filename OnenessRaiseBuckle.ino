#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"

#define SW_0 2
#define SW_1 6
#define RX 4
#define TX 5
#define BUSY 3

#define BTNS 2
#define ON LOW
#define OFF HIGH

#define RAISE_BUCKLE 1
#define SET 2
#define TRANSFORM_STANDBY 3
#define ACTIVATE 4
#define TRANSFORM 5
#define ATTACK 6
#define LOOP_1 7
#define LOOP_2 8

typedef enum State {
  STATE_IDLE,
  STATE_INSERT, // Set sound
  STATE_HENSHIN_RDY, // Standby
  STATE_HENSHIN_ACT, // Activation sound
  STATE_HENSHIN, // All as One, Geats Oneness
  STATE_FINISH_RDY,
  STATE_ATTACK_1_RDY, // Attack 1 Loop
  STATE_ATTACK_1, // Oneness Victory
} states;

State lastSoundState = STATE_IDLE;
uint8_t randomLoopTrackNum = 0;

SoftwareSerial ss(TX, RX);
DFRobotDFPlayerMini mp3;

const uint8_t OFF_OFF[] = {OFF, OFF};
const uint8_t ON_OFF[] = {ON, OFF};
const uint8_t OFF_ON[] = {OFF, ON};
const uint8_t ON_ON[] = {ON, ON};

State state = STATE_IDLE;
State prev_state = STATE_IDLE;
uint8_t prev_sw[] = {OFF, OFF};
uint8_t sw[] = {OFF, OFF};
uint8_t busy = OFF;
uint8_t prev_busy = ON;

unsigned long attack_1_hold_start = 0;
unsigned long attack_1_hold_end = 0;
bool attack_2_ready = false;

unsigned long begin_sound_time = 0;

bool playingSound;

void setup() {
  pinMode(SW_0, INPUT_PULLUP);
  pinMode(SW_1, INPUT_PULLUP);
  pinMode(BUSY, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BUSY), SetPlaying, CHANGE);

  Serial.begin(9600);
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
  mp3.setTimeOut(500); //Set serial communictaion time out 500ms
  mp3.volume(20);  //Set volume value (0~30).
}

unsigned long last = 0;

void loop() {
  unsigned long now = millis();

  sw[0] = digitalRead(SW_0);
  sw[1] = digitalRead(SW_1);
  //Serial.print(sw[0]); Serial.println(sw[1]);
  
  ChangeState(now);
  PlaySound();
  //PrintState();
  if(now - last >= 500) {
    //Serial.print(!sw[0]); Serial.print(!sw[1]); Serial.println(!busy);
    PrintState();
    last = now;
  }

  if(now - begin_sound_time >= 32000 && playingSound) {
    Reset();
    begin_sound_time = now;
  }

  prev_sw[0] = sw[0];
  prev_sw[1] = sw[1];
  prev_state = state;
  prev_busy = busy;
}

void ChangeState(unsigned long now) {
  switch(state) {
    case STATE_IDLE:
      if(memcmp(prev_sw, OFF_OFF, BTNS) == 0) {
        if(memcmp(sw, ON_OFF, BTNS) == 0) {
          state = STATE_INSERT;
        }
      }
      break;
    case STATE_INSERT:
      if(memcmp(prev_sw, ON_OFF, BTNS) == 0) {
        if(memcmp(sw, ON_OFF, BTNS) == 0) {
          state = STATE_HENSHIN_RDY;
        } else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
      }
      break;
    case STATE_HENSHIN_RDY:
      if(memcmp(prev_sw, ON_OFF, BTNS) == 0) {
        if(memcmp(sw, ON_ON, BTNS) == 0) {
          state = STATE_HENSHIN;
        } else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
      }
      break;
    case STATE_HENSHIN:
      if(memcmp(prev_sw, ON_ON, BTNS) == 0) {
        if(memcmp(sw, ON_OFF, BTNS) == 0) {
          state = STATE_FINISH_RDY;
        } else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
      }
    case STATE_FINISH_RDY:
      if(memcmp(prev_sw, ON_OFF, BTNS) == 0) {
        if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        } else if(memcmp(sw, ON_ON, BTNS) == 0) {
          state = STATE_ATTACK_1_RDY;
        }
        /*
        if(memcmp(sw, ON_OFF, BTNS) == 0) {
          if(prev_state == STATE_FINISH_RDY) {
            state = STATE_ATTACK_1_RDY;
          } else {
            state = STATE_FINISH_RDY;
            mp3.stop();
          }
        }else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
        
      */
      }
      break;
    case STATE_ATTACK_1_RDY:
      if(memcmp(prev_sw, ON_ON, BTNS) == 0) {
        if(memcmp(sw, ON_ON, BTNS) == 0) {
          state = STATE_ATTACK_1;
        } else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
      }
      break;
    case STATE_ATTACK_1:
      if(memcmp(prev_sw, ON_ON, BTNS) == 0) {
        if(memcmp(sw, ON_ON, BTNS) == 0) {
          state = STATE_FINISH_RDY;
          attack_2_ready = true;
          attack_1_hold_start = now;
        } else if(memcmp(sw, OFF_OFF, BTNS) == 0) {
          Reset();
        }
      }
      break;
    default:
      mp3.pause();
      break;
  }
}

void PlaySound() {
  switch(state) {
      case STATE_INSERT:
      if(prev_state == STATE_IDLE) {
        mp3.play(SET);
        lastSoundState = STATE_INSERT;
      }
      break;
    case STATE_HENSHIN_RDY:
      if(prev_state == STATE_INSERT) {
        //mp3.loop(TRANSFORM_STANDBY);
      }
      break;
    case STATE_HENSHIN:
      if(prev_state == STATE_HENSHIN_RDY) {
        mp3.play(ACTIVATE);
        lastSoundState = STATE_HENSHIN_ACT;
      }
      break;
    case STATE_ATTACK_1_RDY:
      if(prev_state == STATE_FINISH_RDY) {
        mp3.pause();
      }
      break;
    case STATE_ATTACK_1:
      if(prev_state == STATE_ATTACK_1_RDY) {
        mp3.pause();
        mp3.play(ACTIVATE);
        lastSoundState = STATE_ATTACK_1_RDY;
      }
      break;
    default:
      break;
  }
}

void SetPlaying() {
  Serial.println("BANG");
  busy = digitalRead(BUSY);
  if(prev_busy == HIGH && busy == LOW)
    return;
  //Serial.println(lastSoundState);

  if(busy) { // means it is not busy and can play sounds
    switch(lastSoundState) {
      case STATE_INSERT:
        randomLoopTrackNum = random(7,9);

        if(randomLoopTrackNum == 7)
          Serial.println(F("LOVE LIVE RANDOM LOOP"));
        else if(randomLoopTrackNum == 8)
          Serial.println(F("JELLYFISH JAM"));
        else
          ;

        mp3.loop(randomLoopTrackNum);
        lastSoundState = STATE_HENSHIN_RDY;
        begin_sound_time = millis();
        playingSound = true;
        break;
      case STATE_HENSHIN_ACT:
        //PrintInputState();
        Serial.println(F("All as One! Geats Oneness!"));
        mp3.play(TRANSFORM);
        lastSoundState = STATE_HENSHIN;
        playingSound = false;
        break;
      case STATE_ATTACK_1_RDY:
        //PrintInputState();
        Serial.println(F("Oneness Victory!"));
        mp3.play(ATTACK);
        lastSoundState = STATE_HENSHIN;
        playingSound = false;
        break;
      default:
        //Serial.println(F("idle"));
        break;
    }
  }

  prev_busy = busy;
}

void PrintState() {
  PrintInputState();
  switch(state) {
      case STATE_IDLE: Serial.println(F("STATE IDLE")); break;
      case STATE_INSERT: Serial.println(F("STATE INSERT")); break;
      case STATE_HENSHIN_RDY: Serial.println(F("STATE HENSHIN READY")); break;
      case STATE_HENSHIN: Serial.println(F("STATE HENSHIN")); break;
      case STATE_FINISH_RDY: Serial.println(F("STATE FINISH READY")); break;
      case STATE_ATTACK_1_RDY: Serial.println(F("STATE ATTACK 1 READY")); break;
      case STATE_ATTACK_1: Serial.println(F("STATE ATTACK 1")); break;
      default: Serial.println(F("Invalid")); break;  
  }
}

void PrintInputState() {
  Serial.print(!sw[0]);  Serial.print(F(" | "));
  Serial.print(!sw[1]);  Serial.print(F(" | "));
  Serial.print(!busy); Serial.print(F(" | "));
}

void Reset() {
  state = STATE_IDLE;
  lastSoundState = STATE_IDLE;
  mp3.pause();
  playingSound = false;
}