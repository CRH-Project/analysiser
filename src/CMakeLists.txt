CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(Analysiser-src)

SET(INCLUDES headers.h utils.h spliter.h
	)
SET(SOURCES utils.c spliter.c
	)

ADD_LIBRARY(src_lib STATIC ${SOURCES} ${INCLUDES})
