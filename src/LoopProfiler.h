#ifndef __LOOPPROFILER_H
#define __LOOPPROFILER_H

#ifdef PROFILE
////////////////////
/// Loop Profiler
////////////////////


#include <string.h>
#define strMatch(a, b) ( strcmp(a, b) == 0 )
#define cpName(a,b) strncpy(a, b, PROFILE_POINTNAMELEN -1)


// This object stores timing samples and a running average.
//
// This is statically allocated, so these two effect our mem footprint:
#define PROFILE_POINTNAMELEN 20 // how long can the label of a checkpoint be
#define PROFILE_CPS 20  				// max number of checkpoints
//
#define PROFILE_AVGOVER 100 		// max number of samples to average over

typedef struct {
	char name[PROFILE_POINTNAMELEN];
	unsigned long sampleStart;
	unsigned int sampleCount;
	unsigned long sampleLen;
	double averageLen;
} LoopProfileCheckpoint;

class LoopProfiler {
	private:
		LoopProfileCheckpoint checkpoints[PROFILE_CPS]; 
		int pointCursor, maxPoints;
		unsigned long loopStart_us;

		int findPointByName(const char *pName){
			// Linear array search.
			// TODO: std::unordered_map option instead? Internet thinks it would be faster after about ten items.
			
			for (int i = 0; i < PROFILE_CPS; i++){
				if (strMatch(checkpoints[i].name, pName))
					return i;
			}
			
			// if not found,
			return -1;
		}


	public:
		void init(){
			// zero out counters
			for (int i = 0; i < PROFILE_CPS; i++){
				checkpoints[i].sampleStart = 0;
				checkpoints[i].sampleCount = 0;
				checkpoints[i].sampleLen = 0;
				checkpoints[i].averageLen = 0;
				cpName(checkpoints[i].name, "");
			}
			pointCursor = 0;
		};


		void startLoop(){
			if (pointCursor > 0)
				markEnd("ALL");

			markStart("ALL");
		};


		void markStart(const char *pName){
			unsigned long now_us = micros();

			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
				if (pointCursor == PROFILE_CPS) {
					Serial.println("loopProfiler: only PROFILE_CPS checkpoints allowed");
					return;
				}
				// new point!
				i = pointCursor;
				cpName(checkpoints[i].name, pName);

				if (pointCursor < PROFILE_CPS)
					pointCursor++;
				// else
					// we're run out of room;
					// the next (overflowing) markStart() call will overwrite this point cuz we did not increment pointCursor
			}

			LoopProfileCheckpoint *cp = &(checkpoints[i]);
			// measure start of gap:
			cp->sampleStart = now_us;
		};


		void markEnd(const char *pName){
			unsigned long now_us = micros();

			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
				Serial.printf("loopProfiler: error: closing unopened checkpoint '%s'\n", pName);
				return;
			}

			LoopProfileCheckpoint *cp = &(checkpoints[i]);
			// measure length of gap:
			cp->sampleLen = now_us - cp->sampleStart;

			// calculate avg length
			cp->averageLen =
				((cp->averageLen * cp->sampleCount) + cp->sampleLen) / (float)(cp->sampleCount + 1);

			if (cp->sampleCount < PROFILE_AVGOVER)
				cp->sampleCount++;
		};


		void printRaw(){
			Serial.print("raw: ");
			for (int i=0;i<pointCursor;i++){
				Serial.printf("%s=%d ",checkpoints[i].name, checkpoints[i].sampleLen);
			}
			Serial.println("");
		};


		void printAverage(){
			LoopProfileCheckpoint *cp;

			Serial.print("avg usec: ");
			for (int i=0;i<pointCursor;i++){
				cp = &(checkpoints[i]);
				Serial.printf("%s=%.2f ",cp->name, cp->averageLen);
			}
			Serial.println();
		};
};


// single global instance of the object:
static LoopProfiler __profile; // why static?

// handy macros:
#define PROFILE_SETUP()   		__profile.init()
#define PROFILE_START_LOOP()  __profile.startLoop()
#define PROFILE_MARK_START(label)  __profile.markStart(label)
#define PROFILE_MARK_END(label)  __profile.markEnd(label)
#define PROFILE_PRINT_RAW()  __profile.printRaw()
#define PROFILE_PRINT_AVG()  __profile.printAverage()

#else // !PROFILE 
////////////////////
/// empty macros:
////////////////////

#define PROFILE_SETUP()   
#define PROFILE_START_LOOP()
#define PROFILE_MARK_START(label)
#define PROFILE_MARK_END(label)
#define PROFILE_PRINT_RAW()  
#define PROFILE_PRINT_AVG() 

#endif //PROFILE

#endif //__LOOPPROFILER_H
