#ifndef ALLOCRECORD_H_
#define ALLOCRECORD_H_

//
// Little class to hold the record of a memory allocation
//
class AllocRecord {
public:

    AllocRecord(void* addr, int size): m_addr(addr), m_size(size) { }
    ~AllocRecord() { }
    
    inline void* GetAddress()       { return m_addr; }
    inline unsigned long GetID()     { return (unsigned long)m_addr; }
    inline int GetSize()            { return m_size; }
    
private:
    void* m_addr;
    int m_size;
};
 
#endif /*ALLOCRECORD_H_*/
