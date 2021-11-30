#ifndef ANIMATION_H
    #define ANIMATION_H
    
    #include <FastLED.h>
    #include "util.h"
    #include "settings.h"
    #include "ArtMap.h"

    struct if_anim_t {
        uint8_t saturation = 255;
        CRGBArray<NUM_LEDS> leds;
    };    

    extern if_anim_t ledData;

    struct AnimationBase {
        AnimationBase(){};
        virtual void        initAnim(){};
        virtual void        drawFrame(int16_t){};
    };

    
    // // Light a particular box in one or all sextants
    // void lightBox(int sextant, int box, CHSV color) {
    //     CRGB c = color;
        
    //     // Determine if this is to light all sextants or just one.
    //     for (int s = (sextant == -1 ? 0 : sextant); s <= (sextant == -1 ? 5 : sextant); s++)
    //     {
    //         // Set color for all px indices in the box
    //         for (int i = shape_start_addr[s][box]; i < shape_start_addr[s][box + 1]; i++)
    //         {
    //             ledData.leds[i] = c;
    //         }
    //     }
    // }

#endif