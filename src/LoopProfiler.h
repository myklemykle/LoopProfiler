#ifndef __LOOPPROFILER_H
#define __LOOPPROFILER_H

#ifdef PROFILE
////////////////////
/// Loop Profiler
////////////////////

#include <map>

#ifdef PROFILE_AUTOPRINT_MS
#include <elapsedMillis.h>
#endif

// Max number of samples to average over
#ifndef PROFILE_AVGOVER
#define PROFILE_AVGOVER 100 		
#endif

// Which units-function to use for timekeeping
#ifdef PROFILE_MICROS
#define __loopprofilerTime micros
#define __timeLabel "usec"
#else
#define __loopprofilerTime millis
#define __timeLabel "msec"
#endif


// Store timing samples and a running average
typedef struct {
	unsigned long sampleStart = 0;
	unsigned int 	sampleCount = 0;
	unsigned long sampleLen = 0;
	float 				averageLen = 0;
	float 				maxLen = 0;
	float 				minLen = 0;
	void reset(){
		sampleStart = 0;
		sampleCount = 0;
		sampleLen = 0;
		averageLen = 0;
		maxLen = 0;
		minLen = 0;
	};
} LoopProfileCheckpoint;

class LoopProfiler {
	private:
		std::map<const char *, LoopProfileCheckpoint> checkpoints;


	public:
#ifdef PROFILE_AUTOPRINT_MS
		bool printNow = false;
		elapsedMillis printTimer = 0;
		long prevPrintTimer = 0;
#else
		const bool printNow = true;
#endif


		void reset(){
			for (auto & [ key, value ] : checkpoints) {
				// reset counters, but preserve start time of any checkpoint in progress
				unsigned long sampleStart = value.sampleStart;
				value.reset();
				value.sampleStart = sampleStart;
			}
		};


		void startLoop(){
			if (checkpoints.count("LOOP") > 0)
				markEnd("LOOP");

			markStart("LOOP");

#ifdef PROFILE_AUTOPRINT_MS
			if (printTimer > PROFILE_AUTOPRINT_MS){
				printTimer -= PROFILE_AUTOPRINT_MS;
				printNow = true;
			} else {
				printNow = false;
			}
#endif
		};


		void markStart(const char *pName){
			checkpoints[pName].sampleStart = __loopprofilerTime(); // unorderd_map will create a new object if it doesn't exist
		};


		void markEnd(const char *pName){
			unsigned long now = __loopprofilerTime();

			if (checkpoints.count(pName) == 0){
				Serial.printf("loopProfiler: error: closing unopened checkpoint '%s'\n", pName);
				return;
			}

			auto *cp = &checkpoints[pName];

			// measure length of gap
			// (unsigned math should handle int overflows)
			cp->sampleLen = now - cp->sampleStart;

			if (cp->sampleLen > cp->maxLen)
				cp->maxLen = cp->sampleLen;

			if ( (cp->minLen == 0) || (cp->sampleLen < cp->minLen) )
				cp->minLen = cp->sampleLen;

			// calculate avg length
			cp->averageLen =
				((cp->averageLen * cp->sampleCount) + cp->sampleLen) / (float)(cp->sampleCount + 1);

			if (cp->sampleCount < PROFILE_AVGOVER)
				cp->sampleCount++;
		};


		void printRaw(Stream &s){
			s.print("raw: ");
			for (const auto & [ key, value ] : checkpoints) {
				s.printf("%s=%d ", key, value.sampleLen);
			}
			s.println("");
		};


		void printAverage(Stream &s){
			s.print("avg " __timeLabel ": ");
			for (const auto & [ key, value ] : checkpoints) {
				s.printf("%s=%.2f ", key, value.averageLen);
			}
			s.println();
		};

		void printMax(Stream &s){
			s.print("max " __timeLabel ": ");
			for (const auto & [ key, value ] : checkpoints) {
				s.printf("%s=%.2f ", key, value.maxLen);
			}
			s.println();
		};

		void printMin(Stream &s){
			s.print("min " __timeLabel ": ");
			for (const auto & [ key, value ] : checkpoints) {
				s.printf("%s=%.2f ", key, value.minLen);
			}
			s.println();
		};

		void printRAM(Stream &s){
#if defined(ARDUINO_ARCH_RP2040) && !defined(__MBED__)
			// Implementation for arduino-pico core:
			s.print("Heap RAM used: ");
			s.print(rp2040.getUsedHeap());
			s.println(" bytes");
#else
			// TODO: MBED, AVR, Teensy, ESP32 ...
			// Pull requests appreciated!
			s.println("printRAM not implemented on this MCU");
#endif
		};

};

// single global instance of the object:
LoopProfiler __loopprofiler; 

// handy macros:
#define PROFILE_RESET()				__loopprofiler.reset()
#define PROFILE_LOOP()  			__loopprofiler.startLoop()
#define PROFILE_START(_label)  __loopprofiler.markStart(_label)
#define PROFILE_END(_label)  	__loopprofiler.markEnd(_label)
#define PROFILE_PRINT_RAW(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printRaw(_stream)
#define PROFILE_PRINT_AVG(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printAverage(_stream)
#define PROFILE_PRINT_MAX(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printMax(_stream)
#define PROFILE_PRINT_MIN(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printMin(_stream)
#define PROFILE_PRINT_RAM(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printRAM(_stream)

#else // !PROFILE 
////////////////////
/// empty macros:
////////////////////

#define PROFILE_RESET()
#define PROFILE_LOOP()
#define PROFILE_START(_label)
#define PROFILE_END(_label)
#define PROFILE_PRINT_RAW(_stream)  
#define PROFILE_PRINT_AVG(_stream) 
#define PROFILE_PRINT_MAX(_stream)
#define PROFILE_PRINT_MIN(_stream)
#define PROFILE_PRINT_RAM(_stream)

#endif //PROFILE

#endif //__LOOPPROFILER_H
