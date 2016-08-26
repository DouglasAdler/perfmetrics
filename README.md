# perfmetrics

The performance metrics library is useful for determining call stack and module performance.  

To make the library do the following:

```
autoreconf --verbose --install --force
configure 
make all
```
To start the profiling add this to the entry point of your program.
```
 #ifdef FEATURE_PERFORMANCE_PROFILING
            PERF_START();
            PERF_ENTRY("Root", "ROOT");
 #endif //FEATURE_PERFORMANCE_PROFILING
 ```
 and this to the exit point..
 
 ```
 #ifdef FEATURE_PERFORMANCE_PROFILING
            PERF_EXIT("Root", "ROOT");
            PERF_STOP();
            PERF_REPORT();
            PERF_CLEANUP();
#endif //FEATURE_PERFORMANCE_PROFILING
```

and then anywhere you want to profile can be instrumented with

```
PERF_FUNC(__PRETTY_FUNCTION__, "DRAW");
```
Where DRAW is the category you want to categorize this function under.  Or you can use the _ENTRY and _EXIT macros to work within a particular function..

The first parameter is the ID for this element and it’s a string.  The library will match and entry and exit point based of the category and ID so you need matched _ENTRY and _EXIT macros.
The PERF_STOP will stop collecting data and the PERF_REPORT will print the report to STDOUT and save it to a CSV file if you have that feature enabled.
The library will also track CPU clock as well as wall clock, but that is system dependent and in the systems that I have tried it on the resolution is 10ms so it's only really useful for functions that take a long time.  Otherwise you get a lot of 10 and 0 entries.
