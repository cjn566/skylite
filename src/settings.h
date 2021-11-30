
#ifndef SETTINGS_H
    #define SETTINGS_H

    #define NUM_STRIPS 6
    #define ANIM_TIME 60 * 1000
    
    // Video
    #ifdef DEBUG
        #define FPS                 20
    #else
        #define FPS                 100
    #endif

    // Misc
    #define MAX_MILLIAMPS          1500
    #define STARTUP_DELAY          500
    #define ENC_TICKS_PER_INDENT   4

    

    #define NUM_LEDS            204
    #define ORDER               GRB

#endif

