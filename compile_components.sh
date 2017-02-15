#!/bin/bash

#if [ -z "$1" ]; then
#    echo ""
#    echo "usage:"
#    echo "./compile_components.sh default"
#    echo "./compile_components.sh mountainview"
#    echo "example components: mdachunk mdaconvert mountainprocess mountainsort mountainview mountaincompare prv mda"
#    exit 0
#fi

qmake -recursive

if [ $1 == "default" ]
then
eval qmake
elif [ -z "$1" ]; then
eval qmake
else
eval qmake \"COMPONENTS = $@\"
fi

sha1sum_output_before=$(sha1sum mountainprocess/bin/mountainprocess)

make -j 8
EXIT_CODE=$?
if [[ $EXIT_CODE -ne 0 ]]; then
	echo "Problem in compilation."
else
	echo ""
	echo "Compilation successful."

	sha1sum_output_after=$(sha1sum mountainprocess/bin/mountainprocess)
	if [[ $sha1sum_output_before != $sha1sum_output_after ]]; then
		output=$(mp-daemon-state-summary)
		if [[ $output == "Daemon is running"* ]]; then
			echo ""
			echo "******************************************"
			echo "It appears a processing daemon is running"
			echo "and the mountainprocess binary has changed."
			echo "You may want to restart the daemon using:"
			echo "mp-daemon-restart"
			echo "******************************************"
		fi
	fi
fi
