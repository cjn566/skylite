#include "../Animation.h"

struct Particles: public AnimationBase{

    #ifdef DEBUG
        #define MAX_PARTICLES 64
    #else
        #define MAX_PARTICLES 64
    #endif

    #define TS_TRUNC    3
    #define TS_MOD MAX_UVAL_N_BITS(TS_W)

    #define LOC_REDUCTION       12
    #define VEL_LEFT_SH         9
    #define ACCEL_RIGHT_SH      5
    #define MAX_VELOC (1 << (LOC_REDUCTION + 4))

    #define LS_LEFT_SH          8
    #define LOC_MAX (ENDPOINT << (LOC_REDUCTION + 8))
    #define ENDPOINT 193

    #define MAX_COLOR_SHIFT 32
    #define MAX_HUECYCLE_MS 20000
    #define MIN_HUECYCLE_MS 1000
    #define MAX_SPAWN_MS 200
    #define MIN_SPAWN_MS 25

    enum ParamName {
        SPAWN_RATE,
        RAND_START,
        COLOR_CYCLE_RATE,
        // COLOR_VAR,
        VELOCITY,
        LIFESPAN,
        MODE,
        // SPAWN_RATE_VAR,
        // VELOCITY_VAR,
        // ACCELERATION,
        // ACCELERATION_VAR,
    };

    // params vars
    enum Mode {
        WRAP,
        BOUNCE,
        DIE
    }           mode            = DIE;
    int8_t      randstart       = -100;
    uint8_t     lifespan        = 40;
    uint8_t     spawnRate       = 1;
    uint8_t     spawnRateVar    = 255;
    uint8_t     hueCycleRate    = 80;
    uint8_t     hueVar          = 0;
    int8_t      velocity        = -40;
    uint8_t     velocityVar     = 100;
    int8_t      acceleration    = 0;
    uint8_t     accelVar        = 0;

    // state vars
    uint32_t     realLifespan;
    uint8_t hue = 120;
    uint8_t numParticles = 0, newParticleIdx = 0;
    unsigned int spawnDelay, counter = 0, thisSpawnDelay;
    long hueTimer = 0, hueMaxTime = 0, spawnMaxTime;

    struct Particle{
        uint32_t birthtime;
        uint8_t start_loc, hue;
        int8_t vel_mod, accel_mod;
    } particle[MAX_PARTICLES];
    
    public:
    Particles(){
        speed = 20;
        hueMaxTime = (MAX_HUECYCLE_MS * speed);
        // numParams = 11;
        numParams = 6;
        params = new parameter_t[numParams];
        params[MODE].max = 2;
        params[MODE].type = CHUNKS;
        params[MODE].ticksToAdjust = 3;

        params[VELOCITY].max = 127;
        // params[ACCELERATION].max = 127;
        params[COLOR_CYCLE_RATE].max = 255;
        params[RAND_START].max = 127;
    };


    unsigned int setSpawnDelay(){
        spawnMaxTime = (MAX_SPAWN_MS - scale_to_n( (int)spawnRate, 255, MAX_SPAWN_MS ) + MIN_SPAWN_MS) * speed;
    };

    unsigned int setHueTime(){
        hueMaxTime = (MAX_HUECYCLE_MS - scale_to_n( (int)hueCycleRate, 255, MAX_HUECYCLE_MS ) + MIN_HUECYCLE_MS) * speed;
    };

    void speedAdjHandler () {
        setSpawnDelay();
        setHueTime();
    }

    void initAnim(){
        random16_add_entropy(millis() & 0xffff);
        setSpawnDelay();
        setHueTime();
        realLifespan = lifespan << LS_LEFT_SH;

        for(int i =0; i<MAX_PARTICLES; i++) particle[i].birthtime = 0; // Reset Particles
        numParticles = 0;
    }

    int adjParam(uint8_t paramIdx, int change){
        random16_add_entropy(millis() & MAKE_MASK(16));
        switch(paramIdx){
            case MODE:
                mode = (Mode)clamp_un0(mode + (change > 0? 1:-1), 2);
                return mode;

            case RAND_START:
                randstart = CLAMP_S8(randstart + change);
                return randstart;
                
            case LIFESPAN:
                lifespan = CLAMP_8(lifespan + change);
                realLifespan = lifespan << LS_LEFT_SH;
                return lifespan;
                
            case SPAWN_RATE:
                spawnRate = CLAMP_8(spawnRate + change);
                setSpawnDelay();
                return spawnRate;

            // case SPAWN_RATE_VAR:
            //     spawnRateVar = CLAMP_8(spawnRateVar + change);
            //     return spawnRateVar;

            case COLOR_CYCLE_RATE:
                hueCycleRate = clamp_un0(hueCycleRate + change, (int)params[COLOR_CYCLE_RATE].max);
                setHueTime();
                return hueCycleRate;
                
            // case COLOR_VAR:
            //     hueVar = CLAMP_8(hueVar + change);
            //     return hueVar;

            case VELOCITY:
                velocity = CLAMP_S8(velocity + change);
                return velocity;

            // case VELOCITY_VAR:
            //     velocityVar = CLAMP_8(velocityVar + change);
            //     return velocityVar;

            // case ACCELERATION:
            //     acceleration = CLAMP_S8(acceleration + change);
            //     return acceleration;

            // case ACCELERATION_VAR:
            //     accelVar = CLAMP_8(accelVar + change);
            //     return accelVar;

            default: return 0;
        }
        return 0;
    }

    void killParticle(int i){
        if(particle[i].birthtime){
            particle[i].birthtime = 0;
            numParticles--;
        }
    }

    int debugCounter = 0;
    int displayCtr =  (speed * 100);

    long thisSpawnMaxTime;

    void drawFrame(int16_t millisSinceLastFrame){

        hueTimer += hueCycleRate > 0 ? millisSinceLastFrame : 0;
        if(hueTimer > hueMaxTime){
            hueTimer -= hueMaxTime;
        }
        hue = scale_to_n(hueTimer, hueMaxTime, (long)255);


        unsigned long now = millis() >> TS_TRUNC;

        // Spawn  
        counter += abs(millisSinceLastFrame);
        if(counter > thisSpawnMaxTime){

            counter = 0;

            int variance = ((random8() - 127) * spawnRateVar * spawnMaxTime);
            variance /= (256*127);

            //Serial.printf("v : %d\n", variance);

            thisSpawnMaxTime = spawnMaxTime + variance;

            #ifdef DEBUG
                Serial.printf("%d, %d, %d, %d, %d, %d\n", speed, spawnRate, lifespan, spawnMaxTime, variance, thisSpawnMaxTime);
            #endif
            // Build new particle
            if (numParticles >= MAX_PARTICLES) {
                newParticleIdx = (newParticleIdx + 1) % MAX_PARTICLES;
                killParticle(newParticleIdx);
            } else {
                while(particle[newParticleIdx].birthtime){	// Find next available slot
                    newParticleIdx = (newParticleIdx + 1) % MAX_PARTICLES;
                }
            }
            numParticles++;

            int boop = scale_to_n((int)random8(), 255, ENDPOINT);
            int loc_rand = scale_by_n(boop, 127 - abs((int)randstart), 127);
            if(randstart < 0){
                loc_rand = (uint8_t)(ENDPOINT - (uint8_t)loc_rand);
            }

            int hueRand = scale8(random8(), hueVar);
            int velRand = scale_by_n(random8()-127, (int)velocityVar, 255);
            int velTot = velRand + velocity;
            velTot = velTot >> 1;
            int accelRand = scale_by_n(random8()-127, (int)accelVar, 255);
            int accelTot = acceleration + accelRand;
            accelTot = accelTot >> 1;

            particle[newParticleIdx].birthtime  = now;
            particle[newParticleIdx].start_loc  = (uint8_t)loc_rand;
            particle[newParticleIdx].hue        = hue + hueRand;
            particle[newParticleIdx].vel_mod     = velTot;
            particle[newParticleIdx].accel_mod   = accelTot;            
        }

        // Serial.printf("%d-", numParticles);

        for (int i = 0; i < MAX_PARTICLES; i++){
            if(particle[i].birthtime){
                unsigned long age = (now - particle[i].birthtime);
                age *= speed;
                if(lifespan < 255 && age > realLifespan){
                    killParticle(i);
                    continue;
                }                
                if(!age){
                    age = 1;
                }

                int veloc = (particle[i].vel_mod) << VEL_LEFT_SH;

                int fullAccel = (particle[i].accel_mod) * age;
                int reducedAccel = fullAccel >> ACCEL_RIGHT_SH;
                veloc = clamp_sn(veloc + reducedAccel, MAX_VELOC);
                int location = veloc * age;

                //Serial.printf("%d\n", age);
                //Serial.printf("%d,%d,%d,%d\n", fullAccel, reducedAccel, veloc, location);

                if(particle[i].start_loc) location += ((LOC_MAX/ENDPOINT) * particle[i].start_loc);

                //Serial.printf("%d", location);

                if((location > LOC_MAX) || (location < 0)){                    
                    bool under = (location < 0);
                    if(under) location *= -1;
                    switch(mode){
                        case DIE:
                            killParticle(i);
                            continue;
                        case BOUNCE:
                            under = (location / LOC_MAX) & 0x1;     
                        case WRAP:
                            if(under){
                                location = LOC_MAX - (location % LOC_MAX);
                            } else {
                                location = location % LOC_MAX;
                            }
                            break;
                    }
                }

                int reduced_loc = location >> LOC_REDUCTION;
                uint8_t loc_idx = (reduced_loc >>  8) + 1;
                uint16_t loc_fractional = reduced_loc & 0xff;

                //Serial.printf(" - %x, %d, %x\n", reduced_loc, loc_idx, loc_fractional);

                int numspread = scale_to_n(veloc, MAX_VELOC, 3) + 1;
                int fullFractionalValue = 256/numspread;
                uint8_t scaledFraction = scale_to_n((int)loc_fractional, 255, fullFractionalValue);

                for(int j = 1; j<=numspread; j++){
                    uint8_t forwardValue =  (numspread - j ) * fullFractionalValue + scaledFraction;
                    uint8_t revValue =  (numspread - j + 1) * fullFractionalValue - scaledFraction;
                    if(loc_idx + j < ENDPOINT){
                        ledData.leds[loc_idx + j] += CHSV(particle[i].hue, ledData.saturation, forwardValue);
                    }
                    if(loc_idx - j > 0)
                        ledData.leds[loc_idx - j] += CHSV(particle[i].hue, ledData.saturation, revValue);
                }
                ledData.leds[loc_idx] += CHSV(particle[i].hue, ledData.saturation, 255);
            }
        }
    }
};









