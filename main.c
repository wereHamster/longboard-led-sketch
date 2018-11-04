#include "FastLED.h"
#include <Bounce2.h>

#define NUM_LEDS 72

#define BUTTON_PIN 17
#define INDICATOR_LED_PIN 18
#define LIGHT_SENSOR_PIN 21

CRGB ledsL[NUM_LEDS];
CRGB ledsR[NUM_LEDS];

Bounce debouncer = Bounce();



static uint16_t x;
static uint16_t y;
static uint16_t z;

void setup() {
  Serial.begin(57600);
  Serial.println("setup: begin");

  // LED strips
  LEDS.setBrightness(255);
  LEDS.addLeds<APA102, 14, 19, BGR>(ledsR, NUM_LEDS);
  LEDS.addLeds<APA102, 16, 15, BGR>(ledsL, NUM_LEDS);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  debouncer.attach(BUTTON_PIN);
  debouncer.interval(5); // interval in ms

  // Indicator LED
  pinMode(INDICATOR_LED_PIN, OUTPUT);

  // Read the button value a few times to let it stabilize.
  for (int i = 0; i < 10; ++i) {
    debouncer.update();
    delay(2);
  }

  // Flash the LEDs to indicate setup is done.
  whiteflash(5);

  Serial.println("setup: done");

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();
}

void whiteflash(int nFlashes) {
  for (int j = 0; j < nFlashes; ++j) {
    for (int i = 0; i < NUM_LEDS; i++) {
      ledsL[i] = CHSV(0, 0, 50);
      ledsR[i] = CHSV(0, 0, 50);
    }
  
    FastLED.show();
    delay(50);  
  
    resetall();
    FastLED.show();

    delay(50);
  }
}

void fadeall(uint8_t scale) {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledsL[i].nscale8(scale);
    ledsR[i].nscale8(scale);
  }
}

void resetall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledsL[i] = CRGB::Black;
    ledsR[i] = CRGB::Black;
  }
}


void flash() {
  uint8_t hue = 0; // Red

  resetall();
    
  for (int i = 0; i < 10; i++) {
    ledsL[i] = CHSV(hue, 0, 100);
    ledsR[i] = CHSV(hue, 0, 100);
  }

  FastLED.show();
  delay(500);

  resetall();
  FastLED.show();
}

int strideOffset = 0;
void accelerateStep() {
  uint8_t hue = 85; // Green

  fadeall(200);

  int nStrides = 2;
  int strideWidth = NUM_LEDS / nStrides;
  
  for (int i = 0; i < nStrides; i++) {
    ledsL[NUM_LEDS - (i * strideWidth + strideOffset) - 1] = CHSV(hue, 255, 255);
    ledsR[NUM_LEDS - (i * strideWidth + strideOffset) - 1] = CHSV(hue, 255, 255);
  }

  strideOffset++;
  if (strideOffset == strideWidth) {
    strideOffset = 0;
  }
}

int noticeValue = 0;
int noticeDiff = 1;

void noticeStep() {
  for (int i = 0; i < NUM_LEDS; i++) {
    ledsL[i] = CHSV(140, 255, noticeValue);
    ledsR[i] = CHSV(140, 255, noticeValue);
  }
  
  noticeValue = noticeValue + noticeDiff;
  if (noticeDiff > 0 && noticeValue >= 200) {
    noticeDiff = -1;
  } else if (noticeDiff < 0 && noticeValue < 50) {
    noticeDiff = 1;
  }
}

void accelerate() {
  uint8_t hue = 85; // Green

  uint8_t strideWidth = NUM_LEDS / 2;
  for (int i = 0; i < strideWidth; i++) {
    for (int stride = 0; stride < NUM_LEDS / strideWidth; stride++) {
      ledsL[NUM_LEDS - (stride * strideWidth + i)] = CHSV(hue, 255, 255);
      ledsR[NUM_LEDS - (stride * strideWidth + i)] = CHSV(hue, 255, 255);
    }

    FastLED.show();
    fadeall(200);
    delay(15);
  }
}

void deaccelerate() {
  uint8_t hue = 0; // Red

  for (int i = 0; i < NUM_LEDS; i++) {
    ledsL[i] = CHSV(hue, 255, 255);
    ledsR[i] = CHSV(hue, 255, 255);
  }

  FastLED.show();
  delay(100);

  for (int i = 0; i < 300; i++) {
    fadeall(245);
    FastLED.show();
    delay(5);
  }

  //  FastLED.show();
  //  delay(50);
}

void kitfox() {
  int start = 0;
  int end = NUM_LEDS;

  uint8_t hue = 0; // Red

  uint8_t fadeScale = 200;
  int ndelay = 15;

  for (int i = start; i < end; i++) {
    ledsL[i] = CHSV(hue, 255, 255);
    ledsR[i] = CHSV(hue, 255, 255);
    FastLED.show();
    fadeall(fadeScale);
    delay(ndelay);
  }

  for (int i = end - 1; i >= start; i--) {
    ledsL[i] = CHSV(hue, 255, 255);
    ledsR[i] = CHSV(hue, 255, 255);
    FastLED.show();
    fadeall(fadeScale);
    delay(ndelay);
  }
}


enum pattern_t {
  PATTERN_NONE = 0,
  PATTERN_NOTICE = 1,
  PATTERN_ACCELERATE = 2,
  PATTERN_DISCO = 3,
  PATTERN_NOISE = 4,
  PATTERN_LAST = 5
};

int current_pattern = PATTERN_NONE;
int debounce = 0;

int next_pattern(int pattern) {
  int ret = pattern + 1;
  if (ret == PATTERN_LAST) {
    return PATTERN_NONE;
  } else {
    return ret;
  }
}

int discoHue = 0;
int discoOffset = 0;

void discoStep() {
  if (discoOffset < NUM_LEDS) {
    int offset = discoOffset;
    ledsL[offset] = CHSV(discoHue, 255, 255);
    ledsR[NUM_LEDS - offset - 1] = CHSV(discoHue, 255, 255);
  } else {
    int offset = NUM_LEDS - (discoOffset - NUM_LEDS) - 1;
    ledsR[offset] = CHSV(discoHue, 255, 255);
    ledsL[NUM_LEDS - offset - 1] = CHSV(discoHue, 255, 255);
  }

  fadeall(240);

  discoOffset++;
  if (discoOffset >= NUM_LEDS * 2) {
    discoOffset = 0;
  }

  discoHue++;
  if (discoHue > 255) {
    discoHue = 0;
  }
}

const uint8_t kMatrixWidth = 2;
const uint8_t kMatrixHeight = NUM_LEDS;
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
const bool    kMatrixSerpentineLayout = false;

uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];

uint16_t XY( uint8_t x, uint8_t y)
{
  uint16_t i;

  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }

  return i;
}

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 60; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
 uint16_t scale = 4011; // very zoomed out and shimmery
//uint16_t scale = 311;


// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}

void noiseStep() {
  fillnoise8();

  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {
      if (i == 0) {
        ledsR[j] = CHSV(noise[j][i],255,noise[i][j]);
      } else {
        ledsL[j] = CHSV(noise[j][i],255,noise[i][j]);
      }
      // We use the value at the (i,j) coordinate in the noise
      // array for our brightness, and the flipped value from (j,i)
      // for our pixel's hue.

      // You can also explore other ways to constrain the hue used, like below
      // leds[XY(i,j)] = CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]);
    }
  }
}

void loop() {

  debouncer.update();

  // int sensorValue = analogRead(LIGHT_SENSOR_PIN);

  switch (current_pattern) {
    case PATTERN_NONE:
      resetall();
      break;
    case PATTERN_NOTICE:
      noticeStep();
      break;
    case PATTERN_ACCELERATE:
      accelerateStep();
      break;
    case PATTERN_DISCO:
      discoStep();
      break;
    case PATTERN_NOISE:
      noiseStep();
      break;
  }

  int value = debouncer.read();

  if ( value == LOW ) {
    digitalWrite(18, HIGH );
    if (!debounce) {
      whiteflash(2);
      current_pattern = next_pattern(current_pattern);
      debounce = 1;
    }
  } else {
    digitalWrite(18, LOW );
    debounce = 0;
  }

  FastLED.show();

  delay(10);
}
