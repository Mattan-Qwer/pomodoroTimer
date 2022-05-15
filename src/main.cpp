#include <Arduino.h>

#include <Adafruit_NeoPixel.h>
#include <Encoder.h>

#define PIN A1
#define PIN_ROTARY_A 5
#define PIN_ROTARY_B 6

#define SECOND 1000L // how many millis do a second have
#define LEDUPDATETIME 100L

#define DEBUG

const int LED_COUNT = 35;

// functions
void set_timer(long time);
uint32_t colorToUInt(uint8_t red, uint8_t green, uint8_t blue);
uint8_t UIntToColor(uint8_t rgbSelector, uint32_t color);
void writeLEDs();

// objects
Adafruit_NeoPixel pixels(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);
Encoder myEnc(PIN_ROTARY_A, PIN_ROTARY_B);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello World!");
  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
}

unsigned long endTime = 20;
const unsigned long TWENTY_MINUTES = SECOND * 60L * 20L;
const unsigned long FIVE_MINUTES = SECOND * 60L * 5L;

int32_t leds[LED_COUNT];

bool running = false;
bool working = false;
long oldPosition = -99;

void loop() {
  if (!running) {
    working = !working;
    if (working) {
      endTime = millis() + TWENTY_MINUTES;
    } else {
      endTime = millis() + FIVE_MINUTES;
    }
    running = true;

  } else {
    long remainingTime = long(endTime - millis());
    if (remainingTime > 0) {
      set_timer(remainingTime);
    } else {
      running = false;
    }
  }
  writeLEDs();
#ifdef DEBUG
  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
  }
#endif
}

void set_timer(long time) {
  uint8_t minutes = time / (SECOND * 60L);
  uint16_t overmillis = time % (SECOND * 60);
  uint8_t led_counter = 0;

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
#ifdef DEBUG

  // Serial.println(255L * overmillis / (60L * SECOND));
#endif
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
    for (int ii = 0; ii < LED_COUNT; ii++) {
      pixels.setPixelColor(ii, pixels.Color(UIntToColor(2, leds[ii]),
                                            UIntToColor(1, leds[ii]),
                                            UIntToColor(0, leds[ii])));
    }
    pixels.show();
    writeTime += LEDUPDATETIME;
  }
}