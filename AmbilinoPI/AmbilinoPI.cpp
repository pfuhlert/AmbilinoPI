// Do not remove the include below
#include "AmbilinoPi.h"

// Initialize global timer for  printTimeSinceLastCall() function
uint32_t time = micros();
void printTimeSinceLastCall(String caption) {
	time = micros() - time;
	Serial.print(caption);
	time = time / 1000;
	Serial.print(time);
	Serial.println("ms.");
	time = micros();
}

void printFrame(uint8_t* buffer, uint8_t length) {

	for(int i=1;i<=length;i++) {
		Serial.print("0x");
		Serial.print(buffer[i],HEX);
		Serial.print(" ");
		if(i%10 == 0) {
			Serial.println();
		}
	}
	Serial.println();
}

void printCRGB(CRGB color) {
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
	printCRGB(color);
}

CRGB currValues[LED_COUNT];
CRGB scanValues[LED_COUNT];
int diffValues[LED_COUNT][3];

String inputString;
bool stringComplete;

void setup() {

	// TODO Reset LEDs on startup - not possible with LEDS.showcolor -> nothing known of stripelength

	// fill stripes with zeroes
//	memset(leds_left, 	0, LED_LEFT_COUNT * 3);
//	memset(leds_right, 0, LED_RIGHT_COUNT * 3);
//	memset(leds_top, 	0, LED_TOP_COUNT * 3);


	LEDS.addLeds<WS2811, LED_LEFT_PIN	, GRB>(leds_left, 	LED_LEFT_COUNT); //initializes the left LED-stripe
	LEDS.addLeds<WS2811, LED_RIGHT_PIN	, GRB>(leds_right, 	LED_RIGHT_COUNT); //initializes the left LED-stripe
	LEDS.addLeds<WS2811, LED_TOP_PIN	, GRB>(leds_top, 	LED_TOP_COUNT); //initializes the left LED-stripe

	// HACK for resetting LEDs
//	LEDS.setBrightness(255);
//	LEDS.showColor(CRGB::Black);
//	while(1){}

	LEDS.setBrightness(LED_MAX_BRIGHTNESS);

	// Short Test
	LEDS.showColor(CRGB::White);
	delay(300);
	LEDS.showColor(CRGB::Black);

	// Debugging Serial
	Serial.begin(COMM_HW_BAUDRATE);

	// Software Serial, TODO use Hardware Serial instead?
	softSerial.begin(COMM_SW_BAUDRATE);

	// Wait for Serials to set up.
	while (!Serial) {}

	// avoid loop() function
	fastLoop();
}

// limitChange() : limit max change according to sign
int limitChange(int* value) {

	int untouched = false;

	if(*value > SMOOTHING_MAX_STEP) {
		*value = SMOOTHING_MAX_STEP;
	} else if(*value < -SMOOTHING_MAX_STEP) {
		*value = -SMOOTHING_MAX_STEP;
	} else {
		untouched = true;
	}
	return untouched;
}


// new values should be in "scanvalues" - current LED values in currValues
void UpdateStrip() {
	uint32_t updateTime = millis();
	bool finished = true;

	do {
		for (int i = 0; i < LED_COUNT; i++) {
			for(int color=0; color<3; color++) {
				diffValues[i][color] = scanValues[i][color] - currValues[i][color];
					if(diffValues[i][color] != 0) {
							bool untouched = limitChange(&diffValues[i][color]);
							if(untouched == false) {
								finished = false;
							}
					}
			}
		}

		// Limit to maxchange
		for (int i = 0; i < LED_COUNT; i++) {
			for(int color=0; color<3; color++) {
				currValues[i][color] = currValues[i][color] + diffValues[i][color];
			}

//			printCRGB(currValues[i]);

			int tempRightPosition = i-LED_LEFT_COUNT;
			int tempTopPosition = i - LED_LEFT_COUNT - LED_RIGHT_COUNT;

			// HACK ugly stripe decision. TODO better way?

			if(i<LED_LEFT_COUNT) {
				leds_left[i].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
			} else if(tempRightPosition >= 0 && tempRightPosition<LED_RIGHT_COUNT) {
				leds_right[tempRightPosition].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
			} else if (tempTopPosition >= 0 && tempTopPosition<LED_TOP_COUNT) {
				leds_top[tempTopPosition].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
			}
		}

//		leds_top[0].setColorCode(CRGB::Pink);

		// Update Strips
		LEDS.show();
		delay(SMOOTHING_DELAY);


	} while (((millis() - updateTime) < SMOOTHING_MAX_DELAY) && !finished);
}

void fastLoop() {

//			printTimeSinceLastCall("for(;;) loop: ");

	uint32_t updateTime = millis();

	// single fade in red -> teal -> green -> white -> blue -> white -> red ...
	#ifdef MODE_SIMULATION

		for (;;) {

			updateTime = millis();

			for(int j = 0; j < 3; j++) {
				for(int i = 0 ; i < LED_COUNT; i++ ) {
				  scanValues[i][j] = 255;
				  scanValues[i][(j+1)%3] = 0;
				  scanValues[i][(j+2)%3] = 0;
				  UpdateStrip();
				  delay(CAPTURE_DELAY_MS);
				}
				for(int i = LED_COUNT ; i >= 0; i-- ) {
					  scanValues[i][j] = 0;
					  scanValues[i][(j+1)%3] = 255;
					  scanValues[i][(j+2)%3] = 255;
				  UpdateStrip();
				  delay(CAPTURE_DELAY_MS);
				}
			  }

			while(millis() - updateTime < CAPTURE_DELAY_MS) {};

		}

	#else


		// start of main loop
		for (;;) {

			if(Serial.find(SYNC_PREFIX)) {

				// TODO anders organisieren?

				// manually limit buffer
				serialBuffer[108] = '\0';

				#ifdef MODE_ECHO
				#endif

				char inChar;
				if(Serial.find(SYNC_PREFIX)) {
					for(int i=0;i<COMM_FRAMESIZE-SYNC_PREFIX_LENGTH;i++) {
						while(!Serial.available()) {}
						inChar = Serial.read();
						inputString += inChar;
					}

					int i=0;
					for(int i=0;i<LED_LEFT_COUNT;i++) {
						for(int color=0;color<3;color++) {
							scanValues[i][color] = (byte) inputString[i*3 + color];
						}

						// Print read Colors of pixel i
//						Serial.print(i);
//						Serial.print(": ");
//						printCRGB(scanValues[i]);
					}


					// Update stripvalues
					UpdateStrip();

					inputString = "";
					while(millis() - updateTime < CAPTURE_DELAY_MS) {};

				} else {
					// timeout on find
					inputString = "";
				}


				// reset updatetime
				updateTime = millis();

			}
		}
	#endif // !MODE_SIMULATION
}




