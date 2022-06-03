#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Encoder.h>

#include "button_handling.h"

#define PIN A1
#define PIN_ROTARY_A 5
#define PIN_ROTARY_B 6
#define BUTTON_PIN 9

#define SECOND 1000L // how many millis do a second have
#define LEDUPDATETIME 100L

#define LONG_PRESS_MILLIS 2000L

#define DEBUG

const int LED_COUNT = 35;

// enums
// enum buttonPressed { NoPress, ShortPress, LongPress };

// functions
void set_timer(long time);
uint32_t colorToUInt(uint8_t red, uint8_t green, uint8_t blue);
uint8_t UIntToColor(uint8_t rgbSelector, uint32_t color);
void writeLEDs();
void updateTimer();
unsigned long restartTimer();
void buttonHandling();
void checkPauseTime();

// objects
Adafruit_NeoPixel pixels(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
Encoder myEnc(PIN_ROTARY_A, PIN_ROTARY_B);

void setup() {
#ifdef DEBUG
  Serial.begin(9600);
  Serial.println("Hello World!");
#endif
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pixels.begin();
}

bool buttonPressed = false;

unsigned long endTime = 20;
const unsigned long TWENTY_MINUTES = SECOND * 60L * 20L;
const unsigned long FIVE_MINUTES = SECOND * 60L * 5L;

int32_t leds[LED_COUNT];

bool running = false;
bool working = false;
bool pause = false;
long oldRotaryPosition = -99;

void loop() {
  updateTimer();
  buttonHandling();
  checkPauseTime();
  // restartTimer();
  writeLEDs();
#ifdef DEBUG
  long newRotaryPosition = myEnc.read();
  if (newRotaryPosition != oldRotaryPosition) {
    oldRotaryPosition = newRotaryPosition;
    Serial.println(newRotaryPosition);
  }
#endif
}

void set_timer(long time) {
  uint8_t minutes = time / (SECOND * 60L);
  uint16_t overmillis = time % (SECOND * 60);
  uint8_t led_counter = 0;
  for (int ii = 0; ii < LED_COUNT; ii++) {
    leds[ii] = 0;
  }
  for (; led_counter < minutes and led_counter < LED_COUNT; led_counter++) {
    leds[led_counter] = colorToUInt(working ? 0 : 63, working ? 63 : 0, 0);
  }
  uint8_t colorScheme = 127L * overmillis / (60L * SECOND / 2L);
  if (led_counter < LED_COUNT) {
    leds[led_counter] =
        colorToUInt(working ? 0 : colorScheme, 0, working ? colorScheme : 0);
    led_counter++;
  }
  if (led_counter < LED_COUNT) {
    leds[led_counter] = 0;
  }
}

unsigned long nextchange = 0;
bool setLED = false;
void blinker(uint32_t time) {
  if (time == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if (millis() >= nextchange) {

    setLED = !setLED;
    nextchange = millis() + (setLED ? time : (time / 2.0));
    digitalWrite(LED_BUILTIN, setLED);
  }
}

uint32_t colorToUInt(uint8_t red, uint8_t green, uint8_t blue) {
  return (uint32_t(red) << 16) + (uint32_t(green) << 8) + blue;
}

uint8_t UIntToColor(uint8_t rgbSelector, uint32_t color) {
  return color >> (rgbSelector * 8) & 0b11111111;
}
unsigned long writeTime = 0;
void writeLEDs() {
  if (writeTime <= millis()) {
    pixels.clear();
    for (int ii = 0; ii < LED_COUNT; ii++) {
      pixels.setPixelColor(ii, pixels.Color(UIntToColor(2, leds[ii]),
                                            UIntToColor(1, leds[ii]),
                                            UIntToColor(0, leds[ii])));
    }

    if (pause && (writeTime >> 10) % 2) {
      pixels.clear();
    }
    pixels.show();
    writeTime += LEDUPDATETIME;
  }
}

void updateTimer() {
  long remainingTime;
  if (!running and pause) {
    remainingTime = restartTimer();
    running = true;
  } else if (!pause) {
    remainingTime = long(endTime - millis());
  }
  if (remainingTime > 0) {
    set_timer(remainingTime);
  } else {
    running = false;
    pause = true;
  }
}

unsigned long restartTimer() {
  if (!running) {
    working = !working;
    if (working) {
      endTime = millis() + TWENTY_MINUTES;
    } else {
      endTime = millis() + FIVE_MINUTES;
    }
    pause = true;
  }
  return endTime - millis();
}

void buttonHandling() {
  enum buttonPressed event = buttonEvaluation();
#ifdef DEBUG
  if (event != NoPress) {

    Serial.print(millis());
    Serial.print(" - ");
    Serial.println((event == LongPress) ? "long press released"
                                        : "short press released");
  }
#endif
  if (running) {
    switch (event) {

    case LongPress:
      pause = true;
      running = false;
      break;

    default:
      break;
    }
  }
  if (event == ShortPress) {
    pause = !pause;
    running = true;
  }
}

bool wasPaused = false;
unsigned long pauseTime = 0L;

void checkPauseTime() {
  if (wasPaused && !pause) {
    endTime += millis() - pauseTime;
  } else if (!wasPaused && pause) {
    pauseTime = millis();
  }
  wasPaused = pause;
}