#ifndef PEPPERMINT
    #define PEPPERMINT

    #include "../Animation.h"
    #include "radii.h"

    struct Peppermint: public AnimationBase{

        #define MAX_SPOKES 5
        #define MAX_SKEW 127
        enum ParamName {
            SPOKE,
            BASE_HUE,
            D_HUE,
            SKEW,
            WHITE_MODE
        };
 
        // params vars
        uint8_t deltaHue;
        uint8_t baseHue = 0;
        uint8_t numSpokes = 2;
        int8_t skew = 0;
        bool white_mode = false;

        // state vars
        uint8_t currAngle = 0, baseAngle = 0;
        uint8_t halfAngleBetweenSpokes;
        uint8_t angleBetweenSpokes;
        int millisInFullRotation = 4000 * SPEED_SCALE_BASE;
        int millisInFractionalRotation;

        public:
        Peppermint(){
            numParams = 5;
            params = new parameter_t[numParams];
            params[SPOKE].max = MAX_SPOKES;
            params[SPOKE].ticksToAdjust = 2;
            params[SPOKE].scaleColor = CRGB::OrangeRed;

            params[BASE_HUE].type = HUE;

            params[D_HUE].max = MAX_SPOKES;
            params[D_HUE].ticksToAdjust = 2;
            params[D_HUE].scaleColor = CRGB::Green;

            params[SKEW].max = MAX_SKEW;
            params[SKEW].scaleColor = CRGB::Purple;

            params[WHITE_MODE].max = 1;
            params[WHITE_MODE].type = BOOL;
            params[WHITE_MODE].ticksToAdjust = 3;
            params[WHITE_MODE].scaleColor = CRGB::White;
        };

        void initAnim(){
            if(numSpokes){
                millisInFractionalRotation = millisInFullRotation / numSpokes;
                halfAngleBetweenSpokes = (128 / numSpokes);
                angleBetweenSpokes = halfAngleBetweenSpokes * 2;
                if(!angleBetweenSpokes) angleBetweenSpokes -= 1;
                deltaHue = numSpokes;
            } else {
                millisInFractionalRotation = millisInFullRotation;
                angleBetweenSpokes = 255;
                halfAngleBetweenSpokes = 255;
                deltaHue = 1;
            }
        }

        int adjParam(uint8_t paramIdx, int change){
            switch(paramIdx){
                case SPOKE:
                    numSpokes = clamp_un0(numSpokes + change, MAX_SPOKES);
                    initAnim();
                    return numSpokes;
                case BASE_HUE:
                    baseHue += change;
                    return baseHue;
                case D_HUE:
                    deltaHue = clamp_un1(deltaHue + change, MAX_SPOKES);
                    return deltaHue;
                case SKEW:
                    skew = clamp_sn(skew + change, MAX_SKEW);
                    return skew;
                case WHITE_MODE:
                    white_mode = clamp_un0(white_mode + change, 1);
                    return white_mode;
                default: return 0;
            }
        }

        void specialScroll(bool up){
            baseAngle += INCDEC * 5;
        }

        int curMillis = 0;

        void drawFrame(int16_t scaledTimeSinceLastFrame){

            curMillis += scaledTimeSinceLastFrame;
            if(curMillis > millisInFractionalRotation){
                curMillis -= millisInFractionalRotation;            
            }
            currAngle = baseAngle + (uint8_t)scale_to_n(curMillis, millisInFullRotation, 255);

            for(int i=0;i< NUM_LEDS ;i++){
                uint8_t anglePlusRotation = mod8(sub8(radii[i][ANGLE], currAngle), angleBetweenSpokes);
                uint8_t distance = radii[i][DISTANCE];
                int mul_dist = qmul8(distance, 4);
                int skewFact = scale_by_n(mul_dist, (int)skew, 127);
                //uint8_t skew = scale8(halfAngleBetweenSpokes, distance);

                if(anglePlusRotation > halfAngleBetweenSpokes){
                    anglePlusRotation = angleBetweenSpokes - anglePlusRotation;
                }
                anglePlusRotation *= deltaHue;
                anglePlusRotation += skewFact;

                if(white_mode)
                    ledData.leds[i] = CHSV(baseHue, anglePlusRotation, 255);
                else
                    ledData.leds[i] = CHSV(baseHue + anglePlusRotation, ledData.saturation, 255);
            }
        }
    };
#endif