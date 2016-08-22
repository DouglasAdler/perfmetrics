lib_LIBRARIES = libperfmetrics.a

libperfmetrics_a_SOURCES = 	Node.cpp \
				PerfMetrics.cpp \
				PerformanceRec.cpp \
				ThreadRecord.cpp

AM_CXXFLAGS = -DSEC_TARGET_LOCAL -Wall -Werror -Wfatal-errors -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-value -fPIC -fdata-sections -ffunction-sections -lpthread -O3

AM_CXXFLAGS += -I../include/

