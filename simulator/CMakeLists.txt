CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Analysiser-Simulator)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../src)

SET(INCLUDES 
		sim.h simlib.h sample.h 
		FlowInfo.h
		scheduler.h
	)

SET(C_SOURCES
		
	)

SET(CXX_SOURCES
		simlib.cpp simcpp.cpp sample.cpp 
		FlowInfo.cpp
		simple_greddy.cpp
	)

LINK_LIBRARIES(cxx_src_lib)
#ADD_LIBRARY(c_sim_lib STATIC ${C_SOURCES} ${INCLUDES})
ADD_LIBRARY(cxx_sim_lib STATIC ${CXX_SOURCES} ${INCLUDES})


