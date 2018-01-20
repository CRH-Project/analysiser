PROJECT(Analysiser)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#ADD_COMPILE_OPTIONS(-Ofast)
ADD_COMPILE_OPTIONS(-g)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

FIND_LIBRARY(PCAP pcap REQUIRED)


ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(simulator)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/simulator)

#TODO: ADD_EXECUTABLES AND TARGET_LINK_LIBRARIES

ADD_EXECUTABLE(unpcap unpcap.c)			#UNPCAP
TARGET_LINK_LIBRARIES(unpcap ${PCAP})
ADD_EXECUTABLE(flowAnal flowAnal.cpp)	#FLOW ANALYSIS
TARGET_LINK_LIBRARIES(flowAnal cxx_src_lib c_src_lib ${PCAP})
ADD_EXECUTABLE(tcpflow tcpflow.c)		#TCP ANALYSIS @DEPRECATED
TARGET_LINK_LIBRARIES(tcpflow ${PCAP})
ADD_EXECUTABLE(http http.cpp)			#HTTP ANALYSIS
TARGET_LINK_LIBRARIES(http cxx_src_lib ${PCAP})
ADD_EXECUTABLE(split split.cpp)			#SPLITER
TARGET_LINK_LIBRARIES(split c_src_lib cxx_src_lib ${PCAP})
ADD_EXECUTABLE(merge_data merge_data.cpp)	#FOR DATA MERGE
TARGET_LINK_LIBRARIES(merge_data c_src_lib cxx_src_lib)
ADD_EXECUTABLE(percentage percentage.cpp)
TARGET_LINK_LIBRARIES(percentage cxx_src_lib ${PCAP})

ADD_EXECUTABLE(sim sim.cpp)				#SIMULATOR
TARGET_LINK_LIBRARIES(sim 
	cxx_src_lib cxx_sim_lib c_src_lib
	${PCAP})


INSTALL(TARGETS 
			unpcap flowAnal tcpflow http split merge_data sim
			percentage
		DESTINATION bin)
