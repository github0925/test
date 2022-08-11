#!/bin/bash

prjlst=$(ls project/safety*.mk ospi*.mk | awk -F/ '{print $NF}' | awk -F. '{print $1}')

for prj in $prjlst
do	
	echo "Building Project $prj"
	make $prj -j16
	if [ $? -ne 0 ]; then
		echo "====================== FAIL TO BUILD $prj ======================"
		exit 1
	fi
done

exit 0
