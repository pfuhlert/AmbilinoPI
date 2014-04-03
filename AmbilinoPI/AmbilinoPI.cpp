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
	LEDS.showColor(CRGB::Red);
	delay(200);
	LEDS.showColor(CRGB::Green);
	delay(200);
	LEDS.showColor(CRGB::Blue);
	delay(200);
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


// new values should be in "scanValues" - current LED values in "currValues"
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

			int tempRightPosition = i - LED_LEFT_COUNT;
			int tempTopPosition =   i - LED_LEFT_COUNT - LED_RIGHT_COUNT;

			// HACK ugly stripe decision. TODO better way?
			if(i<LED_LEFT_COUNT) {
				if(LED_REVERSE_STRIPE[0]) {
					leds_left[LED_LEFT_COUNT - i - 1].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				} else {
					leds_left[i].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				}
			} else if(tempRightPosition >= 0 && tempRightPosition<LED_RIGHT_COUNT) {
				if(LED_REVERSE_STRIPE[1]) {
					leds_right[LED_RIGHT_COUNT - tempRightPosition - 1].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				} else {
					leds_right[tempRightPosition].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				}
			} else if (tempTopPosition >= 0 && tempTopPosition<LED_TOP_COUNT) {
				if(LED_REVERSE_STRIPE[2]) {
					leds_top[LED_TOP_COUNT - tempTopPosition - 1].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				} else {
					leds_top[tempTopPosition].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
				}
			}
		}

		// Physically map new values to LEDs
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
				  scanValues[i][j] 			= 255;
				  scanValues[i][(j+1)%3] 	= 0;
				  scanValues[i][(j+2)%3] 	= 0;
				  UpdateStrip();
				  delay(CAPTURE_DELAY_MS);
				}
//				for(int i = LED_COUNT ; i >= 0; i-- ) {
//					  scanValues[i][j] = 0;
//					  scanValues[i][(j+1)%3] = 255;
//					  scanValues[i][(j+2)%3] = 255;
//				  UpdateStrip();
//				  delay(CAPTURE_DELAY_MS);
//				}
			  }

			while(millis() - updateTime < CAPTURE_DELAY_MS) {};

		}
	#else


		// start of main loop
		char inChar;

		for (;;) {

//			Serial.println(softSerial.available());

			// TODO detection not working properly.. hier weitermachen!
			if(softSerial.find("SYNC")) {
				for(int i=0;i<LED_CHANNELS;i++) {

					while(!softSerial.available()) {}
					inChar = softSerial.read();
					inputString += inChar;
				}

				for(;;) {
					if(softSerial.available()) {
						Serial.write(softSerial.read());
					}
				}

				Serial.println(inputString.length());
				Serial.println(COMM_SW_BAUDRATE);

				// put items in buffer
				for(int i=0;i<LED_COUNT;i++) {
					for(int color=0;color<3;color++) {
						scanValues[i][color] = (byte) inputString[3*i + color];
					}
				}

				// Update stripvalues
				UpdateStrip();

				while(millis() - updateTime < CAPTURE_DELAY_MS) {};

			} else {
				Serial.println("timeout!");
				Serial.flush();
			}


			inputString = "\0";
			// reset updatetime
			updateTime = millis();
		}
	#endif // !MODE_SIMULATION
}




