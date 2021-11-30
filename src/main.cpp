#include <Arduino.h>
#include <OctoWS2811.h>
#include <FastLED.h>

#include "ArtMap.h"
#include "util.h"
#include "settings.h"


#define USE_OCTOWS2811

CRGB leds[NUM_STRIPS * SEXTANT_LED_COUNT];

void setup() {
	FastLED.addLeds<OCTOWS2811>(leds, SEXTANT_LED_COUNT);
  FastLED.setMaxRefreshRate(FPS);
	mapInit();  
}

void initAnimation(){
  allAnims[currAnimationIdx]->initAnim();
}


// -------- ANIMATIONS ---------------
#define NUM_ANIMATIONS 3

#include "Animations/peppermint.cpp"
#include "Animations/rainbow.cpp"
#include "Animations/particles.cpp"

Peppermint peppermint = Peppermint();
Rainbow rainbow = Rainbow();
Particles particles = Particles();

AnimationBase *allAnims[NUM_ANIMATIONS] = {
    &particles,
    &peppermint,
    &rainbow
    };

#ifdef DEBUG
  uint8_t currAnimationIdx = 0;
#else
  uint8_t currAnimationIdx = 2;
#endif

//-------------- THE LOOP -------------------------

long lastMillis;
long animationtTimer = 0;
void loop() {    
  long now = millis();
  long elapse = now - lastMillis;
  lastMillis = now;
  FastLED.show();
  // FastLED.clear(); ?
  allAnims[currAnimationIdx]->drawBase(elapse);


  animationtTimer += elapse;
  if(animationtTimer > ANIM_TIME) {
    animationtTimer -= ANIM_TIME;
    currAnimationIdx = (currAnimationIdx + 1) % NUM_ANIMATIONS;
    initAnimation();
  }
}