#include <iostream>

#include "Node.h"


Node::Node()
{
	mType	= UnknownType;
	mParent = NULL;
	mSiblingList.clear();
	return;
}

Node::~Node()
{
	Node* pNode = NULL;
	
	while(!mSiblingList.empty()) {
		pNode = mSiblingList.front();
		delete pNode;
		
		mSiblingList.pop_front();
	}
	return;
}


bool Node::SetParent(Node* pNode)
{
	mParent = pNode;
	return true;
}
Node* Node::GetParent()
{
	return mParent;
}

bool Node::AddSibling(Node* pNode)
{
//	cout << "Node " << this << " Adding sibling " << pNode << endl;
	mSiblingList.push_back(pNode);
	return true;
}
Node* Node::GetNextSibling(Node* pCurrentSibling)
{
	list<Node*>::iterator iter = mSiblingList.begin();
	
	if(pCurrentSibling == NULL) {
		return mSiblingList.front();
	}
	
	while(iter != mSiblingList.end()) {
		if(*iter == pCurrentSibling) {
			break;
		}
		iter++;
	}
	if(iter == mSiblingList.end()) {
		return NULL;
	}
	// Get the NEXT sibling
	iter++;
	if(iter == mSiblingList.end()) {
		return NULL;
	}
	else {
		return *(iter);
	}
}
list<Node*>::iterator Node::GetSiblingIterator()
{
	return mSiblingList.begin();
}

bool Node::IsSiblingEnd(list<Node*>::iterator iter)
{
	if(iter == mSiblingList.end()) {
		return true;
	}
	return false;
}
bool Node::SetNodeType(NodeType type)
{
	mType = type;
	return true;
}
NodeType Node::GetNodeType()
{
	return mType;
}
