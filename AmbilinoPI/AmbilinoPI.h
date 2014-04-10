// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _AmbilinoPi_H_
#define _AmbilinoPi_H_
#include "Arduino.h"
#include "FastLED.h"

// remember to change the line in SoftwareSerial.h:
// #define _SS_MAX_RX_BUFF xx
// to
// #define _SS_MAX_RX_BUFF 256
#include <SoftwareSerial.h>

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

/*** Physical Setup **/
	const uint8_t 	COMM_SW_RX_PIN 	= 	8;
	const uint8_t 	COMM_SW_TX_PIN 	= 	11;

	// Left Strip
	const uint8_t 	LED_LEFT_PIN 		= 	5;
	const uint32_t 	LED_LEFT_COUNT 		= 	18;
	// Right Strip
	const uint8_t 	LED_RIGHT_PIN 		= 	7;
	const uint32_t 	LED_RIGHT_COUNT 	= 	0;
	// Top Strip
	const uint8_t 	LED_TOP_PIN 		= 	6;
	const uint32_t 	LED_TOP_COUNT 		= 	15;

/*** Other Options ***/

	// Set delay of getting new Data in ms. make it between 10 an 30 Hz
	#define CAPTURE_DELAY_MS 30

	// delay between 2 smoothing steps - empiric
	#define SMOOTHING_DELAY CAPTURE_DELAY_MS / 5
	//CAPTURE_DELAY_MS / 5

	// TODO umbau auf max # steps?
	// Maximimum Smoothing time for one frame. ~~ CAPTURE / 3 seems OK. empiric!
	#define SMOOTHING_MAX_DELAY 4 * SMOOTHING_DELAY

	// Color Difference: The Higher the value, the more reactive but also more aggressive
	#define SMOOTHING_MAX_STEP 5

	// Set maximum brightness (0-255)
	const uint32_t LED_MAX_BRIGHTNESS = 32;

	// Communication to Raspberry
	#define COMM_SYNC_PREFIX "SYNC"
	#define COMM_SYNC_POSTFIX "\r\n"
	#define COMM_SYNC_PREFIX_LENGTH 4
	#define COMM_SYNC_POSTFIX_LENGTH 2

/*** Debug Modes ***/

	// use simulated input
//	#define MODE_SIMULATION 					// simulate input

	// echo Soft and Hardware Serial to HardwareSerial
	#define MODE_ECHO

/*** LEDs ***/


	/*
	 * Set the Orientation of the Stripes. The orientation points from the cable connection to the loose end.
	 * If Reverse[i] = false, it looks like this:
	 *
	 *			Top
	 *   | ---------------> | R
	 * L |			  		| I
	 * E |			 		| G
	 * F |			 		| H
	 * T |					| T
	 *   v ---------------> v
	 *			Bottom
	 */

	// TODO struct LED_SETUP!
	// pin, count, reverse
	bool LED_REVERSE_STRIPE[4] = {false, false, false, false};

	// Sum up stripes
	const uint32_t LED_COUNT = LED_LEFT_COUNT + LED_RIGHT_COUNT + LED_TOP_COUNT;

	// 3 Channels (RGB) per LED
	const uint32_t LED_CHANNELS = LED_COUNT * 3;

	// create color structures
	CRGB leds_left[LED_LEFT_COUNT];
	CRGB leds_right[LED_RIGHT_COUNT];
	CRGB leds_top[LED_TOP_COUNT];
	// TODO bottom

	// structs for leds
	int currValues[LED_COUNT][3];
	int scanValues[LED_COUNT][3];
	int diffValues[LED_COUNT][3];

	// debugging led
	const int led_pin = 13;

/*** Communication ***/
	const uint32_t COMM_FRAMESIZE = COMM_SYNC_PREFIX_LENGTH + COMM_SYNC_POSTFIX_LENGTH + LED_CHANNELS;

	#define 	COMM_HW_BAUDRATE 	115200
	#define 	COMM_SW_BAUDRATE 	57600

	// TODO letztlich auf USB serial umstellen? arduino und raspberry über USB verbinden!
	SoftwareSerial 	softSerial(COMM_SW_RX_PIN, COMM_SW_TX_PIN);
	uint8_t* serialBuffer = new uint8_t[COMM_FRAMESIZE];
	uint32_t serialCounter;

/*** Functions ***/

	void fastLoop();
	void filterValues();
	void limitChange();
	void getNewScan();

	// dummy loop fkt. for arduino. Using fastLoop (for(;;)) instead for better performance
	void loop() {}

#endif /* _AmbilinoPi_H_ */
