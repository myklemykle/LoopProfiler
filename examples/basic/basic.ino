#include <elapsedMillis.h>

#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
#endif

#define PROFILE 
// if PROFILE is not defined before LoopProfiler.h is included, all profile code is removed from binary.
#include "LoopProfiler.h"

// Example function with varying execution time:
void foo() {
    delay(random(1,200)); 
}

// Example function with constant execution time:
void bar() {
    delay(150); 
}

void setup() {
	  pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial);
}

void loop() { 
	PROFILE_LOOP(); 

	static elapsedMillis reportTimer = 0;


	PROFILE_START("foo");
	foo();
	PROFILE_END("foo");

	PROFILE_START("bar");
	bar();
	PROFILE_END("bar");

	// report the profile results every 1000ms:
	if (reportTimer > 1000){ // TODO: move timer into LoopProfiler ...
		reportTimer -= 1000;

		digitalWrite(LED_BUILTIN, HIGH);
		PROFILE_PRINT_AVG(Serial);
		delay(200);
		digitalWrite(LED_BUILTIN, LOW);

	}
}
