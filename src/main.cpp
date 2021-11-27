#include <Arduino.h>
#include <FastLED.h>
#include "GyverButton.h" // https://alexgyver.ru/gyverbutton/
#include <avr/sleep.h>

#define DATA_PIN    PD2
#define LEFT_BUTTON PD5
#define MIDDLE_BUTTON PD4
#define RIGHT_BUTTON PD3

#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define LED_COUNT    16

const CRGB colors[] = {CRGB::HTMLColorCode::Red, CRGB::HTMLColorCode::Yellow, CRGB::HTMLColorCode::Green, CRGB::HTMLColorCode::Blue};
#define NUM_COLORS sizeof(colors) / sizeof(colors[0];
#define PLAYER1_COLOR CRGB::HTMLColorCode::Red
#define PLAYER2_COLOR CRGB::HTMLColorCode::Blue

CRGB leds[LED_COUNT]; // Состояния светодиодов
int8_t player1Position = 0; // Текущая позиция игрока 1
int8_t player1Length = 1; // Текущая длина игрока 1
int8_t player2Position = 8; // Текущая позиция игрока 2
int8_t player2Length = 1; // Текущая длина игрока 1

#define BRIGHTNESS          5 // Общая яркость ленты
#define FRAMES_PER_SECOND  20 // "Частота кадров"
#define DEBOUNCE 30  // Поправка на дребезг
#define PLAYER1_WIN 1
#define PLAYER2_WIN 2
#define MAX_PLAYER_LENGTH 4 // Максимальная длина игрока
#define MODE_GARLAND 0 // Режим работы - гирлянда
#define MODE_GAME 1 // Режим работы - игра

GButton leftButton(LEFT_BUTTON);
GButton middleButton(MIDDLE_BUTTON);
GButton rightButton(RIGHT_BUTTON);

byte mode = MODE_GARLAND;
// TODO добавить watchdog
// При включении по умолчанию - гирлянда, на сколько хватит памяти. Кнопки - смена режима
// Если зажата средняя кнопка - игра

// Различные режимы работы гирлянды (взято из WS2812_FX-master.zip)
int garlandMode = 3; // Текущий режим работы
#define GARLAND_MODE_COUNT 8 // Общее количество режимов работы
int thisdelay = 40;          //-FX LOOPS DELAY VAR
int thisstep = 10;           //-FX LOOPS DELAY VAR
int thishue = 0;             //-FX LOOPS DELAY VAR
int thissat = 255;           //-FX LOOPS DELAY VAR
int thisindex = 0;
int thisRED = 0;
int thisGRN = 0;
int thisBLU = 0;
int idex = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue = 0;                //-HUE (0-255)
int ibright = 0;             //-BRIGHTNESS (0-255)5
int isat = 0;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
int lcount = 0;              //-ANOTHER COUNTING VAR
int BOTTOM_INDEX = 0;        // светодиод начала отсчёта
int TOP_INDEX = int(LED_COUNT / 2);
int EVENODD = LED_COUNT % 2;
int ledsX[LED_COUNT][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)

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
for(byte i=0; i<LED_COUNT; i++) {
      leds[i] = color;
    }
}
// Победа одного игрока - мигаем его цветом, ожидая нажатия средней кнопки
void finalWin(int playerNum) {
  CRGB color = playerNum == 1 ? PLAYER1_COLOR : PLAYER2_COLOR;
  while(!middleButton.isPress()) {
    fillArray(color);
    FastLED.show();
    delay(500);
    fillArray(CRGB::HTMLColorCode::Black);
    FastLED.show();
    delay(500);
  }
  fillLedsWithSampleColors();
  return;
}
// Один игрок догнал другого
int catched(int playerNum) {
  if (playerNum == 1) {
    player1Length++;
    if (player1Length > MAX_PLAYER_LENGTH) {
      return PLAYER1_WIN;
    }
    player1Position = player2Position;
    player2Position = (player1Position + (LED_COUNT / 2)) % LED_COUNT; // Ставим проигравшего игрока напротив
  }
  if (playerNum == 2) {
    player2Length++;
    if (player2Length > MAX_PLAYER_LENGTH) {
      return PLAYER2_WIN;
    }
    player2Position = player1Position;
    player1Position = (player2Position + (LED_COUNT / 2)) % LED_COUNT; // Ставим проигравшего игрока напротив
  }
  return 0;
}

void setup() {
  Serial.begin(19200);
  Serial.println(F("reset"));
  Serial.println(F("leftButton setTickMode"));
  leftButton.setTickMode(AUTO);
  // leftButton.setDebounce(DEBOUNCE); // Антидребезг отключаем, т.к.
  Serial.println(F("middleButton setTickMode"));
  middleButton.setTickMode(AUTO);
  // middleButton.setDebounce(DEBOUNCE);
  Serial.println(F("rightButton setTickMode"));
  rightButton.setTickMode(AUTO);
  // rightButton.setDebounce(DEBOUNCE);
  Serial.println(F("start"));
  FastLED.setBrightness(BRIGHTNESS);
  // tell FastLED about the LED strip configuration
  Serial.println(F("addLeds"));
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  //fillLedsWithSampleColors();

  // Если при включении зажата средняя кнопка - включаем игру, иначе - гирлянда
  if (!digitalRead(MIDDLE_BUTTON)) {
    mode = MODE_GAME;
  } else {
    mode = MODE_GARLAND;
  }
Serial.println(F("setup end"));
}

//---FIND INDEX OF ANTIPODAL OPPOSITE LED
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {
    iN = ( i + TOP_INDEX ) % LED_COUNT;
  }
  return iN;
}
void copy_led_array() {
  for (int i = 0; i < LED_COUNT; i++ ) {
    ledsX[i][0] = leds[i].r;
    ledsX[i][1] = leds[i].g;
    ledsX[i][2] = leds[i].b;
  }
}
void one_color_all(int cred, int cgrn, int cblu) {       //-SET ALL LEDS TO ONE COLOR
  for (int i = 0 ; i < LED_COUNT; i++ ) {
    leds[i].setRGB( cred, cgrn, cblu);
  }
}
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == BOTTOM_INDEX) {
    return BOTTOM_INDEX;
  }
  if (i == TOP_INDEX && EVENODD == 1) {
    return TOP_INDEX + 1;
  }
  if (i == TOP_INDEX && EVENODD == 0) {
    return TOP_INDEX;
  }
  return LED_COUNT - i;
}
//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i) {
  int r;
  if (i > 0) {
    r = i - 1;
  }
  else {
    r = LED_COUNT - 1;
  }
  return r;
}
void rainbow_fade() {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
  ihue++;
  if (ihue > 255) {
    ihue = 0;
  }
  for (int idex = 0 ; idex < LED_COUNT; idex++ ) {
    leds[idex] = CHSV(ihue, thissat, 255);
  }
  LEDS.show();
  delay(thisdelay);
}
void rainbow_loop() {                        //-m3-LOOP HSV RAINBOW
  idex++;
  ihue = ihue + thisstep;
  if (idex >= LED_COUNT) {
    idex = 0;
  }
  if (ihue > 255) {
    ihue = 0;
  }
  leds[idex] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay(thisdelay);
}
void ems_lightsONE() {                    //-m7-EMERGENCY LIGHTS (TWO COLOR SINGLE LED)
  idex++;
  if (idex >= LED_COUNT) {
    idex = 0;
  }
  int idexR = idex;
  int idexB = antipodal_index(idexR);
  int thathue = (thishue + 160) % 255;
  for (int i = 0; i < LED_COUNT; i++ ) {
    if (i == idexR) {
      leds[i] = CHSV(thishue, thissat, 255);
    }
    else if (i == idexB) {
      leds[i] = CHSV(thathue, thissat, 255);
    }
    else {
      leds[i] = CHSV(0, 0, 0);
    }
  }
  LEDS.show();
  delay(thisdelay);
}
void ems_lightsALL() {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
  idex++;
  if (idex >= LED_COUNT) {
    idex = 0;
  }
  int idexR = idex;
  int idexB = antipodal_index(idexR);
  int thathue = (thishue + 160) % 255;
  leds[idexR] = CHSV(thishue, thissat, 255);
  leds[idexB] = CHSV(thathue, thissat, 255);
  LEDS.show();
  delay(thisdelay);
}
void random_march() {                   //-m14-RANDOM MARCH CCW
  copy_led_array();
  int iCCW;
  leds[0] = CHSV(random(0, 255), 255, 255);
  for (int idex = 1; idex < LED_COUNT ; idex++ ) {
    iCCW = adjacent_ccw(idex);
    leds[idex].r = ledsX[iCCW][0];
    leds[idex].g = ledsX[iCCW][1];
    leds[idex].b = ledsX[iCCW][2];
  }
  LEDS.show();
  delay(thisdelay);
}
void rainbow_vertical() {                        //-m23-RAINBOW 'UP' THE LOOP
  idex++;
  if (idex > TOP_INDEX) {
    idex = 0;
  }
  ihue = ihue + thisstep;
  if (ihue > 255) {
    ihue = 0;
  }
  int idexA = idex;
  int idexB = horizontal_index(idexA);
  leds[idexA] = CHSV(ihue, thissat, 255);
  leds[idexB] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay(thisdelay);
}
void random_color_pop() {                         //-m25-RANDOM COLOR POP
  idex = random(0, LED_COUNT);
  ihue = random(0, 255);
  one_color_all(0, 0, 0);
  leds[idex] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay(thisdelay);
}
void rgb_propeller() {                           //-m27-RGB PROPELLER
  idex++;
  int ghue = (thishue + 80) % 255;
  int bhue = (thishue + 160) % 255;
  int N3  = int(LED_COUNT / 3);
  //int N6  = int(LED_COUNT / 6);
  int N12 = int(LED_COUNT / 12);
  for (int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + LED_COUNT - N12) % LED_COUNT;
    int j1 = (j0 + N3) % LED_COUNT;
    int j2 = (j1 + N3) % LED_COUNT;
    leds[j0] = CHSV(thishue, thissat, 255);
    leds[j1] = CHSV(ghue, thissat, 255);
    leds[j2] = CHSV(bhue, thissat, 255);
  }
  LEDS.show();
  delay(thisdelay);
}


void loop()
{
  leftButton.tick();
  rightButton.tick();
  middleButton.tick();
  if (mode == MODE_GARLAND) {
    
    if (leftButton.isClick()) { // previous mode
      garlandMode = garlandMode == 1 ? GARLAND_MODE_COUNT : garlandMode-1;
    }
    if (rightButton.isClick()) { //next mode
      garlandMode = garlandMode == GARLAND_MODE_COUNT ? 1 : garlandMode+1;
    }
    switch (garlandMode) {
      case 1: rainbow_fade(); break;            // плавная смена цветов всей ленты
      case 2: rainbow_loop(); break;            // крутящаяся радуга
      case 3: ems_lightsONE(); break;           // вращаются красный и синий
      case 4: ems_lightsALL(); break;           // вращается половина красных и половина синих
      case 5: random_march(); break;            // безумие случайных цветов
      case 6: rainbow_vertical(); break;        // радуга в вертикаьной плоскости (кольцо)
      case 7: random_color_pop(); break;        // безумие случайных вспышек
      case 8: rgb_propeller(); break;           // RGB пропеллер
    }
    return;
  }
  fillLedsWithSampleColors();
  FastLED.show();
  while(!middleButton.isClick()) {
    delay(50);
    // sleep_enable();
    // set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    // sleep_cpu();
  }
  delay(100);
  middleButton.resetStates();

  // Старт игры 
  player1Position = 0;
  player1Length = 1;
  player2Position = 8;
  player2Length = 1;
  while (true) {
    if (middleButton.isClick()) { // Досрочный выход из игры
      return;
    }
    if (leftButton.isClick()) {
      player1Position = (player1Position + 1) % LED_COUNT;
      if (player1Position == ((player2Position + LED_COUNT - player2Length + 1) % LED_COUNT) ){
        if (catched(1) > 0) {
          finalWin(1);
          return;
        };
        
      }
    }
    if (rightButton.isClick()) {
      player2Position = (player2Position + 1) % LED_COUNT;
      if (player2Position == ((player1Position + LED_COUNT - player1Length + 1) % LED_COUNT)) {
        if (catched(2) > 0) {
          finalWin(2);
          return;
        };
      }
    }

    fillArray(CRGB::HTMLColorCode::Black);
    for(int i=0; i<player1Length;i++) {
      leds[(player1Position - i + 16) % LED_COUNT] = PLAYER1_COLOR;
      
    }
    for(int i=0; i<player2Length;i++) {
      leds[(player2Position - i + 16) % LED_COUNT] = PLAYER2_COLOR;
    }
    FastLED.show();
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
  //   if (pos >= LED_COUNT) {
  //     pos = 0;
  //   } else if (pos < 0) {
  //     pos = LED_COUNT;
  //   }
  
}