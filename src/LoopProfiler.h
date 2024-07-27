#ifndef __LOOPPROFILER_H
#define __LOOPPROFILER_H

#ifdef PROFILE
////////////////////
/// Loop Profiler
////////////////////


#ifdef PROFILE_AUTOPRINT_MS
#include <elapsedMillis.h>
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
#define cpName(a,b) strncpy(a, b, __ACTUAL_PROFILE_LABEL_LENGTH) // why -1?


// Store timing samples and a running average
typedef struct {
	char name[__ACTUAL_PROFILE_LABEL_LENGTH] = "";
	unsigned long sampleStart = 0;
	unsigned int sampleCount = 0;
	unsigned long sampleLen = 0;
	double averageLen = 0;
	double maxLen = 0;
	double minLen = 0;
} LoopProfileCheckpoint;

class LoopProfiler {
	private:
		LoopProfileCheckpoint checkpoints[__ACTUAL_PROFILE_CHECKPOINTS]; 
		int cpCount = 0;

		int findPointByName(const char *pName){
			// Linear array search.
			// TODO: std::unordered_map option instead? Internet thinks it would be faster after about ten items.
			
			for (int i = 0; i < __ACTUAL_PROFILE_CHECKPOINTS; i++){
				if (strMatch(checkpoints[i].name, pName))
					return i;
			}
			
			// if not found,
			return -1;
		}


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
			for (int i = 0; i < __ACTUAL_PROFILE_CHECKPOINTS; i++){
				checkpoints[i].sampleStart = 0;
				checkpoints[i].sampleCount = 0;
				checkpoints[i].sampleLen = 0;
				checkpoints[i].averageLen = 0;
				checkpoints[i].maxLen = 0;
				checkpoints[i].minLen = 0;
				cpName(checkpoints[i].name, "");
			}
			cpCount = 0;
		};


		void startLoop(){
			if (cpCount > 0)
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
			unsigned long now = __loopprofilerTime();

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

			auto *cp = &(checkpoints[i]);
			// measure start of gap:
			cp->sampleStart = now;
		};


		void markEnd(const char *pName){
			unsigned long now = __loopprofilerTime();

			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
				Serial.printf("loopProfiler: error: closing unopened checkpoint '%s'\n", pName);
				return;
			}

			auto *cp = &(checkpoints[i]);

			// measure length of gap:
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
			for (int i=0;i<cpCount;i++){
				s.printf("%s=%d ",checkpoints[i].name, checkpoints[i].sampleLen);
			}
			s.println("");
		};


		void printAverage(Stream &s){
			s.print("avg " __timeLabel ": ");
			for (int i=0;i<cpCount;i++){
				auto cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->averageLen);
			}
			s.println();
		};

		void printMax(Stream &s){
			s.print("max " __timeLabel ": ");
			for (int i=0;i<cpCount;i++){
				auto cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->maxLen);
			}
			s.println();
		};

		void printMin(Stream &s){
			s.print("min " __timeLabel ": ");
			for (int i=0;i<cpCount;i++){
				auto cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->minLen);
			}
			s.println();
		};
};


// single global instance of the object:
LoopProfiler __loopprofiler; 

// handy macros:
#define PROFILE_RESET()   		__loopprofiler.reset()
#define PROFILE_LOOP()  			__loopprofiler.startLoop()
#define PROFILE_START(_label)  __loopprofiler.markStart(_label)
#define PROFILE_END(_label)  	__loopprofiler.markEnd(_label)
#define PROFILE_PRINT_RAW(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printRaw(_stream)
#define PROFILE_PRINT_AVG(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printAverage(_stream)
#define PROFILE_PRINT_MAX(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printMax(_stream)
#define PROFILE_PRINT_MIN(_stream)  	if (__loopprofiler.printNow) __loopprofiler.printMin(_stream)

#else // !PROFILE 
////////////////////
/// empty macros:
////////////////////

#define PROFILE_SETUP()   
#define PROFILE_LOOP()
#define PROFILE_START(_label)
#define PROFILE_END(_label)
#define PROFILE_PRINT_RAW(_stream)  
#define PROFILE_PRINT_AVG(_stream) 

#endif //PROFILE

#endif //__LOOPPROFILER_H
