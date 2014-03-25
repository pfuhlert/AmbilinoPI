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
	const uint8_t 	COMM_SW_RX_PIN 	= 	10;
	const uint8_t 	COMM_SW_TX_PIN 	= 	11;

	// Left Strip
	const uint8_t 	LED_LEFT_PIN 		= 	7;
	const uint32_t	LED_LEFT_COUNT 		= 	18;
	// TODO Right Strip

/*** Other Options ***/

	// Set delay of getting new Data in ms. make it between 10 an 30 Hz
	#define CAPTURE_DELAY_MS 50

	// delay between 2 smoothing steps - empiric
	#define SMOOTHING_DELAY CAPTURE_DELAY_MS / 5

	// Maximimum Smoothing time for one frame. ~~ CAPTURE / 3 seems OK. empiric!
	#define SMOOTHING_MAX_DELAY 4 * SMOOTHING_DELAY

	// Color Difference: The Higher the value, the more reactive but also more aggressive
	#define SMOOTHING_MAX_STEP 15

	// Set maximum brightness - highest = 255
	const uint32_t LED_MAX_BRIGHTNESS = 32;

	// Communication to Raspberry
	#define SYNC_PREFIX = "SYNC";
	#define SYNC_POSTFIX = "\r\n";
	#define SYNC_PREFIX_LENGTH 4
	#define SYNC_POSTFIX_LENGTH 2

/*** Debug Modes ***/

	// use simulated input
//	#define MODE_SIMULATION 					// simulate input

	// echo Soft/Hardware Serial to HardwareSerial
	#ifndef MODE_SIMULATION
	#define MODE_ECHO
	#endif


/*** Communication ***/

	#define 	COMM_HW_BAUDRATE 	115200
	#define 	COMM_SW_BAUDRATE 	38400

	// TODO if raspberry dann softserial in code zurückändern!
	SoftwareSerial 	softSerial(COMM_SW_RX_PIN, COMM_SW_TX_PIN);

/*** LEDs ***/

	const uint32_t 	LED_LEFT_CHANNELS 	= 	LED_LEFT_COUNT * 3;

	// create color structures
	CRGB leds_left[LED_LEFT_COUNT];

	//CRGB leds_right[LEFT_LED_COUNT];

/*** Functions ***/
	void fastLoop();
	void UpdateStrip();
	void limitChange();

	// dummy loop fkt. for arduino. Using fastLoop (for(;;)) instead for better performance
	void loop() {}



#endif /* _AmbilinoPi_H_ */
