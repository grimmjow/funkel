#!/bin/bash

count=1;
max=32;

while [ $count -le $max ]; do

	echo "#################"
	echo $count/$max
	echo "#################"
	echo ""

	./build.sh
	rc=$?

	if [ "$rc" -ne 0 ]; then

		while [ $rc != 0 ]; do
			echo "failed, retry [Y/n]?"
			read entry
			if [ "$entry" != "n" ]; then
				./build.sh
				rc=$?
			else
				rc=0
			fi
		done
	else
		echo "done, [enter] for next"
		read entry
	fi

	count=$(( count + 1))
done

