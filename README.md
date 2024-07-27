# Arduino Loop Profiler

Profile minimum/maximum/average execution time through looping code sections.

There are several profiling libraries for Arduino. This one has these features:
* Completely strip all profiling code from your binary with one `#undef`
* Report minimum, maximum and rolling averages over a configurable window.
* Measure time in milliseconds, microseconds or your own custom time units.
* Reports to any Stream (including Serial) at a configurable interval.
* Profiled sections may intersect any way you like.



Usage example:

~~~cpp
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
	PROFILE_LOOP(); 

	PROFILE_START("delayRnd");
	delayRnd();
	PROFILE_END("delayRnd");

	PROFILE_START("delay150");
	delay150();
	PROFILE_END("delay150");

	PROFILE_PRINT_AVG(Serial); 
}
~~~

See `examples/complete.ino` for examples of all documented options.
