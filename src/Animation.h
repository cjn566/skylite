#ifndef ANIMATION_H
    #define ANIMATION_H
    
    #include "FastLED.h"
    #include "util.h"
    #include "settings.h"

    struct if_anim_t {
        uint8_t saturation = 255;
        CRGBArray<NUM_LEDS> leds;
    };    

    enum DispType {
        CHUNKS,
        BOOL,
        HUE,
        OTHER
    };

    struct parameter_t {
        CRGB scaleColor = CRGB::Black;
        int max = 255;
        uint8_t ticksToAdjust = 1;
        DispType type = OTHER;
    };

    struct if_ui_t {
        #ifdef DEBUG
            bool selectingParams = true;
            uint8_t paramIdx = 0, numAnimParams;
        #else
            bool selectingParams = true;
            uint8_t paramIdx, numAnimParams;
        #endif
        parameter_t * currParam;
    };

    extern if_anim_t ledData;
    extern if_ui_t if_ui;


    struct AnimationBase {
        int8_t speed = SPEED_SCALE_BASE;
        uint8_t numParams;
        parameter_t *params;
        AnimationBase(){};
        virtual void        initAnim(){};
        virtual int         adjParam(uint8_t paramIdx, int change);
        virtual void        drawFrame(int16_t){};
        virtual void        speedAdjHandler(){};
        int8_t             adjSpeed(int adj){
            speed  = CLAMP_S8(speed + adj);
            speedAdjHandler();
            return speed;
        }
        void                drawBase(uint16_t millisSinceLastFrame){
            // get modified time passed
            int16_t modTime = millisSinceLastFrame * speed;
            drawFrame(modTime);
        }
    };

#endif