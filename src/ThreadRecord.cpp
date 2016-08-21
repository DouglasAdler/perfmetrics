#include "ThreadRecord.h"

ThreadRecord::ThreadRecord()
{
	mTree 			= NULL;
	mCurrentNode	= NULL;
	mThreadID		= (pthread_t)pthread_self();
}

ThreadRecord::~ThreadRecord()
{
	if(mTree != NULL) {
		delete mTree;
		mCurrentNode = NULL;
	}
}

bool ThreadRecord::SetRootNode(Node* pRoot)
{
	mTree = pRoot;
	return true;
}
Node* ThreadRecord::GetRootNode()
{
	return mTree;
}
bool ThreadRecord::SetCurrentNode(Node* pCurrent)
{
	mCurrentNode = pCurrent;
	return true;
}
Node* ThreadRecord::GetCurrentNode()
{
	return mCurrentNode;
}
pthread_t ThreadRecord::GetThreadID()
{
	return mThreadID;
}
