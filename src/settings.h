
#ifndef SETTINGS_H
    #define SETTINGS_H

    #define SEXTANT_LED_COUNT (12+27+27+27)
    #define NUM_STRIPS 6
    #define NUM_LEDS (NUM_STRIPS * SEXTANT_LED_COUNT)
    #define ANIM_TIME 60 * 1000
    
    // Video
    #ifdef DEBUG
        #define FPS                 20
    #else
        #define FPS                 100
    #endif 

    

#endif

