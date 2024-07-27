#include "LoopProfiler.h"
#include <elapsedMillis.h>

// if PROFILE is not defined, all other profile code is compiled out.
#define PROFILE

// Example function with varying execution time:
void foo() {
    delay(random(1,200)); 
}

// Example function with constant execution time:
void bar() {
    delay(150); 
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

		PROFILE_SETUP(); // TODO: necessary?
}

void loop() { 
	static elapsedMillis reportTimer = 0;

	PROFILE_START_LOOP();

	PROFILE_MARK_START("foo");
	foo();
	PROFILE_MARK_END("foo");

	PROFILE_MARK_START("bar");
	bar();
	PROFILE_MARK_END("bar");

	// report the profile results every 1000ms:
	if (reportTimer > 1000){ // TODO: move timer into LoopProfiler ...
		PROFILE_PRINT_AVG();
		reportTimer -= 1000;
	}
}
