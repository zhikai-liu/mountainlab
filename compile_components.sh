#!/bin/bash

#if [ -z "$1" ]; then
#    echo ""
#    echo "usage:"
#    echo "./compile_components.sh default"
#    echo "./compile_components.sh mountainview"
#    echo "example components: mdachunk mdaconvert mountainprocess mountainsort mountainview mountaincompare prv"
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

make -j 8
EXIT_CODE=$?
if [[ $EXIT_CODE -ne 0 ]]; then
	false
fi
