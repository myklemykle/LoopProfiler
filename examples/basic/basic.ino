#include <elapsedMillis.h>

#ifdef USE_TINYUSB
#include <Adafruit_TinyUSB.h>
#endif

// Define PROFILE before LoopProfiler.h is included to include profiling in the binary.
#define PROFILE 


// Some options (must also be defined before loopProfiler.h is included):
// Restrict reports to once every 1000ms:
#define PROFILE_AUTOPRINT_MS 	1000

// Measure time in microseconds instead of the default milliseconds
#define PROFILE_MICROS

// Maximum number of checkpoints we can record (not counting the main loop total)
// Default is 20.
#define PROFILE_CHECKPOINTS 	4

// Maximum length of a checkpoint label. Default is 20 chars. 
#define PROFILE_LABEL_LENGTH 	10

#define IAMTHEDEVELOPER
#ifdef IAMTHEDEVELOPER
#include "LoopProfiler.h" // I do this while developing the library.
#else
#include <LoopProfiler.h> // You do this after installing the library.
#endif


// Example function with varying execution time:
void delayRnd() {
    delay(random(1,200));   
}

// Example function with constant execution time:
void delay150() {
    delay(150); 						
}

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(115200);
	while (!Serial);
}


void loop() { 
	// Place this at the top of the main loop:
	PROFILE_LOOP(); 

	static elapsedMillis blinkTimer = 0;

	// This section encompasses both delay calls, so will average to 250ms + overhead
	PROFILE_START("bothDelays");

	// This section should slowly average to 100ms + calling overhead
	PROFILE_START("delayRnd");
	delayRnd();
	PROFILE_END("delayRnd");

	// This section should stay close to 150ms, plus calling overhead
	PROFILE_START("delay150");
	delay150();
	PROFILE_END("delay150");

	PROFILE_END("bothDelays");


	// This section's average time (per loop) will depend on the speed of our main loop, i.e. MCU clock speed.
	PROFILE_START("blink");
	if (blinkTimer > 1000){ 
		// Blink every second so we know the sketch is running.
		blinkTimer -= 1000;

		digitalWrite(LED_BUILTIN, HIGH);
		delay(200);
		digitalWrite(LED_BUILTIN, LOW);
	}
	PROFILE_END("blink");


	// PROFILE_PRINT_* macros print only once per PROFILE_AUTOPRINT_MS (when PROFILE_AUTOPRINT_MS is defined).
	PROFILE_PRINT_MAX(Serial); 
	PROFILE_PRINT_AVG(Serial); 
	PROFILE_PRINT_MIN(Serial);
}
