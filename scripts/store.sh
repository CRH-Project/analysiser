DOWN=DOWNLOAD
UP=UPLOAD
for FILE in *.pcap; do
	FD=${FILE}${DOWN}
	FU=${FILE}${UP}
	mkdir -p ${FD}
	mkdir -p ${FU}
	for file in ${FILE}-D*; do
	   mv $file	${FD}
    done 
	for file in ${FILE}D*; do
	   mv $file	${FD}
    done 
	for file in ${FILE}-U*; do
	   mv $file	${FU}
    done 
	for file in ${FILE}U*; do
	   mv $file	${FU}
    done 
done
