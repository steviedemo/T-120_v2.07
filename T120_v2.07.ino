
#include <EEPROM.h>
#include <Potentiometer.h>
#include <TinyLFO.h>
#include <Footswitch.h>
#include "definitions.h"
#include "functionPrototypes.h"
uint32_t last_lfo_update(0), last_pot_update(0);
uint8_t rate_val(0), glide_val(0), rand_val(0), depth_val(0);
bool effect_active(false), glide_changed(false), 
     rand_changed(false), configMode(false);
// Footswitch
uint32_t debounce_start(0), lockout_begin(0);
bool press_key(false), hold_key(false), lockout_active(false);
uint8_t depth_max(255), scaled_depth(255);
#define LED_INTERVAL 1000

void scaleDepth();
Potentiometer ratePot(SPEED_IN, 2, 10),
              depthPot(DEPTH_IN, 2, 10);
TinyLFO       lfo;
Footswitch    fs(FS_IN);
void setup() {
  delay(1000);
  TCCR1A = _BV(COM1A1) | _BV(WGM10);
  TCCR1B = _BV(CS10) | _BV(WGM12);
  pinMode(LFO_OUT, OUTPUT);
  pinMode(FS_IN, INPUT);
  pinMode(A3, INPUT);
  randomSeed(analogRead(A3));
  pinMode(A3, OUTPUT);
  // set mux control and r/y pins of led as outputs.
  DDRB |= (1<<PB0) | (1<<PB1) | (1<<PB2);
  initialize();
}

void initialize(){
  rand_val = 127;
  if (EEPROM.read(VALIDATION_ADDR) == VALIDATION_VAL){
    rand_val = EEPROM.read(RAND_ADDR);
  } 
  glide_val = EEPROM.read(GLIDE_ADDR);
  ratePot.readPot();
  rate_val = ratePot.get8Bit();
  depth_max = map(rate_val, 0, 255, DEPTH_MAX, DEPTH_MIN);
  depthPot.readPot();
  depth_val = depthPot.get8Bit();
  scaleDepth();
  lfo.setRate(rate_val);
  lfo.setRand(rand_val);
  lfo.setGlide(glide_val);
  fs.attachCallbacks(fsPressed, fsReleased, fsHeld, fsHoldReleased);
  fs.setActiveHigh();
  effect_active = false;
  configMode = false;
  glide_changed = false;
  rand_changed = false;
  turnOff();
  setOutput(false);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint32_t curr = millis();
  fs.check();
  if (curr - last_pot_update > POT_UPDATE_INTERVAL) {
    last_pot_update = curr;
    readPots();
  }
  if (curr - last_lfo_update > LFO_UPDATE_INTERVAL) {
    last_lfo_update = curr;
    uint8_t lfoVal = lfo.update();
    uint8_t scaled = map(lfoVal, 0, 255, 0, scaled_depth);
    analogWrite(LFO_OUT, scaled);
  }  
}

void readPots() {
  if (ratePot.readPot()) {
    uint8_t val = ratePot.get8Bit();
    if (configMode) { // Holding Footswitch enables alt rand function
      rand_val = val;
      lfo.setRand(val);
      rand_changed = true;
    } else {
      rate_val = val;
      lfo.setRate(rate_val);
      if (rate_val > 150){
        depth_max = DEPTH_MIN;
      } else {
        depth_max = map(rate_val, 0, 150, DEPTH_MAX, DEPTH_MIN);
      }
      scaleDepth();
    }
  }
  if (depthPot.readPot()) {
    uint8_t val = depthPot.get8Bit();
    if (configMode) { // Holding footswitch enables alt GLIDE function
      glide_val = val;
      lfo.setGlide(glide_val);
      glide_changed = true;
    } else {
      depth_val = map(val, 0, 255, 0, depth_max);
      scaleDepth();
    }
  }
}

void scaleDepth(){
  scaled_depth = map(depth_val, 0, 255, 0, depth_max);
}
