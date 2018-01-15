
NAMES=(
"DOWNLOAD_RATE.txt" "UPLOAD_1-10MB.txt" 
"DOWNLOAD_SIZE.txt" "UPLOAD_DURATION.txt" 
"DWLOAD_0-0.1MB.txt" "(TIMESTP,DURATION).txt" 
"UPLOAD_RATE.txt" "DWLOAD_0.1-1MB.txt" 
"(TIMESTP,SIZE).txt" "UPLOAD_SIZE.txt" 
"DWLOAD_10+MB.txt" "UPLOAD_0-0.1MB.txt" 
"DWLOAD_1-10MB.txt" "UPLOAD_0.1-1MB.txt" 
)

TARGET_ROOT_DIR=total
BIN_PATH=/media/xia/Soar1/tcpdump-new/analysiser/bin
mkdir -p $TARGET_ROOT_DIR

#MERGE FLOW DATA
for CURR_DIR in ./*-anal; do
	for CURR_FILE in ${NAMES[@]}; do
		if [ -f $CURR_DIR/$CURR_FILE ]; then
			echo "copying ${CURR_DIR}/${CURR_FILE} into ${TARGET_ROOT_DIR}/${CURR_FILE}"
			touch ${TARGET_ROOT_DIR}/${CURR_FILE}
			cat $CURR_DIR/$CURR_FILE >> ${TARGET_ROOT_DIR}/${CURR_FILE}
		fi 
	done
done

#MERGE HTTP AND HTTPS DATA
touch dir.txt
ls | awk '{print i$0}' i=`pwd`'/' | grep anal > dir.txt
${BIN_PATH}/merge_data dir.txt http 2&>1 > /dev/null
${BIN_PATH}/merge_data dir.txt https 2$>1 > /dev/null 

