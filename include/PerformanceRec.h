#ifndef PERFRECORD_H_
#define PERFRECORD_H_

#include <pthread.h>
#include <sys/times.h>

#include "PerfMetrics.h"
#include "performance_id.h"
#include "PerfRecordReport.h"
#include "Node.h"

class PerformanceRec : public Node
{
public:
	PerformanceRec();
	virtual ~PerformanceRec();
	
	bool 		SetID(PerfID nID);
	bool 		SetCatID(PerfID nID);
	PerfID 		GetID();
	PerfID 		GetCatID();
	bool 		SetThreadID(pthread_t nThreadID);
	pthread_t 	GetThreadID();
	
	bool 		AddEntry();
	bool 		AddExit();
	bool 		GetReport(PerfRecordReport* report);
	uint64_t 	GetTotalTime();
	clock_t 	GetTotalCPUTime();
	uint32_t 	GetTotalSamples();
	
private:
	bool 		GetCurrentTimeStamp(uint64_t* pnTimeStamp);
	uint64_t 	GetChildTotalTime();
	clock_t		GetChildTotalTimeCPU();
	
	bool		mbFirstEntry;
	PerfID		mID;
	PerfID		mCatID;
	pthread_t	mThreadID;
	
	uint64_t	mCurrentEntryTime;
	uint64_t	mLastExitTime;
	uint64_t	mStartTime;
	uint64_t	mTotalTime;
	uint32_t	mMinTime;
	uint32_t	mMaxTime;
	clock_t		mEntryCPUTime;
	clock_t		mStartCPUTime;
	clock_t		mExitCPUTime;
	clock_t		mTotalCPUTime;
	clock_t		mMinCPUTime;
	clock_t		mMaxCPUTime;
	uint32_t	mTotalCalls;
	
};

#endif /*PERFRECORD_H_*/

