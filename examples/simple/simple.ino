#include <WS2812_MSP432.h>

#define NUM_LEDS 8

WS2812_MSP432 strip(NUM_LEDS);

void setup() {
  strip.begin();
  strip.clear();
}

void loop() {
  // Sweep red across the strip one pixel at a time
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();
    strip.setPixelColor(i, 150, 0, 0);
    strip.show();
    delay(100);
  }
}
