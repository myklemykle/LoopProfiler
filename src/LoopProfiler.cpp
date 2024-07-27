#include "LoopProfiler.h"
#include "config.h"

#define strMatch(a, b) ( strcmp(a, b) == 0 )
#define cpName(a,b) strncpy(a, b, PROFILE_POINTNAMELEN -1)

void LoopProfiler::init(){
	// zero stuff:
	for (int i = 0; i < PROFILE_CPS; i++){
		checkpoints[i].sampleStart = 0;
		checkpoints[i].sampleCount = 0;
		checkpoints[i].sampleLen = 0;
		checkpoints[i].averageLen = 0;
		cpName(checkpoints[i].name, "");
	}
	pointCursor = 0;
}

void LoopProfiler::startLoop(){
	if (pointCursor > 0)
		markEnd("ALL");

	markStart("ALL");
}

// CS Majors: how should I optimize the decision about whether or not to optimize
// a search through no more than PROFILE_CPS items?  In terms of my own
// time vs. search time vs. memory vs. etc?  I don't have a go-to hash/dict
// implementation for Arduino, so linear search for now ...

int LoopProfiler::findPointByName(const char *pName){

	for (int i = 0; i < PROFILE_CPS; i++){
		if (strMatch(checkpoints[i].name, pName))
			return i;
	}
	
	// if not found,
	return -1;
}

void LoopProfiler::markStart(const char *pName){
	unsigned long now_us = micros();

	// find by name
	int i = findPointByName(pName);
	if (i < 0) {
		if (pointCursor == PROFILE_CPS) {
			Dbg_println("loopProfiler: only PROFILE_CPS checkpoints allowed");
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
}

void LoopProfiler::markEnd(const char *pName){
	unsigned long now_us = micros();

	// find by name
	int i = findPointByName(pName);
	if (i < 0) {
		Dbg_printf("loopProfiler: error: closing unopened checkpoint '%s'\n", pName);
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
}

// say deep things about the profiling (for this loop):
void LoopProfiler::printRaw(){
	Dbg_print("raw: ");
	for (int i=0;i<pointCursor;i++){
		Dbg_printf("%s=%d ",checkpoints[i].name, checkpoints[i].sampleLen);
	}
	Dbg_println("");
}

void LoopProfiler::printAverage(){
	LoopProfileCheckpoint *cp;

	Dbg_print("avg: ");
	for (int i=0;i<pointCursor;i++){
		cp = &(checkpoints[i]);
		Dbg_printf("%s=%.2f ",cp->name, cp->averageLen);
	}
	Dbg_println();
}

