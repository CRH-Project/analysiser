recurse(){
	DIR=$1
	for FILE in ${DIR}/*; do
		if [[ ${FILE} != *.pcap ]]; then
			if [[ ${FILE} != *.sh ]]; then
				if [[ -f ${FILE} ]]; then 
					mv ${FILE} ${FILE}${SUFFIX}
				elif [[ -d ${FILE} ]]; then 
					recurse ${FILE}
				fi
			fi
		fi
	done	
}

SUFFIX=.pcap
START_DIR=.
if [[ -n $1 ]]; then
	START_DIR=$1
fi
recurse ${START_DIR}


#for FILE in ./*; do
#	if [[ ${FILE} != *.pcap ]]; then
#		if [[ ${FILE} != *.sh ]]; then
#			mv ${FILE} ${FILE}${SUFFIX}
#		fi
#	fi
#done	
