/*****************************************************************************
MIT License

Copyright (c) 2016 Douglas Adler

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*****************************************************************************/

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

