#ifndef __LOOPPROFILER_H
#define __LOOPPROFILER_H

#ifdef PROFILE
////////////////////
/// Loop Profiler
////////////////////


#ifdef PROFILE_AUTOPRINT_MS
#include <elapsedMillis.h>
#endif

#ifndef PROFILE_NOMAP
#define PROFILE_MAP
#endif

#ifdef PROFILE_MAP
#include <unordered_map>
#endif

// Max length of a checkpoint label
#ifndef PROFILE_LABEL_LENGTH
#define PROFILE_LABEL_LENGTH 20 
#endif
#define __ACTUAL_PROFILE_LABEL_LENGTH (PROFILE_LABEL_LENGTH + 1)  // because the string is null-terminated.

// Max number of checkpoints
#ifndef PROFILE_CHECKPOINTS
#define PROFILE_CHECKPOINTS 20  				
#endif
#define __ACTUAL_PROFILE_CHECKPOINTS (PROFILE_CHECKPOINTS + 1) // because we also use one for timing the main loop.

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


// handy macros:
#include <string.h>
#define strMatch(a, b) ( strcmp(a, b) == 0 )
#define cpName(a,b) strncpy(a, b, __ACTUAL_PROFILE_LABEL_LENGTH) 


// Store timing samples and a running average
typedef struct {
#ifndef PROFILE_MAP
	char name[__ACTUAL_PROFILE_LABEL_LENGTH] = "";
#endif
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
#ifdef PROFILE_MAP
		std::unordered_map<const char *, LoopProfileCheckpoint> checkpoints;
#else
		LoopProfileCheckpoint checkpoints[__ACTUAL_PROFILE_CHECKPOINTS]; 

		int cpCount = 0;

		int findPointByName(const char *pName){
			// Linear array search
			for (int i = 0; i < __ACTUAL_PROFILE_CHECKPOINTS; i++){
				if (strMatch(checkpoints[i].name, pName))
					return i;
			}
			
			// if not found,
			return -1;
		}
#endif


	public:
#ifdef PROFILE_AUTOPRINT_MS
		bool printNow = false;
		elapsedMillis printTimer = 0;
		long prevPrintTimer = 0;
#else
		const bool printNow = true;
#endif


		void reset(){
			// zero out counters
#ifdef PROFILE_MAP
			for (auto & [ key, value ] : checkpoints) {
				auto cp = value;
#else
			for (int i = 0; i < __ACTUAL_PROFILE_CHECKPOINTS; i++){
				auto cp = checkpoints[i];
#endif
				unsigned long sampleStart = cp.sampleStart;
				cp.reset();
				cp.sampleStart = sampleStart;
			}
		};


		void startLoop(){
#ifdef PROFILE_MAP
			if (checkpoints.count("LOOP") > 0)
#else
			if (cpCount > 0)
#endif
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

#ifdef PROFILE_MAP
			checkpoints[pName].sampleStart = __loopprofilerTime(); // unorderd_map will create a new object if it doesn't exist
#else
			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
				if (cpCount == __ACTUAL_PROFILE_CHECKPOINTS) {
					Serial.printf("loopProfiler: skipping %s, only %d checkpoints allowed\n", pName, PROFILE_CHECKPOINTS);
					return;
				}
				// new point!
				i = cpCount;
				cpName(checkpoints[i].name, pName);

				if (cpCount < __ACTUAL_PROFILE_CHECKPOINTS)
					cpCount++;
			}

			checkpoints[i].sampleStart = __loopprofilerTime();
#endif
		};


		void markEnd(const char *pName){
			unsigned long now = __loopprofilerTime();

#ifdef PROFILE_MAP
			if (checkpoints.count(pName) == 0){
#else
			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
#endif
				Serial.printf("loopProfiler: error: closing unopened checkpoint '%s'\n", pName);
				return;
			}

#ifdef PROFILE_MAP
			auto *cp = &(checkpoints[pName]);
#else
			auto *cp = &(checkpoints[i]);
#endif

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
#ifdef PROFILE_MAP
			for (const auto & [ key, value ] : checkpoints) {
				auto *cp = &value;
				s.printf("%s=%d ", key, cp->sampleLen);
#else
			for (int i=0;i<cpCount;i++){
				auto *cp = &(checkpoints[i]);
				s.printf("%s=%d ",cp->name, cp->sampleLen);
#endif
			}
			s.println("");
		};


		void printAverage(Stream &s){
			s.print("avg " __timeLabel ": ");
#ifdef PROFILE_MAP
			for (const auto & [ key, value ] : checkpoints) {
				auto *cp = &value;
				s.printf("%s=%.2f ", key, cp->averageLen);
#else
			for (int i=0;i<cpCount;i++){
				auto *cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->averageLen);
#endif
			}
			s.println();
		};

		void printMax(Stream &s){
			s.print("max " __timeLabel ": ");
#ifdef PROFILE_MAP
			for (const auto & [ key, value ] : checkpoints) {
				auto *cp = &value;
				s.printf("%s=%.2f ", key, cp->maxLen);
#else
			for (int i=0;i<cpCount;i++){
				auto *cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->maxLen);
#endif
			}
			s.println();
		};

		void printMin(Stream &s){
			s.print("min " __timeLabel ": ");
#ifdef PROFILE_MAP
			for (const auto & [ key, value ] : checkpoints) {
				auto *cp = &value;
				s.printf("%s=%.2f ", key, cp->minLen);
#else
			for (int i=0;i<cpCount;i++){
				auto *cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->minLen);
#endif
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
			//TODO: MBED, AVR, Teensy, ESP32 ...
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
