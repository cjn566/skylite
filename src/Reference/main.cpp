#include <Arduino.h>
#include <Encoder.h>
#include "Animation.h"
#include "util.h"

// Parameters
uint8_t brightness = INIT_BRIGHTNESS;

if_anim_t ledData;
if_ui_t if_ui;  
unsigned long click_wait_start_millis = 0, lastActivityMillis;

// -------- ANIMATIONS ---------------
#define NUM_ANIMATIONS 3
#include "Animations/peppermint.cpp"
#include "Animations/rainbow.cpp"
#include "Animations/particles.cpp"
#include "Animations/sparkles.cpp"
Peppermint peppermint = Peppermint();
Rainbow rainbow = Rainbow();
Particles particles = Particles();
Sparkles sparkles = Sparkles();
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

//-------------- INTERRUPT HANDLERS -------------------------

volatile bool v_debouncing = false;
volatile unsigned long v_debounceStartTime = 0;
void debounceButton()
{
  if (!v_debouncing)
  {
    v_debouncing = true;
    v_debounceStartTime = millis();
  }
}

//-------------- SETUP -------------------------
void changeState(UI_State);
void initAnimation();
void initParam();

void setup()
{
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  delay(STARTUP_DELAY);            

  encoder.write(0);
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENCODER_BTN), debounceButton, CHANGE);

  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
  FastLED.addLeds<WS2812B, LED_DATA, ORDER>(ledData.leds, NUM_LEDS);
  FastLED.setBrightness(INIT_BRIGHTNESS);
  FastLED.setMaxRefreshRate(FPS);
  
  int mostParams = 0;
  for(int i=0;i<NUM_ANIMATIONS;i++){
    int numParams = allAnims[i]->numParams;
    if (numParams > mostParams) mostParams = numParams;
  }


  params = new parameter_t[NUM_GLOBAL_PARAMS + mostParams];

  params[SPEED].max                 = 127;
  params[SPEED].scaleColor          = CRGB::Green;
  params[SPEED].ticksToAdjust       = 1;
  params[SPEED].type                = OTHER;

  params[SPARKLES].max                 = 255;
  params[SPARKLES].scaleColor          = CRGB::Turquoise;
  params[SPARKLES].ticksToAdjust       = 1;
  params[SPARKLES].type                = OTHER;

  params[SATURATION].max            = 255;
  params[SATURATION].scaleColor     = CRGB::Red;
  params[SATURATION].ticksToAdjust  = 1;
  params[SATURATION].type           = OTHER;  

  ui.init(params, mostParams);
  
  initAnimation();
  initParam();  
}

//-------------- UI -> PARAMETERS ---------------------------------

uint8_t numTotalParams;
void changeSelectedParam(bool up){
  if(up && (if_ui.paramIdx == (numTotalParams - 1)))
    if_ui.paramIdx = 0;
  else if(!up && (if_ui.paramIdx == 0))
    if_ui.paramIdx = (numTotalParams - 1);
  else if_ui.paramIdx += INCDEC;
  initParam();
}

uint8_t fastScrollCtr = 2;
long lastScrollMillis = 0;
int fastScroll(bool up) {
  int delay = millis() - lastScrollMillis;
  if(delay < FAST_SCROLL_MS){
    fastScrollCtr = clamp_un0(fastScrollCtr+1, FAST_SCROLL_MAX) ;
  } else if( delay > FAST_SCROLL_RESET){
    fastScrollCtr = 2;
  }
  lastScrollMillis = millis();
  return ((INCDEC * fastScrollCtr) >> 1);
}

void adjParam(bool up)
{
  switch ( if_ui.paramIdx)
  {
    case SPEED:
      ui.setValue(allAnims[currAnimationIdx]->adjSpeed(fastScroll(up)));
      break;
    case SPARKLES:
      ui.setValue(sparkles.adjParam(0, fastScroll(up)));
      break;
    case SATURATION:
      ledData.saturation = CLAMP_8(ledData.saturation + fastScroll(up));
      ui.setValue(ledData.saturation);
    default:
      ui.setValue(allAnims[currAnimationIdx]->adjParam(if_ui.paramIdx - NUM_GLOBAL_PARAMS, fastScroll(up)));
      break;
  }
}

void handleSpin(bool up){
  switch (ui_state)
  {
  case EDIT:
    if(if_ui.selectingParams){
      changeSelectedParam(up);
    } else {
      adjParam(up);
    }
    break;
  case HOME:
    brightness = CLAMP_8(brightness + fastScroll(up));
    FastLED.setBrightness(brightness);
    break;
  default:
    break;
  }
}

uint8_t ticksToAdjust = 1;
void initParam()
{
    if_ui.currParam = &params[ if_ui.paramIdx];
    ui.setParameter();
    switch( if_ui.paramIdx ){
      case SPEED:
        ui.setValue(allAnims[currAnimationIdx]->speed);
        break;
      case SPARKLES:
        ui.setValue(sparkles.adjParam(0,0));
        break;
      case SATURATION:
        ui.setValue(ledData.saturation);
        break;
      default:
        ui.setValue(allAnims[currAnimationIdx]->adjParam(if_ui.paramIdx - NUM_GLOBAL_PARAMS, 0));
        break;
    }
}

//-------------- UI -> STATE ---------------------------------

void changeState(UI_State newState)
{
  switch (newState)
  {
  case EDIT: //Entering edit mode
    initParam();
    break;
  default:
    break;
  }
  ui_state = newState;
}

void initAnimation(){
    if_ui.numAnimParams = allAnims[currAnimationIdx]->numParams;
    numTotalParams = if_ui.numAnimParams + NUM_GLOBAL_PARAMS;
    if(if_ui.paramIdx > numTotalParams){
      if_ui.paramIdx = 0;
      initParam();
    }
    memcpy(&params[NUM_GLOBAL_PARAMS], allAnims[currAnimationIdx]->params, sizeof(parameter_t) * if_ui.numAnimParams);
    ui.setAnimation(allAnims[currAnimationIdx]);
    allAnims[currAnimationIdx]->initAnim();
}

bool clickWaiting = false;

void handleButton()
{
  noInterrupts();
  lastActivityMillis = v_debounceStartTime;
  interrupts();
  bool isPressed = !digitalRead(ENCODER_BTN);

  if (isPressed) {
    click_wait_start_millis = lastActivityMillis;
    clickWaiting = true;
  } else if(clickWaiting) {
    clickWaiting = false;
    switch (ui_state) {
    case HOME:
      currAnimationIdx = (currAnimationIdx + 1) % NUM_ANIMATIONS;
      initAnimation();
      break;
    case EDIT:  // Button is pressed while in edit mode
      if_ui.selectingParams = !if_ui.selectingParams;
      if(if_ui.selectingParams){
        ticksToAdjust = 1;
      } else {
        ticksToAdjust = if_ui.currParam->ticksToAdjust;
      }
      encoder.write(0);
      break;
    }
  }
}

// -------------- VISUAL FUNCTIONS -------------------------

long elapseMillis;
void doFrame()
{
  long now = millis();
  long elapse = now - elapseMillis;
  if(elapse > 50) elapse = 50;
  elapseMillis = now;

  FastLED.show();
  FastLED.clear();

  allAnims[currAnimationIdx]->drawBase(elapse);
  if ( ui_state == EDIT ) ui.draw();
  sparkles.drawFrame(elapse);
}

//-------------- THE LOOP -------------------------
void loop()
{
  long now = millis();

  if (v_debouncing && ((now - v_debounceStartTime) > DEBOUNCE_MILLIS))
  {
    v_debouncing = false;
    handleButton();
  }
  if (clickWaiting && ((now - click_wait_start_millis) > CLICK_WAIT_MILLIS)) {
    //Held down button long enough to change modes
    switch (ui_state)
    {
    case HOME:
      changeState(EDIT);
      break;
    case EDIT:
      changeState(HOME);
      break;
    }
    clickWaiting = false;
  }
  int8_t newPosition = encoder.read();
  if ((abs8(newPosition)>>2) >= ticksToAdjust)
  {
    encoder.write(0);
    handleSpin(newPosition >= ticksToAdjust);
    lastActivityMillis = now;
  }
  doFrame();
}