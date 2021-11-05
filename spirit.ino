#include <Arduino.h>
#include <Wire.h>
#include <radio.h>
#include <TEA5767.h>


/// ------------------------ RADIO ------------------------
#define LOW_STATION 8800
#define NUM_STATIONS 220
#define MAX_DELAY 200
#define DELAY_INTERVAL 10

TEA5767 radio;
unsigned int stationCount = 0;
unsigned long lastCycle = 0;
/// --------------------------------------------------------


/// ------------------- ROTARY ENCODER ---------------------
#define ROT_CLK 5
#define ROT_DT 3
#define ROT_SW 4
#define MAX_MODE 5
#define LED_PIN 6

volatile int mode = 0;
volatile int delayMs = 20;
volatile unsigned long lastInterrupt = 0;

int currentStateCLK, currentStateDT;
int lastStateCLK;
int lastStateSW = 1;
/// --------------------------------------------------------


void setup() {
  pinMode(ROT_CLK,INPUT);
  pinMode(ROT_DT,INPUT);
  pinMode(ROT_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ROT_DT), changeSpeed, CHANGE);

  for (int i = 0; i <= MAX_MODE; i++) {
    pinMode(LED_PIN + i, OUTPUT);
  }

  lastStateCLK = digitalRead(ROT_CLK);
  
  delay(200);

  radio.init();
  radio.setBandFrequency(RADIO_BAND_FM, LOW_STATION);
  radio.setVolume(2);
  radio.setMono(false);
}

unsigned int zakCount = 0;
char zakbagans[] = "ZAKBAGANS";
int zak() {
  zakCount++;
  if (zakCount >= sizeof(zakbagans)) {
    zakCount = 0;
  }

  return zakbagans[zakCount];
}

unsigned int primeCount = 0;
int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97 };
int prime() {
  primeCount++;
  if (primeCount >= sizeof(primes)) {
    primeCount = 0;
  }

  return primes[primeCount];
}

int nextFreq() {
  switch(mode){
    // MODE 0: Forward
    case 0:
      return stationCount + 1;
    
    // MODE 1: Backward
    case 1:
      return stationCount - 1;

    // MODE 2: Random
    case 2:
      return random(0, NUM_STATIONS);
  
    // MODE 3: Prime
    case 3:
      return prime();
  
    // MODE 4: ZAK BAGANS
    case 4:
      return stationCount + zak();

    // MODE 5: 666
    case 5:
      return stationCount + 6;
  }
}


void changeSpeed() {
  cli();
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterrupt < 100){
    sei();
    return;
  }
  
  currentStateCLK = digitalRead(ROT_CLK);
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

    currentStateDT = digitalRead(ROT_DT);
    if (currentStateDT != currentStateCLK) {
      delayMs += 10;
    } else {
      delayMs -= 10;
    }

    if (delayMs > 500) {
      delayMs = 500;
    }
    if (delayMs < 0) {
      delayMs = 0;
    }
  }
  
  lastStateCLK = currentStateCLK;
  sei();
}


void displayMode() {
  for (int i = 0; i <= MAX_MODE; i++) {
    digitalWrite(LED_PIN + i, LOW);
  }

  digitalWrite(LED_PIN + mode, HIGH);
}

void cycleFreq() {
  stationCount = nextFreq() % NUM_STATIONS;
  lastCycle = millis();
  if (stationCount > NUM_STATIONS) {
    stationCount = 0;
  }

  radio.setMute(true);
  delay(delayMs * 0.5);

  radio.setFrequency(LOW_STATION + stationCount*10);

  radio.setMute(false);
  delay(delayMs * 0.5);
}

void checkModeSwitch() {
  unsigned long currTime = millis();
  int stateSW = digitalRead(ROT_SW);
  if (stateSW == LOW && currTime - lastInterrupt > 250 && lastStateSW) {
    mode++;
    lastInterrupt = currTime;
    if (mode > MAX_MODE) {
      mode = 0;
    }
    delay(50);
  }
  lastStateSW = stateSW;
}


void loop() {
  checkModeSwitch();  
  
  cycleFreq();

  displayMode();
}
