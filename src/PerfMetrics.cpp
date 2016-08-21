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

/*
**---------------------------------------------------------------------
** Includes
**---------------------------------------------------------------------
*/

#include "PerfMetrics.h"
#include "performance_id.h"

#ifdef FEATURE_PERFORMANCE_PROFILING

#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>

#include <list>
#ifdef PERFORMANCE_MEMORY    
#include <map>
#endif
#include <iostream>
#include <iomanip>
#include <string>

#include "PerfMetrics.h"

#include "Node.h"
#include "ThreadRecord.h"
#include "AllocRecord.h"
#include "PerformanceRec.h"
#include "PerfRecordReport.h"

#ifdef __ANDROID__
// For Android LOGI
#undef LOG_TAG
#define LOG_TAG "PerfMetrics"
#include <utils/Log.h>
#include <cutils/properties.h>
#else
#include <stdarg.h>
#endif

#define USEC_PER_SEC			1000000
#define MAX_UINT32       		0xFFFFFFFFul
#define MAX_REPORT_NUMBER_TAB	1000

#define ORDER_CHILD_DATA
#define TREE_REPORT_XML
#define WRITE_REPORT_TO_FILE
#define WRITE_REPORT_TO_SCREEN
//#define DISPLAY_CPU_TOTALS

/*
**---------------------------------------------------------------------
** Constants and Primary Macro Definitions
**---------------------------------------------------------------------
*/
static const char * szIDReportFile 			= "./IDReport.txt";
static const char * szCatReportFile 		= "./CategoryReport.txt";
#ifdef TREE_REPORT_XML
static const char * szTreeReportFile 		= "./TreeReport.xml";
#else
static const char * szTreeReportFile 		= "./TreeReport.txt";
#endif
static const char * ELEMENT_DELIMITER		= ";";

/*
**---------------------------------------------------------------------
** Internal Types
**---------------------------------------------------------------------
*/
typedef enum ReportType_e
{
	CategoryReportType,
	IDReportType,
	TreeReportType
} ReportType;

typedef struct CategoryReport_s
{
	const char * 	szName;
	uint32_t		catID;
	uint32_t		nSamples;
	uint64_t		nTotalTime;
	uint64_t		nSelfTime;
	uint32_t		nMinTime;
	uint32_t		nMaxTime;
	uint32_t		nAvgTime;
	double			nTotalCPUTime;
	double			nSelfCPUTime;
	double			nMinCPUTime;
	double			nMaxCPUTime;
	double			nAvgCPUTime;
} CategoryReport;

typedef struct IDReport_s
{
	const char * 	szName;
	const char * 	szCategory;
	uint32_t		nSamples;
	uint64_t		nTotalTime;
	uint64_t		nSelfTime;
	uint32_t		nMinTime;
	uint32_t		nMaxTime;
	uint32_t		nAvgTime;
	double			nTotalCPUTime;
	double			nSelfCPUTime;
	double			nMinCPUTime;
	double			nMaxCPUTime;
	double			nAvgCPUTime;
} IDReport;

typedef struct PerfCategoryData_s {
    const char*     szName;
    uint32_t        nID;
} PerfCategoryData;

list<PerfCategoryData*>		gPerfCatList;


PerfID	gPERF_ID_THREAD_START		= INVALID_PERF_ID;
PerfID	gPERF_CATID_THREAD_START	= INVALID_PERF_ID;

typedef struct PerfIDData_s {
    const char*     szName;
    PerfID			id;
    PerfID			categoryID;
    const char *    szCategory;
} PerfIDData;

list<PerfIDData*>		gPerfIDList;


#ifdef PERFORMANCE_MEMORY    
struct AllocPair : pair<unsigned int, AllocRecord*> {
    AllocPair(AllocRecord* pRec) : pair<unsigned int, AllocRecord*>(pRec->GetID(), pRec) { }
};
#endif
/*
**---------------------------------------------------------------------
** Internal Prototypes
**---------------------------------------------------------------------
*/

/* Helper Functions */


/*
**---------------------------------------------------------------------
** File-Scoped Static Variables
**---------------------------------------------------------------------
*/
static	list<ThreadRecord*>	mThreadList;
static 	uint64_t			gStartTime	= 0;
static 	uint64_t			gEndTime	= 0;


#ifdef PERFORMANCE_MEMORY    
static  map<unsigned int, AllocRecord*> mAllocMap;
static  uint64_t                        mAllocCurrent;
static  uint64_t                        mAllocMax;
#endif

static PerfID 				nID			= 0;
static pthread_mutex_t		lock;
static bool					bLockInit	= false;

/*
**---------------------------------------------------------------------
** Internal Functions
**---------------------------------------------------------------------
*/

static void LogData(const char * strFmt, ...)
{
    va_list     va;
    va_start(va, strFmt);
    char buff[2048];
    if ( vsnprintf(buff, 2048, strFmt, va) != -1 ) {
#ifdef __ANDROID__
    	LOGI("%s", buff);
#else
    	printf("%s", buff);
#endif
    }
    va_end(va);
    
 }
static bool GetCurrentTimeStamp(uint64_t* pnTimeStamp)
{
	struct timeval 	timeStamp;

	gettimeofday(&timeStamp, NULL);
	
	// Convert timestamp to Micro Seconds
	*pnTimeStamp = (uint64_t)((timeStamp.tv_sec * USEC_PER_SEC) + timeStamp.tv_usec);
	
	return true;
}
//static uint32_t FindCategoryByID(PerfID id)
//{
//	list<PerfIDData*>::iterator 	iter = gPerfIDList.begin();
//
//	while(iter != gPerfIDList.end()) {
//		PerfIDData* pPerfData = *iter;
//		if(pPerfData->id == id) {
//			return pPerfData->category;
//		}
//		iter++;
//	}
//	return INVALID_PERF_ID;
//}
static uint32_t FindIndexByCategoryID(uint32_t catID)
{
	uint32_t							idx 	= 0;
	list<PerfCategoryData*>::iterator 	iter 	= gPerfCatList.begin();

	while(iter != gPerfCatList.end()) {
		PerfCategoryData* pPerfData = *iter;
		if(pPerfData->nID == catID) {
			return idx;
		}
		iter++;
		idx++;
	}

	return INVALID_PERF_ID;
}
static uint32_t FindIndexByPerfID(PerfID id)
{
	uint32_t						idx 	= 0;
	list<PerfIDData*>::iterator 	iter 	= gPerfIDList.begin();

	while(iter != gPerfIDList.end()) {
		PerfIDData* pPerfData = *iter;
		if(pPerfData->id == id) {
			return idx;
		}
		iter++;
		idx++;
	}
	return INVALID_PERF_ID;
}
static PerfIDData* FindPerfDataByPerfID(PerfID id)
{
	list<PerfIDData*>::iterator 	iter = gPerfIDList.begin();

	while(iter != gPerfIDList.end()) {
		PerfIDData* pPerfData = *iter;
		if(pPerfData->id == id) {
			return pPerfData;
		}
		iter++;
	}
	return NULL;
}
static PerfIDData* FindPerfDataByIdx(uint32_t idx)
{
	uint32_t						nCount	= 0;
	list<PerfIDData*>::iterator 	iter 	= gPerfIDList.begin();

	while(nCount != gPerfIDList.size()) {
		PerfIDData* pPerfData = *iter;
		if(nCount == idx) {
			return pPerfData;
		}
		iter++;
		nCount++;
	}
	return NULL;
}
static void SumCatReportData(CategoryReport* pCatReport, PerfRecordReport* pReport)
{
	pCatReport->nSamples 	+= pReport->nTotalCalls;
	// Clock
	pCatReport->nTotalTime 	+= pReport->nTotalTime;
	pCatReport->nSelfTime	+= pReport->nTotalSelf;
	if(pReport->nMaxTime > pCatReport->nMaxTime) {
		pCatReport->nMaxTime = pReport->nMaxTime;
	}
	if(pReport->nMinTime < pCatReport->nMinTime || pCatReport->nMinTime == 0) {
		pCatReport->nMinTime = pReport->nMinTime;
	}
	if(pCatReport->nSamples > 0) {
		pCatReport->nAvgTime 	= pCatReport->nTotalTime / pCatReport->nSamples;
	}
	// CPU
	pCatReport->nTotalCPUTime 	+= pReport->nTotalCPUTime;
	pCatReport->nSelfCPUTime	+= pReport->nTotalCPUTimeSelf;
	if(pReport->nMaxCPUTime > pCatReport->nMaxCPUTime) {
		pCatReport->nMaxCPUTime = pReport->nMaxCPUTime;
	}
	if(pReport->nMinCPUTime < pCatReport->nMinCPUTime || pCatReport->nMinCPUTime == 0) {
		pCatReport->nMinCPUTime = pReport->nMinCPUTime;
	}
	if(pCatReport->nSamples > 0) {
		pCatReport->nAvgCPUTime 	= pCatReport->nTotalCPUTime / pCatReport->nSamples;
	}
	return;
}
static void SumIDReportData(IDReport* pIDReport, PerfRecordReport* pReport)
{
	pIDReport->nSamples 	+= pReport->nTotalCalls;
	// Clock
	pIDReport->nTotalTime 	+= pReport->nTotalTime;
	pIDReport->nSelfTime	+= pReport->nTotalSelf;
	if(pReport->nMaxTime > pIDReport->nMaxTime) {
		pIDReport->nMaxTime = pReport->nMaxTime;
	}
	if(pReport->nMinTime < pIDReport->nMinTime || pIDReport->nMinTime == 0) {
		pIDReport->nMinTime = pReport->nMinTime;
	}
	if(pIDReport->nSamples > 0) {
		pIDReport->nAvgTime 	= pIDReport->nTotalTime / pIDReport->nSamples;
	}
	// CPU
	pIDReport->nTotalCPUTime 	+= pReport->nTotalCPUTime;
	pIDReport->nSelfCPUTime		+= pReport->nTotalCPUTimeSelf;
	if(pReport->nMaxCPUTime > pIDReport->nMaxCPUTime) {
		pIDReport->nMaxCPUTime = pReport->nMaxCPUTime;
	}
	if(pReport->nMinCPUTime < pIDReport->nMinCPUTime || pIDReport->nMinCPUTime == 0) {
		pIDReport->nMinCPUTime = pReport->nMinCPUTime;
	}
	if(pIDReport->nSamples > 0) {
		pIDReport->nAvgCPUTime 	= pIDReport->nTotalCPUTime / pIDReport->nSamples;
	}
	return;
}
static bool GetNodeCategoryData(Node* pNode, CategoryReport* catReport, uint16_t nCategories)
{
	bool retVal = false;
	if(pNode != NULL) {
		if(pNode->GetNodeType() == PerfRecord) {
			PerfRecordReport report;
			PerformanceRec* pPerfRec = (PerformanceRec*)pNode;
			pPerfRec->GetReport(&report);
			// Find the Category for this ID
			uint32_t idCat = pPerfRec->GetCatID();
			if(idCat != INVALID_PERF_ID) {
				bool bFoundCat = false;
				uint32_t idx = FindIndexByCategoryID(idCat);
				if(idx <= nCategories) {
					// Have we reported this category for this node tree
					Node *pParent = pNode->GetParent();
					while(pParent != NULL) {
						PerformanceRec* pParentPerfRec = (PerformanceRec*)pParent;
						//cout << "Checking node catID " << (int)idCat << " with parent's id " << (int)pParentPerfRec->GetCatID() << endl;
						if(pParentPerfRec->GetCatID() == idCat) {
							bFoundCat = true;
						}
						pParent = pParent->GetParent();
					}
					if(bFoundCat == false)
					// We found one
					SumCatReportData(&catReport[idx], &report);
				}
				retVal = true;
			}
		}
	}	
	return retVal;
}
static bool GetNodeIDData(Node* pNode, IDReport* idReport, uint16_t nIDs)
{
	bool retVal = false;
	if(pNode != NULL) {
		if(pNode->GetNodeType() == PerfRecord) {
			PerfRecordReport report;
			PerformanceRec* pPerfRec = (PerformanceRec*)pNode;
			pPerfRec->GetReport(&report);
			// Find the Category for this ID
			uint32_t idx = FindIndexByPerfID(pPerfRec->GetID());
			if(idx != INVALID_PERF_ID) {
				if(idx <= nIDs) {
					// We found one
					SumIDReportData(&idReport[idx], &report);
				}
				retVal = true;
			}
		}
	}	
	return retVal;	
}
static bool WriteNodeDataToScreen(Node* pNode, uint16_t nSize)
{
	Node* pParent = pNode->GetParent();;
	while(pParent != NULL) {
		pParent = pParent->GetParent();
		cout << "\t";
	}
	if(pNode->GetNodeType() == PerfRecord) {
		PerformanceRec* 	pPerfRec 		= (PerformanceRec*)pNode;
		PerfRecordReport 	PerfRecord;

		pPerfRec->GetReport(&PerfRecord);
		PerfIDData* pPerfData = FindPerfDataByPerfID(pPerfRec->GetID());
		if(pPerfData != NULL) {
			cout << pPerfData->szName;
		}
		else {
//				cout << "ERROR:  pPerfData = NULL" << endl;
		}

		if(pPerfRec->GetID() == gPERF_ID_THREAD_START) {
			cout << " ThreadID = " <<  hex << pPerfRec->GetThreadID();
		}
		if(PerfRecord.nTotalCalls > 0) {
			cout.setf(ios::showpoint);
			cout << setiosflags(ios::fixed) << setprecision(3) << " (Calls) ";
			cout << dec << PerfRecord.nTotalCalls << " ";
			cout << " (Clock:T,S,Mx,Mn,A) ";
			cout << dec << PerfRecord.nTotalTime / 1000.0 << " ";
			cout << PerfRecord.nTotalSelf / 1000.0 << " ";
			cout << PerfRecord.nMaxTime / 1000.0 << " ";
			cout << PerfRecord.nMinTime / 1000.0 << " ";
			cout << (PerfRecord.nTotalTime/PerfRecord.nTotalCalls) / 1000.0;
#ifdef DISPLAY_CPU_TOTALS
			cout << " (CPU:T,S,Mx,Mn,A) ";
			cout << dec << PerfRecord.nTotalCPUTime * 1000.0 << " ";
			cout << PerfRecord.nTotalCPUTimeSelf * 1000.0 << " ";
			cout << PerfRecord.nMaxCPUTime * 1000.0 << " ";
			cout << PerfRecord.nMinCPUTime * 1000.0 << " ";
			cout << (PerfRecord.nTotalCPUTime/PerfRecord.nTotalCalls) * 1000.0;
#endif
			cout.unsetf(ios::showpoint);
		}
	}
	else {
		cout << "Error, incorrect node type";
	}
	cout << endl;

	return true;
}
#ifndef TREE_REPORT_XML
static bool WriteNodeDataToFile(Node* pNode, uint16_t nSize, FILE* fp)
{
	// Write the data to a file
	Node* pParent = pNode->GetParent();;
	while(pParent != NULL) {
		pParent = pParent->GetParent();
		fprintf(fp,"|  ");
	}
	if(pNode->GetNodeType() == PerfRecord) {
		PerformanceRec* 	pPerfRec 		= (PerformanceRec*)pNode;
		PerfRecordReport 	PerfRecord;

		pPerfRec->GetReport(&PerfRecord);
		PerfIDData* pPerfData = FindPerfDataByPerfID(pPerfRec->GetID());
		if(pPerfData != NULL) {
			fprintf(fp,"%s", pPerfData->szName);
		}
		if(pPerfRec->GetID() == gPERF_ID_THREAD_START) {
			fprintf(fp,"ThreadID = %lX\n", (unsigned long)pPerfRec->GetThreadID());
		}
		if(PerfRecord.nTotalCalls > 0) {
			fprintf(fp, " (Calls) ");
			fprintf(fp, "%d", PerfRecord.nTotalCalls);
			fprintf(fp, " (Clock:T,S,Mx,Mn,A) ");
			fprintf(fp, "%0.3f ", PerfRecord.nTotalTime / 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nTotalSelf / 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nMaxTime / 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nMinTime / 1000.0);
			fprintf(fp, "%0.3f ", (PerfRecord.nTotalTime/PerfRecord.nTotalCalls) / 1000.0);
#ifdef DISPLAY_CPU_TOTALS
			cout << " (CPU:T,S,Mx,Mn,A) ";
			fprintf(fp, "%0.3f ", PerfRecord.nTotalCPUTime * 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nTotalCPUTimeSelf * 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nMaxCPUTime * 1000.0);
			fprintf(fp, "%0.3f ", PerfRecord.nMinCPUTime * 1000.0);
			fprintf(fp, "%0.3f ", (PerfRecord.nTotalCPUTime/PerfRecord.nTotalCalls) * 1000.0);
#endif
			fprintf(fp,"\n");
		}
	}
	return true;
}
#else //TREE_REPORT_XML

static void EscapeToXML(std::string& data)
{
    std::string buffer;
    buffer.reserve(data.size() + 50);
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(1, data[pos]);  break;
        }
    }
    data.swap(buffer);
}
static bool WriteNodeDataToFileAsXML(Node* pNode, uint16_t nSize, FILE* fp)
{
	// Tracking vars
	static char 		indent[512];
	static const char* 	indentStr = "   ";
	int 				indentLen = strlen(indentStr);
	int 				idx = 0;

	// Write the data to a file
	Node* pParent = pNode->GetParent();
	memset(indent, 0, 512);
	while(pParent != NULL) {
		pParent = pParent->GetParent();
		sprintf(&indent[idx], "%s", indentStr);
		idx += indentLen;
	}
	if(pNode->GetNodeType() == PerfRecord) {
		PerformanceRec* 	pPerfRec 		= (PerformanceRec*)pNode;
		PerfRecordReport 	PerfRecord;

		pPerfRec->GetReport(&PerfRecord);
		PerfIDData* pPerfData = FindPerfDataByPerfID(pPerfRec->GetID());
		if(pPerfRec->GetID() == gPERF_ID_THREAD_START) {
			fprintf(fp,"<Thread ID='%lX' >\n", (unsigned long)pPerfRec->GetThreadID());
		}
		if(PerfRecord.nTotalCalls > 0) {
			fprintf(fp,"%s", indent);
			std::string funcName(pPerfData->szName);
			EscapeToXML(funcName);
			fprintf(fp,"<Entry Name='%s'", funcName.c_str());

			fprintf(fp, " Calls='%d'", PerfRecord.nTotalCalls);
			fprintf(fp, " Total='%0.3f' Self='%0.3f' Max='%0.3f' Min='%0.3f' Avg='%0.3f'",
							PerfRecord.nTotalTime / 1000.0,
							PerfRecord.nTotalSelf / 1000.0,
							PerfRecord.nMaxTime / 1000.0,
							PerfRecord.nMinTime / 1000.0,
							(PerfRecord.nTotalTime/PerfRecord.nTotalCalls) / 1000.0);

#ifdef DISPLAY_CPU_TOTALS
			fprintf(fp, " Total_CPU='%0.3f' Self_CPU='%0.3f' Max_CPU='%0.3f' Min_CPU='%0.3f' Avg_CPU='%0.3f'",
							PerfRecord.nTotalCPUTime * 1000.0,
							PerfRecord.nTotalCPUTimeSelf * 1000.0,
							PerfRecord.nMaxCPUTime * 1000.0),
							PerfRecord.nMinCPUTime * 1000.0,
							(PerfRecord.nTotalCPUTime/PerfRecord.nTotalCalls) * 1000.0);
#endif
			if(pNode->IsSiblingEnd(pNode->GetSiblingIterator())) {
				// no children
				fprintf(fp," />\n");
				// Do we close the branch
				Node* pParentNode = pNode->GetParent();
				Node* pCurNode = pNode;
				while(pParentNode != NULL && pParentNode->GetNextSibling(pCurNode) == NULL) {
					// no more sibling peers.
					PerformanceRec* 	pPerfRecParent  = (PerformanceRec*)pCurNode->GetParent();
					//PerfIDData* 		pPerfDataParent = FindPerfDataByPerfID(pPerfRecParent->GetID());

					if(pPerfRecParent->GetID() == gPERF_ID_THREAD_START) {
						fprintf(fp,"</Thread>\n");
					}
					else {
						// close parent
						indent[(idx - indentLen)] = '\0';
						fprintf(fp,"%s", indent);
						fprintf(fp,"</Entry>\n");
					}
					// Move up the tree
					idx -= indentLen;
					pCurNode = pParentNode;
					pParentNode = pCurNode->GetParent();
				}
			}
			else {
				fprintf(fp," >\n");
			}

		}
	}
	return true;
}
#endif //TREE_REPORT_XML

#ifdef ORDER_CHILD_DATA
static bool OrderChildByTotal(Node* first, Node* second)
{
	if(first->GetNodeType() == PerfRecord && second->GetNodeType() == PerfRecord) {
		PerformanceRec* 	pPerfRec1 		= (PerformanceRec*)first;
		PerformanceRec* 	pPerfRec2 		= (PerformanceRec*)second;
		if(pPerfRec1->GetTotalTime() < pPerfRec2->GetTotalTime()) {
			return false;
		}
	}
	return true;
}
#endif
static bool GetChildData(Node* pNode, void* report, uint16_t nSize, ReportType eType, FILE* fp=NULL)
{	
	// There are no more children add the data for this node
	if(eType == CategoryReportType) {
		GetNodeCategoryData(pNode, (CategoryReport*)report, nSize);
	}
	else if(eType == IDReportType) {
		GetNodeIDData(pNode, (IDReport*)report, nSize);	
	}
	else if(eType == TreeReportType) {
		if(fp == NULL) {
			WriteNodeDataToScreen(pNode, nSize);
		}
		else {
#ifdef TREE_REPORT_XML
			WriteNodeDataToFileAsXML(pNode, nSize, fp);
#else
			WriteNodeDataToFile(pNode, nSize, fp);
#endif
		}
	}
	else {
		return false;
	}

#ifdef ORDER_CHILD_DATA
	list<Node*>* pSiblingList = pNode->GetSiblingList();
	pSiblingList->sort(OrderChildByTotal);
#endif
	// Recurse
	list<Node*>::iterator iter	= pNode->GetSiblingIterator();
	while(pNode->IsSiblingEnd(iter) == false) {
		Node* pChild = *iter;
//		cout << "GetChildData for " << pNode << " found child " << pChild << endl;
		if(pChild->GetNodeType() == PerfRecord) {
			if(GetChildData(pChild, report, nSize, eType, fp) == false) {
				cout << "GetChildData ERROR"  << endl;
				break;
			}
		}
		iter++;
	}
	
	return true;
}

static bool GenerateReport(void* report, uint16_t nSize, ReportType eType)
{
	ThreadRecord* 					pThread		= NULL;
	list<ThreadRecord*>::iterator 	iter 		= mThreadList.begin();

	// Walk the threads
	while(iter != mThreadList.end()) {
		pThread	= *iter;
		// Walk the nodes
		Node * pParent = pThread->GetRootNode();
		if(pParent != NULL) {
			if(GetChildData(pParent, report, nSize, eType) == false) {
				cout << "GenerateReport ERROR"  << endl;
				break;
			}
		}
		iter++;
	}	
	return true;
}
static PerformanceRec* NewPerfRecord(PerfID id, PerfID catID, NodeType eType, Node* pParent, pthread_t threadID)
{
	if(id == INVALID_PERF_ID) {
		cout << "\t NewPerfRecord invalid ID" << endl;
	}
	PerformanceRec* pNewRecord	= new PerformanceRec();

	pNewRecord->SetID(id);
	pNewRecord->SetCatID(catID);
	pNewRecord->SetNodeType(eType);
	pNewRecord->SetParent(pParent);
	pNewRecord->SetThreadID(threadID);	
	
//	cout << "Created new PerfRecord " << pNewRecord << " ID = " << id;
//	cout << " Name = " << gPerfIDData[FindIndexByPerfID(id)].szName;
//	cout << " Parent = " << pParent << endl;
	return pNewRecord;
}

static void SortIDByTotalCalls(IDReport* pReport, int nElements)
{
	int i, j;
	IDReport temp;
	int numLength = nElements;

	for(i = 0; i< (numLength -1); i++)  {
		for(j = (i+1); j < numLength; j++) {
			if (pReport[i].nSamples < pReport[j].nSamples) {
				temp 		= pReport[i];          // swap
				pReport[i] 	= pReport[j];
				pReport[j] 	= temp;
			}
		}
	}

	return;
}

void WriteCategoryReportToFile(CategoryReport* pReport, int nElements)
{
	FILE * fp = fopen(szCatReportFile, "w");
	if(fp != NULL) {
		int idx = 0;

		fprintf(fp, "Name");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Samples");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Total");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Self");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Min");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Max");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Avg");
		#ifdef DISPLAY_CPU_TOTALS
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "CPU Total");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Self");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Min");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Max");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Avg");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		#endif
		fprintf(fp, "\n");
		for(idx = 0; idx < nElements; idx++) {
			if(pReport[idx].nSamples > 0) {
				fprintf(fp, "%s", pReport[idx].szName);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%u", pReport[idx].nSamples);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nTotalTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nSelfTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMinTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMaxTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nAvgTime / 1000.0);
#ifdef DISPLAY_CPU_TOTALS
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nTotalCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nSelfCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMinCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMaxCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nAvgCPUTime * 1000.0);
#endif
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
	}
	return;
}
void WriteIDReportToFile(IDReport* pReport, int nElements)
{
	FILE * fp = fopen(szIDReportFile, "w");
//	printf("AVE -- WriteIDReportToFile nElements = %d fp = %p\n", nElements, fp);
	if(fp != NULL) {
		int idx = 0;

		fprintf(fp, "Name");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Samples");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Total");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Self");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Min");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Max");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Avg");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Category");
#ifdef DISPLAY_CPU_TOTALS
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "CPU Total");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Self");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Min");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Max");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
		fprintf(fp, "Avg");
		fprintf(fp, "%s", ELEMENT_DELIMITER);
#endif
		fprintf(fp, "\n");
		for(idx = 0; idx < nElements; idx++) {
//			printf("AVE -- WriteIDReportToFile name = %s samples = %d\n", pReport[idx].szName, pReport[idx].nSamples);
			if(pReport[idx].nSamples > 0) {
				fprintf(fp, "%s", pReport[idx].szName);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%u", pReport[idx].nSamples);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nTotalTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nSelfTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMinTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMaxTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nAvgTime / 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%s", pReport[idx].szCategory);
#ifdef DISPLAY_CPU_TOTALS
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nTotalCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nSelfCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMinCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nMaxCPUTime * 1000.0);
				fprintf(fp, "%s", ELEMENT_DELIMITER);
				fprintf(fp, "%lf", pReport[idx].nAvgCPUTime * 1000.0);
#endif
				fprintf(fp, "\n");
			}
		}
		fclose(fp);
	}
	return;
}
void WriteTreeReportToFile()
{
	ThreadRecord* 					pThread		= NULL;
	list<ThreadRecord*>::iterator 	iter 		= mThreadList.begin();

	FILE * fp = fopen(szTreeReportFile, "w");
	if(fp != NULL) {
		// Document Header
#ifdef TREE_REPORT_XML
		fprintf(fp,"<?xml version='1.0' encoding='utf-8' standalone='no'?>\n<TreeReport>\n");
#endif
		// Walk the threads
		while(iter != mThreadList.end()) {
			pThread	= *iter;
			// Walk the nodes
			Node * pParent = pThread->GetRootNode();
			if(pParent != NULL) {
				if(GetChildData(pParent, NULL, 0, TreeReportType, fp) == false) {
					cout << "WriteTreeReportToFile ERROR"  << endl;
					break;
				}
			}
			iter++;
		}
#ifdef TREE_REPORT_XML
		fprintf(fp,"</TreeReport>\n");
#endif
		fclose(fp);
	}
	return;
}
/*
**---------------------------------------------------------------------
** External Functions
**---------------------------------------------------------------------
*/
/* Initialize the performance metrics system */
bool PerfMetrics::PerfStart ( void )
{

	// Create an array for the threads
	mThreadList.clear();
	GetCurrentTimeStamp(&gStartTime);

	// Setup mutex
	if(bLockInit) {
		pthread_mutexattr_t attr;
		if (pthread_mutexattr_init(&attr) == 0) {
			if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0)	{
				int retVal = pthread_mutex_init(&lock, &attr);
				if(retVal == 0)
					bLockInit = true;
			}
			pthread_mutexattr_destroy(&attr);
		}
	}
	
	return true;
}
/* Set the stop time */
bool PerfMetrics::PerfStop ( void )
{
	GetCurrentTimeStamp(&gEndTime);

	pthread_mutex_destroy(&lock);
	bLockInit = false;

	return true;
}
/* Clean up the performance metrics system */
bool PerfMetrics::PerfCleanup ( void )
{
	ThreadRecord* 	pThread		= NULL;
	PerfIDData* 	pPerfData	= NULL;

	if(gEndTime == 0) {
		// If we have't already stopped then stop now.
		PerfStop();
	}
	while(!mThreadList.empty()) {
		pThread		= mThreadList.front();
		mThreadList.pop_front();
		delete pThread;
	}
	while(!gPerfIDList.empty()) {
		pPerfData	= gPerfIDList.front();
		gPerfIDList.pop_front();
		if(pPerfData->szName)
			delete[] pPerfData->szName;
		delete pPerfData;
	}

	// Reset the static variables.
	gStartTime	= 0;
	gEndTime	= 0;

	return true;
}
/* Create detailed report based on accumulated data */
bool PerfMetrics::PerfReport ( void )
{
	uint32_t 			idx			= 0;
	uint64_t			nTotalTime	= gEndTime - gStartTime;
	CategoryReport		catReport[gPerfCatList.size()];
	IDReport			idReport[gPerfIDList.size()];
	
    LogData("Generating Performance Report total time = %llu (%llu - %llu)\n", nTotalTime, gEndTime, gStartTime);
	memset(&catReport[0], 0, sizeof(CategoryReport) * gPerfCatList.size());
	memset(&idReport[0], 0, sizeof(IDReport) * gPerfIDList.size());
	
    cout << endl;

    #ifdef PERFORMANCE_MEMORY// Memory
    cout << "Max allocated = " << mAllocMax << " (bytes)" << endl;
    cout << "Current allocated (leaked) = " << mAllocCurrent << " (bytes)" << endl << endl;
	#endif
	// Total Time
	cout << setiosflags(ios::fixed) << setprecision(3) << "Total Time = " << nTotalTime / 1000.0 << " (msec)" << endl;


	// Total Category
	LogData("Setting up category report - num of categories = %d\n", gPerfCatList.size());
	for(std::list<PerfCategoryData*>::iterator iter = gPerfCatList.begin(); iter != gPerfCatList.end(); ++iter) {
		catReport[idx].szName	= (*iter)->szName;
		catReport[idx].catID	= (*iter)->nID;
//		cout << " Setting up Category " << catReport[idx].szName;
//		cout << " ID = " << catReport[idx].catID << endl;
		idx++;
	}
	GenerateReport((void*)&catReport[0], gPerfCatList.size(), CategoryReportType);

#ifdef WRITE_REPORT_TO_FILE
	WriteCategoryReportToFile(&catReport[0], gPerfCatList.size());
#endif

#ifdef WRITE_REPORT_TO_SCREEN
	cout << "\n\nCategory Report " << endl;
	cout << "Name\t\tSamples\t\tTotal\t\tSelf\t\tMin\t\tMax\t\tAvg";
#ifdef DISPLAY_CPU_TOTALS
	cout << "\t\tCPU Total\t\tSelf\t\tMin\t\tMax\t\tAvg";
#endif
	cout << endl;
	cout << "-------------------------------------------------------------------------------------------------------";
#ifdef DISPLAY_CPU_TOTALS
	cout << "---------------------------------------------------------------------------------------------";
#endif
	cout << endl;
	for(idx = 0; idx < gPerfCatList.size(); idx++) {
		if(catReport[idx].nSamples > 0) {
			cout << catReport[idx].szName;
			if(strlen(catReport[idx].szName) > 8) {
				cout << "\t";
			}
			else {
				cout << "\t\t";
			}
			cout << catReport[idx].nSamples;
			catReport[idx].nSamples >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nTotalTime / 1000.0;
			catReport[idx].nTotalTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nSelfTime / 1000.0;
			catReport[idx].nSelfTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nMinTime / 1000.0;
			catReport[idx].nMinTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nMaxTime / 1000.0;
			catReport[idx].nMaxTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nAvgTime / 1000.0;
#ifdef DISPLAY_CPU_TOTALS
			cout << "\t\t";
			cout << catReport[idx].nTotalCPUTime * 1000.0;
			catReport[idx].nTotalCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nSelfCPUTime * 1000.0;
			catReport[idx].nSelfCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nMinCPUTime * 1000.0;
			catReport[idx].nMinCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nMaxCPUTime * 1000.0;
			catReport[idx].nMaxCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << catReport[idx].nAvgCPUTime * 1000.0;
#endif
			cout << endl;
		}
	}
	cout << endl << endl;

#endif // WRITE_REPORT_TO_SCREEN

	// Total ID
	LogData("Setting up ID report num of IDs = %d\n", gPerfIDList.size() - 1);  // Don't count Thread PerfID
	for(idx = 0; idx < gPerfIDList.size(); idx++) {
		idReport[idx].szName		= FindPerfDataByIdx(idx)->szName;
		idReport[idx].szCategory	= FindPerfDataByIdx(idx)->szCategory;
//		cout << "Setting up ID report Name = " << idReport[idx].szName;
//		cout << " Category = " << idReport[idx].szCategory << endl;
	}
	GenerateReport((void*)&idReport[0], gPerfIDList.size(), IDReportType);
	// Sort the data
	SortIDByTotalCalls(&idReport[0], gPerfIDList.size());

#ifdef WRITE_REPORT_TO_FILE
	WriteIDReportToFile(&idReport[0], gPerfIDList.size());
#endif

#ifdef WRITE_REPORT_TO_SCREEN
	cout << "\n\nID Report " << endl;
	cout << "Name\t\t\t\t\t\tSamples\t\tTotal\t\tSelf\t\tMin\t\tMax\t\tAvg\t\tCategory";
#ifdef DISPLAY_CPU_TOTALS
	cout << "\t\tCPU Total\t\tSelf\t\tMin\t\tMax\t\tAvg";
#endif
	cout << endl;
	cout << "----------------------------------------------------------------------------------------------";
	cout << "----------------------------------------------------------";
#ifdef DISPLAY_CPU_TOTALS
	cout << "---------------------------------------------------------------------------------------------";
#endif
	cout << endl;
	for(idx = 0; idx < gPerfIDList.size(); idx++) {
		if(idReport[idx].nSamples > 0) {
			cout << idReport[idx].szName;
			if(strlen(idReport[idx].szName) < 8) {
				cout << "\t\t\t\t\t\t";
			}
			else if(strlen(idReport[idx].szName) < 16) {
				cout << "\t\t\t\t\t";
			}
			else if(strlen(idReport[idx].szName) < 24) {
				cout << "\t\t\t\t";
			}
			else if(strlen(idReport[idx].szName) < 32) {
				cout << "\t\t\t";
			}
			else if(strlen(idReport[idx].szName) < 40) {
				cout << "\t\t";
			}
			else {
				cout << "\t";
			}
			cout << idReport[idx].nSamples;
			idReport[idx].nSamples >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nTotalTime / 1000.0;
			idReport[idx].nTotalTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nSelfTime / 1000.0;
			idReport[idx].nSelfTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nMinTime / 1000.0;
			idReport[idx].nMinTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nMaxTime / 1000.0;
			idReport[idx].nMaxTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nAvgTime / 1000.0;
			idReport[idx].nAvgTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].szCategory;

#ifdef DISPLAY_CPU_TOTALS
			cout << "\t\t";
			cout << idReport[idx].nTotalCPUTime * 1000.0;
			idReport[idx].nTotalCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nSelfCPUTime * 1000.0;
			idReport[idx].nSelfCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nMinCPUTime * 1000.0;
			idReport[idx].nMinCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nMaxCPUTime * 1000.0;
			idReport[idx].nMaxCPUTime >= MAX_REPORT_NUMBER_TAB ? cout << "\t" :  cout << "\t\t";
			cout << idReport[idx].nAvgCPUTime * 1000.0;
#endif
			cout << endl;
		}
	}		
#endif // WRITE_REPORT_TO_SCREEN

#ifdef WRITE_REPORT_TO_SCREEN
	cout << "\n\nNode Tree Report " << endl;
	GenerateReport((void*)NULL, 0, TreeReportType);
	cout << endl;
	cout << endl;
#endif // WRITE_REPORT_TO_SCREEN

#ifdef WRITE_REPORT_TO_FILE
	WriteTreeReportToFile();
#endif

	return true;
}
/* Record an entry point */
bool PerfMetrics::PerfEntry ( PerfID id )
{
	PerformanceRec* pCurrentRecord	= NULL;
	ThreadRecord*	pActiveThread	= NULL;
	pthread_t 		currentThread 	= pthread_self();
	
	// We have stopped don't collect any more data
	if(gEndTime != 0) {
		return false;
	}

	list<ThreadRecord*>::reverse_iterator iter 	= mThreadList.rbegin();
	
	// Start at the end as this is likely to match first
	while(iter != mThreadList.rend()) {
		if((*iter)->GetThreadID() == currentThread) {
			break;
		}
		iter++;
	}
	if(iter == mThreadList.rend()) {
		// Add new thread
//		cout << "New Thread ID " << currentThread << endl;
		pActiveThread = new ThreadRecord();
		mThreadList.push_back(pActiveThread);
		if(gPERF_ID_THREAD_START == INVALID_PERF_ID ) {
			// First thread
			PerfIDData* pPerfData 	= new PerfIDData();
			pPerfData->szName		= strdup("ThreadStart");
			pPerfData->szCategory 	= strdup("THREAD");
			pPerfData->id 			= GetUniqueID();
			pPerfData->categoryID	= GetUniqueID();
			gPerfIDList.push_back(pPerfData);
			// Save the thread ID.
			gPERF_ID_THREAD_START 		= pPerfData->id;
			gPERF_CATID_THREAD_START 	= pPerfData->categoryID;
		}
		// Add a root node for the tread start.
		// The root node only has one entry and exit
		pCurrentRecord = NewPerfRecord(gPERF_ID_THREAD_START, gPERF_CATID_THREAD_START, PerfRecord, NULL, currentThread);
		pActiveThread->SetRootNode((Node*)pCurrentRecord);
		pActiveThread->SetCurrentNode((Node*)pCurrentRecord);	
		pCurrentRecord->AddEntry();
//		cout << "Adding root node " << (void*)pCurrentRecord << " ID = " << (unsigned long)pCurrentRecord->GetID() << endl;
	}
	else {
		pActiveThread = *iter;
	}

	// Get the current node
	Node* pNode = pActiveThread->GetCurrentNode();
//	cout << "Using threadID " << pActiveThread->GetThreadID() << " Looking for ID " << id << endl;
//	cout << "Current Node = " << pNode << endl;

	if(pNode->GetNodeType() == PerfRecord) {
		list<Node*>::iterator 	childIter	= pNode->GetSiblingIterator();
		PerformanceRec* 		pChild 		= NULL;

		// Found the current record, now does it have this ID as a child already?
		pCurrentRecord = (PerformanceRec*)pNode;

		if(pNode->IsSiblingEnd(childIter) == true) {
			// There are no current child nodes
			// Add a new node
			PerfID catID = FindPerfDataByPerfID(id)->categoryID;
			pChild = NewPerfRecord(id, catID, PerfRecord, pCurrentRecord, currentThread);
			pCurrentRecord->AddSibling(pChild);
//			cout << "No children, adding first child node " << pChild << " ID = " << pChild->GetID() << endl;
//			cout << "Parent =  " << pCurrentRecord << " Parent ID = " << pCurrentRecord->GetID() << endl;
		}
		else {
			// Find the ID in the list of children
			Node* pChildren	= *childIter;
			if(pChildren->GetNodeType() == PerfRecord) {
				pChild = (PerformanceRec*)pChildren;
				while(pNode->IsSiblingEnd(childIter) == false) {
					pChildren	= *childIter;
					if(pChildren->GetNodeType() == PerfRecord) {
						pChild = (PerformanceRec*)pChildren;
//						cout << "Found a Child.  ID = " << pChild->GetID() << endl;
						if(id == pChild->GetID()) {
//							cout << "Child ID matches entry ID using this node " << endl;
							break;
						}
					}
					else {
						pChildren = NULL;
					}
					// Move to the next child
					childIter++;
				}
				if(pNode->IsSiblingEnd(childIter) == true) {
					// End of list
					PerfID catID = FindPerfDataByPerfID(id)->categoryID;
					pChild = NewPerfRecord(id, catID, PerfRecord, pCurrentRecord, currentThread);
					pCurrentRecord->AddSibling(pChild);
//					cout << "Could not find child ID adding new one " << endl;
//					cout << "\tNew child " << pChild << " ID = " << pChild->GetID() << endl;
//					cout << "\tParent =  " << pCurrentRecord << " Parent ID = " << pCurrentRecord->GetID() << endl;
				}
			}
		}
		if(pChild != NULL) {
			pActiveThread->SetCurrentNode((Node*)pChild);
			pChild->AddEntry();
//			cout << "Setting current node to " << pChild << " ID = " << pChild->GetID() << endl;
		}
		else {
			return false;
		}
	}
	else {
		// Error
		return false;
	}
	
	return true;
}
/* Record an exit point */
bool PerfMetrics::PerfExit ( PerfID id )
{
	PerformanceRec* pCurrentRecord	= NULL;
	ThreadRecord*	pActiveThread	= NULL;
	pthread_t 		currentThread 	= pthread_self();
	
	// We have stopped don't collect any more data
	if(gEndTime != 0) {
		return false;
	}

	list<ThreadRecord*>::reverse_iterator iter 	= mThreadList.rbegin();
	
	// Start at the end as this is likely to match first
	while(iter != mThreadList.rend()) {
		if((*iter)->GetThreadID() == currentThread) {
			break;
		}
		iter++;
	}
	if(iter != mThreadList.rend()) {
		pActiveThread = *iter;
	}
	else {
		//Error.  How can we be exiting when we don't have the thread on record.
		cout << "ERROR: PerfMetrics::PerfExit could not find Thread" << endl;
		return false;
	}

	// Get the current node
	Node* pNode = pActiveThread->GetCurrentNode();
	if(pNode->GetNodeType() == PerfRecord) {
		pCurrentRecord = (PerformanceRec*)pNode;
		if(pCurrentRecord->GetID() != id) {
			// Error
			cout << "ERROR: PerfMetrics::PerfExit could not find ID (" << (unsigned int)id << ") current record id = " << (unsigned int)pCurrentRecord->GetID() << endl;
			return false;
		}
		pCurrentRecord->AddExit();
		if(pActiveThread->GetRootNode() != pNode) {
//			cout << "Exiting, setting current node to " << pNode->GetParent() << endl;
			pActiveThread->SetCurrentNode(pNode->GetParent());
		}
	}
	else {
		cout << "pActiveThread->GetCurrentNode()->GetNodeType() returned " << (int)pNode->GetNodeType() << endl;
	}
	return true;
}

bool PerfMetrics::PerfEntry(const char * szName,  const char * szCategory)
{
	PerfID								id		= INVALID_PERF_ID;
	PerfID								catID	= INVALID_PERF_ID;
	list<PerfIDData*>::iterator 		iter 	= gPerfIDList.begin();
	list<PerfCategoryData*>::iterator 	catIter = gPerfCatList.begin();

	// We have stopped don't collect any more data
	if(gEndTime != 0) {
		return false;
	}

	// Is this a new category
	while(catIter != gPerfCatList.end()) {
		PerfCategoryData* pPerfCatData = *catIter;
		if(strcmp(pPerfCatData->szName, szCategory) == 0) {
			catID = pPerfCatData->nID;
//			cout << "Found entry for " << szCategory << endl;
			break;
		}
		catIter++;
	}
	if(catIter == gPerfCatList.end()) {
		PerfCategoryData* pPerfCatData = new PerfCategoryData();
		pPerfCatData->szName 	= strdup(szCategory);
		pPerfCatData->nID 		= GetUniqueID();
		gPerfCatList.push_back(pPerfCatData);
		catID = pPerfCatData->nID;

	}


	// Does this name/cat pair exist already?
	while(iter != gPerfIDList.end()) {
		PerfIDData* pPerfData = *iter;
		if(strcmp(pPerfData->szCategory, szCategory) == 0) {
			if(strcmp(pPerfData->szName, szName) == 0) {
				id = pPerfData->id;
//				cout << "Found entry id for " << szName << ", " << szCategory << endl;
				break;
			}
		}
		iter++;
	}
	// If not create a new entry
	if(iter == gPerfIDList.end()) {
		PerfIDData* pPerfData = new PerfIDData();
		pPerfData->szName		= strdup(szName);
		pPerfData->categoryID	= catID;
		pPerfData->szCategory 	= szCategory;
		pPerfData->id 			= GetUniqueID();
		gPerfIDList.push_back(pPerfData);
		id = pPerfData->id;
//		cout << "Created new entry id " << (unsigned long)pPerfData->id<< " for " << szName << ", " << szCategory << " Cat ID " << (int)pPerfData->categoryID << endl;
//		cout << "ID list is " << gPerfIDList.size() << " elements long" << endl;
	}
	// Record the entry point
	return PerfEntry(id);
}
bool PerfMetrics::PerfExit(const char * szName, const char * szCategory)
{
	PerfID							id		= INVALID_PERF_ID;
	list<PerfIDData*>::iterator 	iter 	= gPerfIDList.begin();

	// We have stopped don't collect any more data
	if(gEndTime != 0) {
		return false;
	}

	// Does this name/cat pair exist already?
	while(iter != gPerfIDList.end()) {
		// Does it have the same category
		PerfIDData* pPerfData = *iter;
		if(strcmp(pPerfData->szCategory, szCategory) == 0) {
			if(strcmp(pPerfData->szName, szName) == 0) {
//				cout << "Found exit id for " << szName << ", " << (int)eCategory << endl;
				id = pPerfData->id;
				break;
			}
		}
		iter++;
	}
	// If not, this is an error
	if(iter == gPerfIDList.end()) {
		cout << endl << "ERROR: PerfExit with no matching PerfEntry!!!";
		cout << " Name = " << szName << " Category = " << szCategory << endl << endl;
		return false;
	}

	// Record the exit point
	return PerfExit(id);
}
bool PerfMetrics::PerfAlloc(void* addr, int size)
{
#ifdef PERFORMANCE_MEMORY    
    AllocRecord* pRec = new AllocRecord(addr, size);
    mAllocMap.insert(AllocPair(pRec));
    mAllocCurrent += size;
    if (mAllocCurrent > mAllocMax) {
        mAllocMax = mAllocCurrent;
    }
#endif
    return true;
}

bool PerfMetrics::PerfFree(void* addr)
{
#ifdef PERFORMANCE_MEMORY    
	if(mAllocMap.size() > 0) {
		AllocRecord* pRec = mAllocMap.at((unsigned int)addr);
		if (pRec) {
			mAllocMap.erase((unsigned int)addr);
			mAllocCurrent -= pRec->GetSize();
			delete pRec;
			return true;
		} else {
			//cout << "WARNING!  FREEING NON-ALLOCATED MEMORY AT " << (unsigned int)addr << endl;
			return false;
		}
	} else {
		//cout << "WARNING!  FREEING MEMORY when allocation map is empty ptr = " << (unsigned int)addr << endl;
	}
#endif
	return false;
}
PerfID PerfMetrics::GetUniqueID()
{
	pthread_mutex_lock(&lock);
	nID++;
	pthread_mutex_unlock(&lock);
	return nID;
}


PerfFunction::PerfFunction(PerfID id)
: m_id(id)
, m_szName(NULL)
, m_szCategory(NULL)
{
	PerfMetrics::PerfEntry(m_id);
}
PerfFunction::PerfFunction(const char * szName,  const char * szCategory)
: m_id(INVALID_PERF_ID)
, m_szName(szName)
, m_szCategory(szCategory)
{
	PerfMetrics::PerfEntry(szName, szCategory);
}
PerfFunction::~PerfFunction()
{
	if(m_id != INVALID_PERF_ID)
		PerfMetrics::PerfExit(m_id);
	else
		PerfMetrics::PerfExit(m_szName, m_szCategory);
}

#endif // FEATURE_PERFORMANCE_PROFILING

