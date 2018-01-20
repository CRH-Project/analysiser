CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Analysiser-src)

SET(INCLUDES headers.h utils.h spliter.h flow.h httpTrace.h
			 roll.h domain_stat.h dns_trace.h flow_percent.h
	)
SET(C_SOURCES utils.c spliter.c
	)
SET(CXX_SOURCES flow.cpp roll.cpp httpTrace.cpp utils.cpp
				domain_stat.cpp dns_trace.cpp flow_percent.cpp
	)

ADD_LIBRARY(c_src_lib STATIC ${C_SOURCES} ${INCLUDES})
ADD_LIBRARY(cxx_src_lib STATIC ${CXX_SOURCES} ${INCLUDES})
