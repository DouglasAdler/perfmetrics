#ifndef THREADRECORD_H_
#define THREADRECORD_H_

#include <pthread.h>

#include "PerfMetrics.h"
#include "Node.h" 

class ThreadRecord
{
public:
	ThreadRecord();
	virtual ~ThreadRecord();
	
	bool		SetRootNode(Node* pRoot);
	Node* 		GetRootNode();
	bool		SetCurrentNode(Node* pCurrent);
	Node* 		GetCurrentNode();
	pthread_t	GetThreadID();
	
	
private:
	Node*		mTree;
	Node*		mCurrentNode;
	pthread_t	mThreadID;
};

#endif /*THREADRECORD_H_*/
