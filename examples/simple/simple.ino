/*
 * Simple example of profiling with LoopProfiler
 */

#define PROFILE 
#define PROFILE_AUTOPRINT_MS 	1000
#include <LoopProfiler.h>


// Example function with varying execution time:
void delayRnd() {
    delay(random(1,200));   
}

// Example function with constant execution time:
void delay150() {
    delay(150); 						
}

void setup() {
	Serial.begin(115200);
	while (!Serial);
}


void loop() { 
	static elapsedMillis timer = 0;
	static int resetCounter = 0;

	PROFILE_LOOP(); 

	PROFILE_START("delayRnd");

	delayRnd();

	PROFILE_END("delayRnd");

	PROFILE_START("delay150");

	delay150();

	PROFILE_END("delay150");

	PROFILE_PRINT_AVG(Serial); 
}
