// Do not remove the include below
#include "AmbilinoPi.h"

uint32_t time;
void printTimeSinceLastCall(String caption) {
	time = micros() - time;
	Serial.print(caption);
	time = time / 1000;
	Serial.print(time);
	Serial.println("ms.");
	time = micros();
}

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
				for(int upscaler=0; upscaler<LED_UPSCALE;upscaler++) {
					leds_left[LED_UPSCALE*i + upscaler][color] = currValues[i][color];
				}
			} else if(i < LED_LEFT_COUNT + LED_RIGHT_COUNT) {
				for(int upscaler=0; upscaler<LED_UPSCALE;upscaler++) {
					leds_right[LED_UPSCALE * (i - LED_LEFT_COUNT) + upscaler][color] = currValues[i][color];
				}
			} else if(i < LED_LEFT_COUNT + LED_RIGHT_COUNT + LED_TOP_COUNT) {
				for(int upscaler=0; upscaler<LED_UPSCALE;upscaler++) {
					leds_top[LED_UPSCALE * (i - LED_LEFT_COUNT - LED_RIGHT_COUNT) + upscaler][color] = currValues[i][color];
				}
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

//		while(Serial.find(COMM_SYNC_PREFIX) != true) {
//			Serial.write("find fail!");
//		}

	if(softSerial.find(COMM_SYNC_PREFIX)) {
		digitalWrite(led_pin, HIGH);
	} else {
		digitalWrite(led_pin, LOW);
	}

	while (softSerial.available() == COMM_FRAMESIZE) {

		for (int i = 0; i < LED_CHANNELS; i++) {
			inByte = softSerial.read();
			scanValues[i/3][i%3] = (int) inByte;
		}

//		printFrame((uint8_t*) scanValues, LED_CHANNELS);
	}

}

void setup() {

	// connect stripes to LED struct
	//initializes the LED-stripes
	LEDS.addLeds<WS2811, LED_LEFT_PIN, 	GRB>(leds_left, LED_LEFT_COUNT * LED_UPSCALE);
	LEDS.addLeds<WS2811, LED_RIGHT_PIN, GRB>(leds_right, LED_RIGHT_COUNT * LED_UPSCALE);
	LEDS.addLeds<WS2811, LED_TOP_PIN, 	GRB>(leds_top, LED_TOP_COUNT * LED_UPSCALE);

	// set max brightness
	LEDS.setBrightness(LED_MAX_BRIGHTNESS);

	// set up LED on pin 13
	pinMode(led_pin, OUTPUT);

	// Debugging Serial
	Serial.begin(COMM_HW_BAUDRATE);

	// Software Serial
	softSerial.begin(COMM_SW_BAUDRATE);

	// Wait for Serials to set up.
	while (!Serial) {}

	// clear serial buffers
	Serial.flush();
	softSerial.flush();

	// Set the used input stream to according serial
	#ifdef COMM_SW_INPUT
		myStream = &softSerial;
	#else
		myStream = &Serial;
	#endif

	// Short Test
	LEDS.showColor(CRGB::Red);
	delay(1000);
	LEDS.showColor(CRGB::Green);
	delay(1000);
	LEDS.showColor(CRGB::Blue);
	delay(1000);
	LEDS.showColor(CRGB::Black);

	// avoid loop() function
	fastLoop();

}

// start of main loop
void fastLoop() {

#ifdef MODE_ECHO
	for(;;) {
		if(softSerial.available()) {
			Serial.write(softSerial.read());
		}
		if(Serial.available()) {
			Serial.write(Serial.read());
		}
	}
#endif

	unsigned long fastLoopTime = millis();
	uint32_t loopTimeShower;

	for (;;) {

		fastLoopTime = millis();

		getNewScan();
		filterValues();
		updateLEDs();

		// TODO timing problem with softSerial! not working!
		while(loopTimeShower < CAPTURE_DELAY_MS) {
			loopTimeShower = millis() - fastLoopTime;
		}


		// clear overhead data
		softSerial.flush();
		Serial.flush();

	}
}
