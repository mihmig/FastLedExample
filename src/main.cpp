#include <Arduino.h>
#include <FastLED.h>
#include "GyverButton.h"
// #include <avr/sleep.h>

#define DATA_PIN    2
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    16

const CRGB colors[] = {CRGB::HTMLColorCode::Red, CRGB::HTMLColorCode::Yellow, CRGB::HTMLColorCode::Green, CRGB::HTMLColorCode::Blue};
#define NUM_COLORS sizeof(colors) / sizeof(colors[0];
#define PLAYER1_COLOR CRGB::HTMLColorCode::Red
#define PLAYER2_COLOR CRGB::HTMLColorCode::Blue

CRGB leds[NUM_LEDS]; // Состояния светодиодов
int8_t pos = 0; // Текущая позиция
int8_t pos1 = 0; // Текущая позиция игрока 1
int8_t pos2 = 8; // Текущая позиция игрока 2
int8_t currentColor = 1; // Текущий цвет
int8_t direction = 1; // Направление вращения

#define BRIGHTNESS          5 // Общая яркость 
#define FRAMES_PER_SECOND  20 // "Частота кадров"
#define DEBOUNCE 80  // Поправка на дребезг

GButton leftButton(8);
GButton middleButton(9);
GButton rightButton(10);

void fillLedsWithSampleColors() {
// leds[0] = CRGB(255, 0, 0);
  leds[0] = CRGB::HTMLColorCode::Red;
  leds[1] = CRGB::HTMLColorCode::Yellow;
  leds[2] =  CRGB::HTMLColorCode::Green;
  leds[3] = CRGB::HTMLColorCode::Blue;

  leds[4] = CRGB::HTMLColorCode::RosyBrown;
  leds[5] = CRGB::HTMLColorCode::LightGoldenrodYellow;
  leds[6] =  CRGB::HTMLColorCode::DarkOliveGreen;
  leds[7] = CRGB::HTMLColorCode::SlateBlue;

  leds[8] = CRGB::HTMLColorCode::DarkRed;
  leds[9] = CRGB::HTMLColorCode::LightYellow;
  leds[10] =  CRGB::HTMLColorCode::LightSeaGreen;
  leds[11] = CRGB::HTMLColorCode::CadetBlue;

  leds[12] = CRGB::HTMLColorCode::MediumVioletRed;
  leds[13] = CRGB::HTMLColorCode::GreenYellow;
  leds[14] =  CRGB::HTMLColorCode::MediumSpringGreen;
  leds[15] = CRGB::HTMLColorCode::AliceBlue;
}
void fillArray(CRGB color) {
for(byte i=0; i<NUM_LEDS; i++) {
      leds[i] = color;
    }
}

void setup() {
  
  leftButton.setTickMode(AUTO);
  leftButton.setDebounce(DEBOUNCE);
  middleButton.setTickMode(AUTO);
  middleButton.setDebounce(DEBOUNCE);
  rightButton.setTickMode(AUTO);
  rightButton.setDebounce(DEBOUNCE);

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  fillLedsWithSampleColors();
}

void loop()
{
  
  FastLED.show();
  while(!middleButton.isClick()) {
    delay(50);
  }
  delay(100);
  middleButton.resetStates();

  // Старт игры 
  pos1 = 0;
  pos2 = 8;
  while (true) {
    if (middleButton.isClick()) {
      return;
    }
    if (leftButton.isClick()) {
      pos1 = (pos1 + 1) % NUM_LEDS;
      if (pos1 == pos2) {
        fillArray(PLAYER1_COLOR);
        return;
      }
    }
    if (rightButton.isClick()) {
      pos2 = (pos2 + 1) % NUM_LEDS;
      if (pos2 == pos1) {
        fillArray(PLAYER2_COLOR);
        return;
      }
    }

    fillArray(CRGB::HTMLColorCode::Black);
    leds[pos1] = PLAYER1_COLOR;
    leds[pos2] = PLAYER2_COLOR;
    
    FastLED.show();
    //delay(50);
  }

  // sleep_enable();
  // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // sleep_cpu();

  // leds[pos] = colors[currentColor];
  //   FastLED.show();
  //   if (middleButton.isClick()) {
  //     currentColor++; 
  //     if (currentColor >= NUM_COLORS) {
  //       currentColor = 0;
  //     }
  //   }
  //   if (leftButton.isClick()) { 
  //     direction = -1;
  //   }
  //   if (rightButton.isClick()) {
  //     direction = 1;
  //   }
  //   delay(50);
  //   pos = pos + direction;
  //   if (pos >= NUM_LEDS) {
  //     pos = 0;
  //   } else if (pos < 0) {
  //     pos = NUM_LEDS;
  //   }
  
}