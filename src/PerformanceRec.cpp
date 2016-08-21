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

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>		// for times() to get cpu ticks
#include <stdlib.h>
#include <unistd.h>
//#include <sys/sysconf.h>

#include <iostream>

#include "PerformanceRec.h"

#define USEC_PER_SEC	1000000
#define MAX_UINT32       0xFFFFFFFFul

PerformanceRec::PerformanceRec()
{
	mMaxTime			= 0;
	mMinTime			= MAX_UINT32;
	mStartTime			= 0;
	mTotalTime			= 0;
	mTotalCalls			= 0;
	mCurrentEntryTime	= 0;
	mLastExitTime		= 0;
	mEntryCPUTime		= 0;
	mStartCPUTime		= 0;
	mExitCPUTime		= 0;
	mTotalCPUTime		= 0;
	mMinCPUTime			= 10000;
	mMaxCPUTime			= 0.0;
	
	mID 				= INVALID_PERF_ID;
	mCatID				= INVALID_PERF_ID;
	mThreadID			= (pthread_t)-1;
	
	mbFirstEntry		= true;
}

PerformanceRec::~PerformanceRec()
{
}

bool PerformanceRec::GetCurrentTimeStamp(uint64_t* pnTimeStamp)
{
	struct timeval 	timeStamp;

	gettimeofday(&timeStamp, NULL);
	
	// Convert timestamp to Micro Seconds
	*pnTimeStamp = (uint64_t)((timeStamp.tv_sec * USEC_PER_SEC) + timeStamp.tv_usec);
	
	return true;
}
uint64_t PerformanceRec::GetChildTotalTime()
{
	Node* 					pChild 			= NULL;
	uint64_t				nTotalChild		= 0;
	list<Node*>::iterator 	iter			= GetSiblingIterator();
	
	while(IsSiblingEnd(iter) == false) {
		pChild = *iter;
		if(pChild->GetNodeType() == PerfRecord) {
			PerformanceRec* pPerfRec = (PerformanceRec*)pChild;
			nTotalChild += pPerfRec->GetTotalTime();
//			cout << "Node (" << this << ") Child (" << pPerfRec << ") Total time is " << pPerfRec->GetTotalTime() << endl;
		}
		iter++;
	}
//	cout << "Total child time for " << this << " is " << nTotalChild << endl;
	return nTotalChild;
}

clock_t	PerformanceRec::GetChildTotalTimeCPU()
{
	Node* 					pChild 			= NULL;
	clock_t					nTotalChildCPU	= 0;
	list<Node*>::iterator 	iter			= GetSiblingIterator();

	while(IsSiblingEnd(iter) == false) {
		pChild = *iter;
		if(pChild->GetNodeType() == PerfRecord) {
			PerformanceRec* pPerfRec = (PerformanceRec*)pChild;
			nTotalChildCPU += pPerfRec->GetTotalCPUTime();
//			cout << "Node (" << this << ") Child (" << pPerfRec << ") Total CPU time is " << pPerfRec->GetTotalCPUTime() << endl;
		}
		iter++;
	}
//	cout << "Total child CPU time for " << this << " is " << nTotalChildCPU << endl;
	return nTotalChildCPU;

}

bool PerformanceRec::SetID(PerfID nID)
{
	mID	= nID;
	return true;
}
PerfID PerformanceRec::GetID()
{
	return mID;
}
bool PerformanceRec::SetCatID(PerfID nID)
{
	mCatID = nID;
	return true;
}
PerfID PerformanceRec::GetCatID()
{
	return mCatID;
}
bool PerformanceRec::SetThreadID(pthread_t nThreadID)
{
	mThreadID	= nThreadID;
	return true;
}
pthread_t PerformanceRec::GetThreadID()
{
	return mThreadID;
}

bool PerformanceRec::AddEntry()
{
	GetCurrentTimeStamp(&mCurrentEntryTime);

	struct tms cpu_data;
	times(&cpu_data);

	mEntryCPUTime	= 	cpu_data.tms_utime + cpu_data.tms_cutime; 		// User Time
	mEntryCPUTime	+= 	cpu_data.tms_stime + cpu_data.tms_cstime; 		// System Time
	
	if(mbFirstEntry == true) {
		mStartTime 		= mCurrentEntryTime;
		mStartCPUTime	= mEntryCPUTime;
		mThreadID		= pthread_self();
		
		mbFirstEntry = false;
	}
	
	return true;
}
bool PerformanceRec::AddExit()
{
	uint64_t	delta		= 0;
	clock_t		deltaCPU	= 0;
	
	GetCurrentTimeStamp(&mLastExitTime);

	struct tms cpu_data;
	times(&cpu_data);

	mExitCPUTime	= 	cpu_data.tms_utime + cpu_data.tms_cutime; 		// User Time
	mExitCPUTime	+= 	cpu_data.tms_stime + cpu_data.tms_cstime; 		// System Time

	// Find the elapsed time
	delta 		= mLastExitTime - mCurrentEntryTime;
	deltaCPU	= mExitCPUTime - mEntryCPUTime;

	// Record the data
	mTotalTime 		+= delta;
	mTotalCPUTime	+= deltaCPU;
	mTotalCalls++;
	if(mMinTime > delta) {
		mMinTime = delta;
	}
	if(mMaxTime < delta) {
		mMaxTime = delta;
	}
	if(mMinCPUTime > deltaCPU) {
		mMinCPUTime = deltaCPU;
	}
	if(mMaxCPUTime < deltaCPU) {
		mMaxCPUTime = deltaCPU;
	}
	return true;
}
bool PerformanceRec::GetReport(PerfRecordReport* report)
{
	if(mTotalCalls == 0) {
//		cout << __FUNCTION__ << "Total calls = 0 for ID " << (unsigned int)GetID() << endl;
		memset(report, 0, sizeof(PerfRecordReport));
		return false;
	}
	else {
		report->nStartTime 			= mStartTime;
		report->nEndTime			= mLastExitTime;
		report->nTotalCalls			= mTotalCalls;
		report->nTotalTime			= mTotalTime;
		report->nTotalSelf 			= mTotalTime - GetChildTotalTime();
		report->nMinTime			= mMinTime;
		report->nMaxTime			= mMaxTime;
		report->nStartCPUTime		= ((double)(mStartCPUTime)) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
		report->nExitCPUTime		= ((double)(mExitCPUTime)) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
		report->nTotalCPUTime		= ((double)(mTotalCPUTime)) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
		report->nTotalCPUTimeSelf	= ((double)(mTotalCPUTime - GetChildTotalTimeCPU())) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
		report->nMinCPUTime			= ((double)(mMinCPUTime)) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
		report->nMaxCPUTime			= ((double)(mMaxCPUTime)) / sysconf(_SC_CLK_TCK); 	// Convert to seconds
	}
	return true;
}
uint64_t PerformanceRec::GetTotalTime()
{
	return mTotalTime;
}
clock_t PerformanceRec::GetTotalCPUTime()
{
	return mTotalCPUTime;
}
uint32_t PerformanceRec::GetTotalSamples()
{
	return mTotalCalls;
}

