

#define USE_OCTOWS2811

#include <Arduino.h>
#include <OctoWS2811.h>
#include <FastLED.h>

#include "Animation.h"

// CRGB leds[NUM_STRIPS * SEXTANT_LED_COUNT];

void setup() {
	// FastLED.addLeds<OCTOWS2811>(ledData.leds, SEXTANT_LED_COUNT);
  FastLED.setMaxRefreshRate(FPS);
	mapInit();  
}


// -------- ANIMATIONS ---------------
// #define NUM_ANIMATIONS 2

// #include "Animations/rainbow.cpp"
// #include "Animations/particles.cpp"

// Rainbow rainbow = Rainbow();
// Particles particles = Particles();

// AnimationBase *allAnims[NUM_ANIMATIONS] = {
//     &particles,
//     &rainbow
//     };


// #ifdef DEBUG
//   uint8_t currAnimationIdx = 0;
// #else
//   uint8_t currAnimationIdx = 1;
// #endif

// void initAnimation(){
//   allAnims[currAnimationIdx]->initAnim();
// }

//-------------- THE LOOP -------------------------

long lastMillis;
long animationtTimer = 0;
void loop() {    
  long now = millis();
  long elapse = now - lastMillis;
  lastMillis = now;

  // FastLED.showColor(CRGB::Blue);

  FastLED.show();
  FastLED.clear();
  allAnims[currAnimationIdx]->drawFrame(elapse);


  animationtTimer += elapse;
  if(animationtTimer > ANIM_TIME) {
    animationtTimer -= ANIM_TIME;
    currAnimationIdx = (currAnimationIdx + 1) % NUM_ANIMATIONS;
    initAnimation();
  }
}