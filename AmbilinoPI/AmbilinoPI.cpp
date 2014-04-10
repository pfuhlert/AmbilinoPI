// Do not remove the include below
#include "AmbilinoPi.h"

void printFrame(uint8_t* buffer, uint8_t length) {

	for (int i = 1; i <= length; i++) {
		Serial.print("0x");
		Serial.print(buffer[i], HEX);
		Serial.print(" ");
		if (i % 10 == 0) {
			Serial.println();
		}
	}
	Serial.println();
}
void printCRGBln(CRGB color) {
	Serial.print(color.r, HEX);
	Serial.print(" ");
	Serial.print(color.g, HEX);
	Serial.print(" ");
	Serial.println(color.b, HEX);
}
void printIntColor(int intColor[3]) {
	CRGB color;
	color.r = intColor[0];
	color.g = intColor[1];
	color.b = intColor[2];
	printCRGBln(color);
}


// limitChange() : limit max change according to sign
bool clipStep(int* value) {

	bool untouched = false;

	if (*value > SMOOTHING_MAX_STEP) {
		*value = SMOOTHING_MAX_STEP;
	} else if (*value < -SMOOTHING_MAX_STEP) {
		*value = -SMOOTHING_MAX_STEP;
	} else {
		untouched = true;
	}

	return untouched;
}

void printScanValues() {
	for(int i=0; i<1; i++) {

			Serial.print(i);
			Serial.print(":");
			for(int color=0; color<3; color++) {

				Serial.print(scanValues[i][color], HEX);
				Serial.print(" ");
			}
			Serial.println();
		}
		Serial.println();
}

void printStripes() {

	Serial.println("left:");
	for(int i=0; i<LED_LEFT_COUNT; i++) {
		Serial.print(i);
		Serial.print(": ");
		printCRGBln(leds_left[i]);

	}
	Serial.println();
}

// shows real led values inside currValues in LED strips
void updateLEDs() {

	for(int i=0; i<LED_COUNT; i++) {

		for(int color=0; color<3; color++) {

			if(i < LED_LEFT_COUNT) {
				leds_left[i][color] = scanValues[i][color];
			} else if(i < LED_LEFT_COUNT + LED_RIGHT_COUNT) {
				leds_right[i - LED_LEFT_COUNT][color] = scanValues[i][color];
			} else if(i < LED_LEFT_COUNT + LED_RIGHT_COUNT + LED_TOP_COUNT) {
				leds_top[i - LED_LEFT_COUNT - LED_RIGHT_COUNT][color] = scanValues[i][color];
			}

		}
	}

	LEDS.show();

}

// new values should be in "scanValues" - current LED values in "currValues"
void filterValues() {
	bool finished = true;

	uint32_t updateStripTime = millis();

	do {

		for (int i = 0; i < LED_COUNT; i++) {
			for (int color = 0; color < 3; color++) {
				diffValues[i][color] = scanValues[i][color]
						- currValues[i][color];
				if (diffValues[i][color] != 0) {

					bool untouched = clipStep(&diffValues[i][color]);

					if (untouched == false && finished == true) {
						finished = false;
					}
				}

				currValues[i][color] = currValues[i][color] + diffValues[i][color];
			}
		}

		delay(SMOOTHING_DELAY);

	} while (((millis() - updateStripTime) < SMOOTHING_MAX_DELAY) && !finished);
}

// writes new line in scanValues array
int ledNo;
void getNewScan() {

	byte inByte;

	while(softSerial.find(COMM_SYNC_PREFIX) != true) {
	}

	while (softSerial.available() < COMM_FRAMESIZE) {
//		Serial.println("waiting... ");
	}

	for (int i = 0; i < LED_CHANNELS; i++) {

		inByte = softSerial.read();
		scanValues[i/3][i%3] = (int) inByte;
	}

}

void setup() {

	// connect stripes to LED struct
	LEDS.addLeds<WS2811, LED_LEFT_PIN, 	GRB>(leds_left, LED_LEFT_COUNT); //initializes the left LED-stripe
	LEDS.addLeds<WS2811, LED_RIGHT_PIN, GRB>(leds_right, LED_RIGHT_COUNT); //initializes the left LED-stripe
	LEDS.addLeds<WS2811, LED_TOP_PIN, 	GRB>(leds_top, LED_TOP_COUNT); //initializes the left LED-stripe

	// set max brightness
	LEDS.setBrightness(LED_MAX_BRIGHTNESS);

	// set up arduino LED on pin 13
	pinMode(13, OUTPUT);

	// Short Test
	LEDS.showColor(CRGB::Red);
	delay(500);
	LEDS.showColor(CRGB::Green);
	delay(500);
	LEDS.showColor(CRGB::Blue);
	delay(500);
	LEDS.showColor(CRGB::Black);

	// Debugging Serial
	Serial.begin(COMM_HW_BAUDRATE);

	// Software Serial
	softSerial.begin(COMM_SW_BAUDRATE);

	// Wait for Serials to set up.
	while (!Serial) {}

//	inputString.reserve(LED_CHANNELS);

	// avoid loop() function
	fastLoop();

}

// start of main loop
void fastLoop() {

	uint32_t fastLoopTime = millis();

	for (;;) {

		fastLoopTime = millis();

		getNewScan();
		filterValues();
		updateLEDs();

		while(millis() - fastLoopTime < CAPTURE_DELAY_MS) {
			// wait
		}

		// clear overhead data
		softSerial.flush();

	}
}
