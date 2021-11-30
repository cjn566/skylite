#pragma once

#include "../Animation.h"
#include "util.h"
//#include "radii.h"

struct Sparkles: public AnimationBase{


    // params vars
    //uint8_t * = 3;

    // state vars
    //uint8_t * = 0;

    public:
    Sparkles(){
        numParams = 0;
        // params = new parameter_t[numParams];

        //     params[].max = ;
        //     params[].ticksToAdjust = ;
        //     params[].scaleColor = CRG::;

    };

    int sparkliness = 0;
    int sparkleThreshold;
    int sparkleIdices[NUM_LEDS];
    long sparkleTimer = 0;

    int adjParam(uint8_t paramIdx, int change){
        sparkliness = CLAMP_8(sparkliness + change);
        sparkleThreshold = 100 - scale_to_n(sparkliness, 255, 100);
        return sparkliness;
    }

    void drawFrame(uint8_t stepsSinceLastFrame){
        #ifdef DEBUG
        //Serial.println();
        #endif
        if(sparkliness) {
            sparkleTimer += stepsSinceLastFrame;
            if (sparkleTimer >= sparkleThreshold) {
                sparkleTimer = 0;
                ledData.leds[ ( random8() * NUM_LEDS ) / 256 ] = CRGB::White;
                for (int i = sparkliness - 180; i > 20; i -= 20)
                {
                    ledData.leds[ ( random8() * NUM_LEDS ) / 256 ] = CRGB::White;
                }
            }
        }
    }
};