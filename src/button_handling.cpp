#define BUTTON_PIN 9
#define LONG_PRESS_MILLIS 2000L

#include "button_handling.h"
#include <Arduino.h>

enum buttonPressed buttonEvaluation() {
  const uint8_t retriggerSupressMillis = 2;
  static bool button_pressure;
  static unsigned long buttonTimer;
  static unsigned long retriggerSupress = 0;
  enum buttonPressed returnVal = NoPress;

  if (retriggerSupress < millis()) {
    if (!digitalRead(BUTTON_PIN) && !button_pressure) {
      buttonTimer = millis();
      button_pressure = true;
    }

    if (button_pressure && digitalRead(BUTTON_PIN)) {
      if ((buttonTimer + LONG_PRESS_MILLIS) < millis()) {
        returnVal = LongPress;
      } else {
        returnVal = ShortPress;
      }

      button_pressure = false;
      retriggerSupress = millis() + retriggerSupressMillis;
    }
  }
  return returnVal;
}