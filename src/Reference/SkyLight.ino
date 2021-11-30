
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

// Directives
#define USE_OCTOWS2811
// External Libraries
#include "OctoWS2811.h"
#include "FastLED.h"
#include "IntervalTimer.h"
// Local Libraries
#include "ArtMap.h"

// Settings
#define NUM_BOXES_CAN_BE_INJECTED 40
#define NUM_BOXES_PER_SEXTANT 18
#define NUM_STRIPS 7
#define NUM_CTRLS 1
#define FRAME_COUNTDOWN_RESET 10
#define MVMNT_TICK_HZ 600
#define CHANCE 200
#define DEFAULT_RATE 15
#define TICKS_FOR_NEW_COLOR 5
#define STEPS_IN_INJ_ANIM 9

// Soft Enums
#define ALL_SEXTANTS 6
#define BTN_ARCADE 0
#define BTN_ENC 1

// GPIOs
#define PIN_BTN_ARC_0 1
#define PIN_BTN_ARC_1 1
#define PIN_BTN_ARC_2 1
#define PIN_BTN_ENC_0 1
#define PIN_BTN_ENC_1 1
#define PIN_BTN_ENC_2 1
#define PIN_ENC_0_A 1
#define PIN_ENC_0_B 1
#define PIN_ENC_1_A 1
#define PIN_ENC_1_B 1
#define PIN_ENC_2_A 1
#define PIN_ENC_2_B 1


// Data Structures
// Box of light that can move from shape to shape
typedef struct Box {
	bool active = false;
	uint8_t sextant = 0;
	uint8_t box_idx = 0;
	uint8_t hue = 0;
	uint8_t move_counter = 0;
	int age = 0;
} Box;

// Objects
IntervalTimer movement_ticker;				// Timer to keep things a-ticking
CRGB leds[NUM_STRIPS * SEXTANT_LED_COUNT];	// Data structure for all LEDs
Box boxes[NUM_BOXES_CAN_BE_INJECTED];		// Keeps track of the injected color-boxes

// Variables
uint8_t frameCountDown = FRAME_COUNTDOWN_RESET;		// Keeps timing for the updating of LEDs

// Gates
volatile bool show_LED_gate = false;
volatile bool move_gate = false;

void setup() {
	LEDS.addLeds<OCTOWS2811>(leds, SEXTANT_LED_COUNT);
	mapInit();

	movement_ticker.begin(mvmntTick, (1000000 / MVMNT_TICK_HZ));

	// Init Controllers
	// TODO: setup interrupt handlers for all encoders, buttons, etc.
}

void loop() {

	// Tick down the movement counter for each element
	if (move_gate) {
		doMove();
		move_gate = false;
	}

	// Show next frame gate
	if (show_LED_gate) {
		LEDS.show();
		show_LED_gate = false;
	}
}


// TODO: turn this into interupt handler
void btnEnc(uint8_t idx) {
	if (digitalRead(ctrl[idx].pin_btn_enc)) {
		if (!ctrl[idx].btn_pressed[BTN_ENC]) {
			ctrl[idx].btn_pressed[BTN_ENC] = true;
			ctrl[idx].btn_press_time[BTN_ENC] = millis();
			// When the button is pressed.
		}
	}
	else {
		if (ctrl[idx].btn_pressed[BTN_ENC]) {
			ctrl[idx].btn_pressed[BTN_ENC] = false;
			// When the button is released.
			long press_time = millis() - ctrl[idx].btn_press_time[BTN_ARCADE];
			if (press_time > 0) {
				// Can do things depending how long the button was pressed.
			}
			else
			{

			}
		}
	}
}

// TODO: turn this into interupt handler
void btnArcade(uint8_t idx) {
	if (digitalRead(ctrl[idx].pin_btn_enc)) {
		if (!ctrl[idx].btn_pressed[BTN_ARCADE]) {
			ctrl[idx].btn_pressed[BTN_ARCADE] = true;

			// When the button is pressed.
				// TODO: Start injection animation
			if (ctrl[idx].animating_injection) {
				for (int i = 0; i < 10; i++)
				{
					leds[ctrl_px_inject_path[i]] = CRGB::Black;
				}
				ctrl[idx].anim_step = 0;
			}
			else
			{
				ctrl[idx].animating_injection = true;
			}

				// Inject color
			injectColor(ctrl[idx].color_picker);
		}
	}
	else {
		ctrl[idx].btn_pressed[BTN_ARCADE] = false;
	}
}


// TODO: turn this into interupt handler
void chkEncoder(uint8_t idx) {


	if (int8_t val = ctrl[idx].enc->read()) {
		ctrl[idx].new_color_counter += val;
		if (ctrl[idx].new_color_counter <= 0) {
			ctrl[idx].new_color_counter = TICKS_FOR_NEW_COLOR;
			ctrl[idx].color_picker--;
			updateControllerLEDs(idx, CHSV(ctrl[idx].color_picker, 255, 255));
		}
		else if (ctrl[idx].new_color_counter >= (2*TICKS_FOR_NEW_COLOR))
		{
			ctrl[idx].new_color_counter = TICKS_FOR_NEW_COLOR;
			ctrl[idx].color_picker++;
			updateControllerLEDs(idx, CHSV(ctrl[idx].color_picker, 255, 255));
		}
		hue++;
		LEDS.show();
		LEDS.delay(10);
	}

	// TODO: What is this nonsense?
	/*
	CHSV bloop = CHSV(hue, 255, 255);
	lightBox(ALL_SEXTANTS, box, bloop);
	box = (box + 1) % NUM_BOXES;
	hue += 20;
	FastLED.show();
	delay(200);
	ctrl[idx].enc->write(0);
	*/
}

//TODO: Convert from DLL to simple array
void injectColor(uint8_t color) {
}
// TODO: This might be useful, but I don't remember how it works. Probably just inline this with the encoder int handler
void updateControllerLEDs(uint8_t idx, CHSV c) {
	// Selected color
	for (uint8_t i = 0; i < 9; i++)
	{
		leds[ctrl_px_selected[i]] = c;
	}
	// Spectrum

	c.h -= 5;
	for (int8_t i = 0; i < 10; i++)
	{
		leds[ctrl_px_spectrum[0]] = c;
		c.h += (i == 4 ? 1 : 2);
	}
}

// Interrupt handler for gating all times events
void mvmntTick() {
	move_gate = true;
	frameCountDown--;
	if (!frameCountDown) {
		frameCountDown = FRAME_COUNTDOWN_RESET;
		show_LED_gate = true;
	}
}




// Light a particular box in one or all sextants
void lightBox(int sextant, int box, CHSV color) {

	CRGB c = color;

	// Ad Hoc fix for central box being in all six quadrants
	if (box == 0) {
		for (int s = 0; s < 6; s++)
		{
			for (int i = shape_start_addr[s][0]; i < shape_start_addr[s][1]; i++)
			{
				leds[i] = c;
			}
		}
	}
	else {
		// Determine if this is to light all sextants or just one.
		for (int s = (sextant == ALL_SEXTANTS ? 0 : sextant); s <= (sextant == ALL_SEXTANTS ? 5 : sextant); s++)
		{
			// Set color for all px indices in the box
			for (int i = shape_start_addr[s][box]; i < shape_start_addr[s][box + 1]; i++)
			{
				leds[i] = c;
			}
		}
	}
}


// Light all boxes in a particular level (depth) in one or all sextants
void lightLevel(int sextant, int level, CHSV color) {
	for (int j = 0; j < (level == 0 ? 2 : 4); j++) // There are only 2 shapes on level 0, but 4 shapes on all other levels
	{
		lightBox(sextant, levels[level][j], color);
	}
}

// Light all boxes of a particular shape in one or all sextants
void lightShapes(int sextant, int shape, CHSV color) {
	for (int j = 0; j < shape_count[shape]; j++)
	{
		lightBox(sextant, shapes[shape][j], color);
	}
}
