#ifndef __LOOPPROFILER_H
#define __LOOPPROFILER_H

#ifdef PROFILE
////////////////////
/// Loop Profiler
////////////////////


#include <string.h>
#define strMatch(a, b) ( strcmp(a, b) == 0 )
#define cpName(a,b) strncpy(a, b, PROFILE_LABEL_LENGTH -1)


// max length of a checkpoint label
#ifndef PROFILE_LABEL_LENGTH
#define PROFILE_LABEL_LENGTH 20 
#endif

// max number of checkpoints
#ifndef PROFILE_CHECKPOINTS
#define PROFILE_CHECKPOINTS 20  				
#endif

// max number of samples to average over
#ifndef PROFILE_AVGOVER
#define PROFILE_AVGOVER 100 		
#endif


// This object stores timing samples and a running average.
typedef struct {
	char name[PROFILE_LABEL_LENGTH] = "";
	unsigned long sampleStart = 0;
	unsigned int sampleCount = 0;
	unsigned long sampleLen = 0;
	double averageLen = 0;
} LoopProfileCheckpoint;

class LoopProfiler {
	private:
		LoopProfileCheckpoint checkpoints[PROFILE_CHECKPOINTS]; 
		int pointCursor = 0;

		int findPointByName(const char *pName){
			// Linear array search.
			// TODO: std::unordered_map option instead? Internet thinks it would be faster after about ten items.
			
			for (int i = 0; i < PROFILE_CHECKPOINTS; i++){
				if (strMatch(checkpoints[i].name, pName))
					return i;
			}
			
			// if not found,
			return -1;
		}


	public:

		void reset(){
			// zero out counters
			for (int i = 0; i < PROFILE_CHECKPOINTS; i++){
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
				markEnd("LOOP");

			markStart("LOOP");
		};


		void markStart(const char *pName){
			unsigned long now_us = micros();

			// find by name
			int i = findPointByName(pName);
			if (i < 0) {
				if (pointCursor == PROFILE_CHECKPOINTS) {
					Serial.println("loopProfiler: only PROFILE_CHECKPOINTS checkpoints allowed");
					return;
				}
				// new point!
				i = pointCursor;
				cpName(checkpoints[i].name, pName);

				if (pointCursor < PROFILE_CHECKPOINTS)
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


		void printRaw(Stream &s){
			s.print("raw: ");
			for (int i=0;i<pointCursor;i++){
				s.printf("%s=%d ",checkpoints[i].name, checkpoints[i].sampleLen);
			}
			s.println("");
		};


		void printAverage(Stream &s){
			LoopProfileCheckpoint *cp;

			s.print("avg usec: ");
			for (int i=0;i<pointCursor;i++){
				cp = &(checkpoints[i]);
				s.printf("%s=%.2f ",cp->name, cp->averageLen);
			}
			s.println();
		};
};


// single global instance of the object:
static LoopProfiler __profile; // why static?

// handy macros:
#define PROFILE_RESET()   		__profile.reset()
#define PROFILE_LOOP()  			__profile.startLoop()
#define PROFILE_START(_label)  __profile.markStart(_label)
#define PROFILE_END(_label)  	__profile.markEnd(_label)
#define PROFILE_PRINT_RAW(_stream)  	__profile.printRaw(_stream)
#define PROFILE_PRINT_AVG(_stream)  	__profile.printAverage(_stream)

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
