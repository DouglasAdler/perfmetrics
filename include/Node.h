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

#ifndef NODE_H_
#define NODE_H_

#include <list>

using namespace std;

typedef enum NodeType_e
{
	PerfRecord,
	UnknownType
} NodeType;

class Node
{
public:
	Node();
	virtual ~Node();
	
	bool SetParent(Node* pMode);
	Node* GetParent();
	bool AddSibling(Node* pMode);
	Node* GetNextSibling(Node* pCurrentSibling);
	list<Node*>::iterator GetSiblingIterator();
	list<Node*>* GetSiblingList() { return &mSiblingList; }
	bool IsSiblingEnd(list<Node*>::iterator iter);

	bool SetNodeType(NodeType type);
	NodeType GetNodeType();
	
private:
	NodeType		mType;
	list<Node*>		mSiblingList;
	Node*			mParent;
};

#endif /*NODE_H_*/
