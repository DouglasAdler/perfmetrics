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
