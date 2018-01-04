#!/bin/sh

TARGET_ROOT_DIR=total

for CURR_DIR in ./*-anal; do
	for CURR_FILE in $CURR_DIR/*; do
		if [ -f $CURR_FILE ]
			touch ${CURR_FILE}
			cat $CURR_FILE >> ${TARGET_ROOT_DIR}/${CURR_FILE}
		fi 
	done
done
