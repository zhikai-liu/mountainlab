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

if [[ $1 == "default" ]]; then
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

	RES=$(which mountainprocess)
	if [[ -z $RES ]]; then
		echo ""
		echo "******************************************"
		echo "It appears that mountainprocess is not"
		echo "found in your path. Be sure to add"
		echo "mountainlab/bin to your path. See the"
		echo "installation instructions for more"
		echo "information."
		echo "******************************************"
	fi

	sha1sum_output_after=$(sha1sum mountainprocess/bin/mountainprocess)
	if [[ $sha1sum_output_before != $sha1sum_output_after ]]; then
		output=$(bin/mp-daemon-state-summary)
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
