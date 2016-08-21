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
