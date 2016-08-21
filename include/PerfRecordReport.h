#ifndef PERFRECORDREPORT_H_
#define PERFRECORDREPORT_H_

/*
**---------------------------------------------------------------------
** Includes
**---------------------------------------------------------------------
*/
#include <stdint.h>

/*
**-------------------------------------------------------------------------
**  Macro Definitions
**-------------------------------------------------------------------------
*/

/*
**---------------------------------------------------------------------
** Constants and Primary Macro Definitions
**---------------------------------------------------------------------
*/


/*
**---------------------------------------------------------------------
** Type Definitions
**---------------------------------------------------------------------
*/

typedef struct PerfRecordReport_s
{
	uint32_t		nTotalCalls;
	uint64_t		nTotalTime;
	uint64_t		nTotalSelf;
	uint32_t		nMinTime;
	uint32_t		nMaxTime;
	uint64_t		nStartTime;
	uint64_t		nEndTime;
	double			nStartCPUTime;
	double			nExitCPUTime;
	double			nTotalCPUTime;
	double			nTotalCPUTimeSelf;
	double			nMinCPUTime;
	double			nMaxCPUTime;
} PerfRecordReport;


/*
**---------------------------------------------------------------------
** Prototypes
**---------------------------------------------------------------------
*/



#endif /*PERFRECORDREPORT_H_*/
