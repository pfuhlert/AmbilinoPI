// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _AmbilinoPi_H_
#define _AmbilinoPi_H_
#include "Arduino.h"
#include "FastLED.h"
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
	const uint32_t 	LED_RIGHT_COUNT 	= 	18;
	// Top Strip
	const uint8_t 	LED_TOP_PIN 		= 	6;
	const uint32_t 	LED_TOP_COUNT 		= 	24;

/*** Other Options ***/

	// Set delay of getting new Data in ms. make it between 10 an 30 Hz
	#define CAPTURE_DELAY_MS 20

	// delay between 2 smoothing steps - empiric
	#define SMOOTHING_DELAY CAPTURE_DELAY_MS / 5

	// TODO umbau auf max # steps?
	// Maximimum Smoothing time for one frame. ~~ CAPTURE / 3 seems OK. empiric!
	#define SMOOTHING_MAX_DELAY 4 * SMOOTHING_DELAY

	// Color Difference: The Higher the value, the more reactive but also more aggressive
	#define SMOOTHING_MAX_STEP 20

	// Set maximum brightness (0-255)
	const uint32_t LED_MAX_BRIGHTNESS = 32;

	// Communication to Raspberry
	#define SYNC_PREFIX "SYNC"
	#define SYNC_POSTFIX "\r\n"
	#define SYNC_PREFIX_LENGTH 4
	#define SYNC_POSTFIX_LENGTH 2

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
	// TODO implement bottom?

	const uint32_t COMM_FRAMESIZE = SYNC_PREFIX_LENGTH + SYNC_POSTFIX_LENGTH + LED_CHANNELS;

/*** Communication ***/

	#define 	COMM_HW_BAUDRATE 	115200
	#define 	COMM_SW_BAUDRATE 	38400

	// RX buffer size from SoftwareSerial.h needs to contain a whole frame size of #LED * 3 Byte
	#define _SS_MAX_RX_BUFF 256

	// TODO letztlich auf USB serial umstellen? arduino und raspberry über USB verbinden!
	SoftwareSerial 	softSerial(COMM_SW_RX_PIN, COMM_SW_TX_PIN);
	uint8_t* serialBuffer = new uint8_t[COMM_FRAMESIZE];
	uint32_t serialCounter;

/*** Functions ***/

	void fastLoop();
	void UpdateStrip();
	void limitChange();

	// dummy loop fkt. for arduino. Using fastLoop (for(;;)) instead for better performance
	void loop() {}

#endif /* _AmbilinoPi_H_ */
