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
