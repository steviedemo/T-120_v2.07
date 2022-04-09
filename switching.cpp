#include <Arduino.h>
#include <EEPROM.h>
#include "functionPrototypes.h"
#include "definitions.h"
#include <Relay.h>
extern bool press_key, hold_key, effect_active, configMode, 
			rand_changed, glide_changed, lockout_active;
extern Relay wet_ctl;
extern uint32_t debounce_start, lockout_begin;
extern uint8_t glide_val, rand_val;

void fsPressed(){
  if (!effect_active){
    setEffectState(true, false);
  }
}

void fsReleased(){
  setEffectState(!effect_active, true);
}

void fsHeld(){
  if (effect_active){
    configMode = true;
    turnYellow();
  }
}


void fsHoldReleased() {
  if (!effect_active) {
    setEffectState(false, true);
  } else {
    configMode = false;
    bool eepromWritten(false);
    if (glide_changed) {
      EEPROM.write(GLIDE_ADDR, glide_val);
      glide_changed = false;
      eepromWritten = true;
    }
    if (rand_changed) {
      EEPROM.write(RAND_ADDR, rand_val);
      rand_changed = false;
      eepromWritten = true;
    }
    setEffectState(true, true);

  }
}

void setEffectState(bool engaged, bool saveState) {
  if (saveState){
    effect_active = engaged;
  }
  if (engaged){
    setOutput(true);
    turnRed();
  } else {
    setOutput(false);
    turnOff();
  }
}

void setOutput(bool state){
	if (state){
		PORTB &= ~(1<<PB0);
	} else {
		PORTB |= (1<<PB0);
	}
}

void turnRed(){
	PORTB &= ~(1<<PB2);
	PORTB |= (1<<PB1);
}
void turnYellow(){
	PORTB &= ~(1<<PB1);
	PORTB |= (1<<PB2);
}
void turnOff(){
	PORTB &= ~((1<<PB1) | (1<<PB2));
}


void checkFootswitch(const uint32_t curr_time) {
  if (lockout_active){
    if ((curr_time - lockout_begin) > LOCKOUT_PERIOD){
      lockout_active = false;
      lockout_begin = 0;
    }
  } else { 
    if (digitalRead(FS_IN) == HIGH) {
      if (debounce_start == 0) {
        debounce_start = curr_time;
      } else if ((curr_time - debounce_start) > PRESS_INTERVAL) {
        if ((curr_time - debounce_start) < HOLD_INTERVAL) { /// PRESS ACTION VALIDATED
          if (!press_key) {
            // set state variables
            fsPressed();
          } // if (!eng_key)
        } else if ((curr_time - debounce_start) > HOLD_INTERVAL){  /// HOLD ACTION VALIDATED
          if (!hold_key) {
            hold_key = true;
            fsHeld();
          }
        }
      } // end else if
    } else if (digitalRead(FS_IN) == LOW) {  // FOOTSWITCH OPEN
      debounce_start = 0;
      if (press_key) {
        fsReleased();
        press_key = false;
        hold_key = false;
        lockout_active = true;
        lockout_begin = curr_time;
      } // end if (eng_key)
    } // end else (Footswitch release)
  } // lockout
}
