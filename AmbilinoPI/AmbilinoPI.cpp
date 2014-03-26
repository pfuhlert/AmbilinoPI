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

CRGB currValues[LED_LEFT_COUNT];
CRGB scanValues[LED_LEFT_COUNT];
int diffValues[LED_LEFT_COUNT][3];

void setup() {

	// TODO Reset LEDs on startup - not possible with LEDS.showcolor -> nothing known of stripelength

	LEDS.addLeds<WS2811, LED_LEFT_PIN, GRB>(leds_left, LED_LEFT_COUNT); //initializes the left LED-stripe

	// HACK for resetting LEDs
//	LEDS.setBrightness(255);
//	LEDS.showColor(CRGB::Black);
//	while(1){}

	//	LEDS.showColor(CRGB(0, 0, 0));
	LEDS.setBrightness(LED_MAX_BRIGHTNESS);

	// Debugging Serial
	Serial.begin(COMM_HW_BAUDRATE);
	Serial.flush();

	// Rasberry Pi Serial, TODO use Hardware Serial instead?
	softSerial.begin(COMM_SW_BAUDRATE);
	softSerial.flush();

	// Wait for Serial to set up.
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
		for (int i = 0; i < LED_LEFT_COUNT; i++) {
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
		for (int i = 0; i < LED_LEFT_COUNT; i++) {
			for(int color=0; color<3; color++) {
				currValues[i][color] = currValues[i][color] + diffValues[i][color];
			}

			//printCRGB(currValues[i]);
			leds_left[i].setRGB(currValues[i].r, currValues[i].g, currValues[i].b);
		}

		// Update Strips
		LEDS.show();
		delay(SMOOTHING_DELAY);


	} while (((millis() - updateTime) < SMOOTHING_MAX_DELAY) && !finished);
}

void fastLoop() {

//			printTimeSinceLastCall("for(;;) loop: ");

	uint32_t updateTime = millis();

	#ifdef MODE_SIMULATION

		for (;;) {

			updateTime = millis();

			for(int j = 0; j < 3; j++) {
				memset(leds_left, 0, LED_LEFT_COUNT * 3);
				for(int i = 0 ; i < LED_LEFT_COUNT; i++ ) {
				  scanValues[i][0] = 255;
				  scanValues[i][1] = 0;
				  scanValues[i][2] = 0;
				  UpdateStrip();
				  delay(CAPTURE_DELAY_MS);
				}
				for(int i = LED_LEFT_COUNT ; i >= 0; i-- ) {
				  scanValues[i][0] = 255;
				  scanValues[i][1] = 255;
				  scanValues[i][2] = 255;
				  UpdateStrip();
				  delay(CAPTURE_DELAY_MS);
				}
			  }

			while(millis() - updateTime < CAPTURE_DELAY_MS) {};

		}
	#else

		// start of main loop
		for (;;) {

			if(Serial.find(SYNC_POSTFIX)) {

				// TODO anders organisieren?
				int i=0;
				for(int i=0;i<108;i++) {
					while(!Serial.available()) {}
					serialBuffer[i] = Serial.read();
				}

				serialBuffer[108] = '\0';

				#ifdef MODE_ECHO
					Serial.println((char*) serialBuffer);
				#endif

				// ########################################
				// TODO hier weitermachen! serialbuffer werte liegen nicht richtig!!!
				// ########################################

				for(int i=0;i<LED_LEFT_COUNT;i++) {
					for(int color=0;color<3;color++) {
						scanValues[i][color] = serialBuffer[i*3 + color + SYNC_PREFIX_LENGTH];
					}

					// Print read Colors of pixel i
//					Serial.print(i);
//					Serial.print(": ");
//					printCRGB(scanValues[i]);
				}


				// Update stripvalues
				UpdateStrip();

				while(millis() - updateTime < CAPTURE_DELAY_MS) {};

				// reset updatetime
				updateTime = millis();

			}
		}
	#endif // !MODE_SIMULATION
}




