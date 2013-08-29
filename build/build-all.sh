#!/bin/bash

count=0;
max=15;

while [ $count -le $max ]; do

	echo "#################"
	echo $count/$max
	echo "#################"
	echo ""

	./build.sh -l $count
	rc=$?

	if [ "$rc" -eq 0 ]; then
    	count=$(( count + 1))
		echo "done, [enter] for next"
		read entry
    else
    	echo "failed, retry [Y/n]?"
		read entry
		if [ "$entry" == "n" ]; then
        	count=$(( count + 1))
		fi
	fi

done

