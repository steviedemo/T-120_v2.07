#ifndef __FUNCTION_PROTOTYPES_H__
#define __FUNCTION_PROTOTYPES_H__
void scaleDepth();

void fsPressed();
void fsReleased();
void fsHeld();
void fsHoldReleased();
void setEffectState(bool, bool);
void checkFootswitch(const uint32_t);
void turnRed();
void turnYellow();
void turnOff();
void setOutput(bool);

#endif
