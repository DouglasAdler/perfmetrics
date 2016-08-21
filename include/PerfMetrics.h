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


#ifndef PERFMETRICS_H
#define PERFMETRICS_H

#define PERF_METRICS

/*
**---------------------------------------------------------------------
** Includes
**---------------------------------------------------------------------
*/
#include "performance_id.h"


/*
**-------------------------------------------------------------------------
**  Macro Definitions
**-------------------------------------------------------------------------
*/
#ifndef BEGIN_EXTERN_C
#ifdef  __cplusplus
#define BEGIN_EXTERN_C                  extern "C" {
#define END_EXTERN_C                    }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif
#endif

#ifndef UNUSED_VARIABLE
#define UNUSED_VARIABLE(x)              ((void)(x))
#endif

#define PERF_E_BADARG                   -1
#define PERF_E_NOMEM                    -2
#define PERF_NO                         0
#define PERF_OK                         1

#ifdef  __cplusplus

#endif // __cplusplus

/*
**---------------------------------------------------------------------
** Constants and Primary Macro Definitions
**---------------------------------------------------------------------
*/
#ifdef FEATURE_PERFORMANCE_PROFILING
#define PERF_START()                    (PerfMetrics::PerfStart())
#define PERF_STOP()                     (PerfMetrics::PerfStop())
#define PERF_REPORT()                   (PerfMetrics::PerfReport())
#define PERF_CLEANUP()                  (PerfMetrics::PerfCleanup())
#define PERF_ALLOC(a, s)                (PerfMetrics::PerfAlloc(a, s))
#define PERF_FREE(a)                    (PerfMetrics::PerfFree(a))
//#define PERF_FUNC(i)                    PerfFunction FuncMetric(i)
//#define PERF_ENTRY(i)                   (PerfMetrics::PerfEntry(i))
//#define PERF_EXIT(i)                    (PerfMetrics::PerfExit(i))
#define PERF_ENTRY(n, c)                (PerfMetrics::PerfEntry(n, c))
#define PERF_EXIT(n, c)                 (PerfMetrics::PerfExit(n, c))
#define PERF_FUNC(n, c)                 PerfFunction FuncMetric(n, c)
#else
#define PERF_START()
#define PERF_STOP()
#define PERF_REPORT()                   
#define PERF_CLEANUP()                 
#define PERF_ALLOC(a, s)
#define PERF_FREE(a)
//#define PERF_FUNC(i)
//#define PERF_ENTRY(i)
//#define PERF_EXIT(i)
#define PERF_ENTRY(n, c)
#define PERF_EXIT(n, c)
#define PERF_FUNC(n, c)
#endif

#define INVALID_PERF_ID 		0xffffffffUL

/*
**---------------------------------------------------------------------
** Type Definitions
**---------------------------------------------------------------------
*/
typedef unsigned long 	PerfID;


/*
**---------------------------------------------------------------------
** Prototypes
**---------------------------------------------------------------------
*/
#ifdef FEATURE_PERFORMANCE_PROFILING

#ifdef  __cplusplus


class PerfFunction
{
public:
	PerfFunction(PerfID id);
	PerfFunction(const char * szName,  const char * szCategory);
	virtual ~PerfFunction();
private:
	PerfID 			m_id;
	const char *	m_szName;
	const char * 	m_szCategory;
};

class PerfMetrics
{
public:
	PerfMetrics();
	virtual ~PerfMetrics();
	
	static bool PerfStart      ( void );
	static bool PerfStop       ( void );
	static bool PerfCleanup    ( void );
	static bool PerfReport     ( void );
	static bool PerfEntry      ( PerfID id );
	static bool PerfExit       ( PerfID id );	
	static bool PerfEntry      ( const char * szName,  const char * szCategory );
	static bool PerfExit       ( const char * szName,  const char * szCategory );
    static bool PerfAlloc      ( void* addr, int size );
    static bool PerfFree       ( void* addr );
private:
    static PerfID GetUniqueID  ( );
	
};
#else	// __cplusplus
#pragma message("PerfMetrics Class can not be used in a C file")
#endif // __cplusplus

//BEGIN_EXTERN_C
//
//extern int   PerfStart      ( void );
//extern int   PerfStop       ( void );
//extern int   PerfReport     ( void );
//extern int   PerfEntry      ( PerfID id );
//extern int   PerfExit       ( PerfID id );
//
//END_EXTERN_C

#endif 	// FEATURE_PERFORMANCE_PROFILING

#endif /* PERFMETRICS_H */
