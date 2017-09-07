#!/bin/bash

#if [ -z "$1" ]; then
#    echo ""
#    echo "usage:"
#    echo "./compile_components.sh default"
#    echo "./compile_components.sh mountainprocess"
#    echo "example components: mda mdaconvert mountainprocess mountainsort prv mda"
#    exit 0
#fi

# verify if we're running Qt4 or Qt5 qmake
QT_VERSION=`qmake -query QT_VERSION`
if [[ "$QT_VERSION" =~ ^4.*$ ]]
then
  echo "You're trying to build MountainLab with Qt4 ($QT_VERSION) but MountainLab only supports Qt5"
  echo "Please make sure qmake from Qt5 installation is first in your PATH."
  
  # try to find a Qt5 install in $HOME or /opt
  if [ -d "/opt/Qt" ]
  then
    ls -1 /opt/Qt/|grep -sqe '^5\.[[:digit:]]$'
    if [[ "$?" == "0" ]]
    then
      echo "It looks like Qt5 installation(s) might be present in /opt/Qt/"
      echo
      exit 1
    fi
  fi
  if [ -d "$HOME/Qt" ]
  then
    ls -1 "$HOME/Qt"|grep -sqe '^5\.[[:digit:]]$'
    if [[ "$?" == "0" ]]
    then
      echo "It looks like Qt5 installation(s) might be present in $HOME/Qt"
      echo
      exit 1
    fi
  fi
  echo "If you don't have Qt5 installed, please go to http://qt.io and download Qt5"
  echo
  exit 1
fi

args0=""
components0=""
for var in "$@"; do
	if [[ "$var" == "default" ]]; then
		echo ""
	else
		components0="$components0 $var"
	fi

    # witold, i think we can remove this line, correct?
    [ "$var" == 'nogui' ] && ARGS+=("$var")
done

qmake -recursive

if [ -z "$components0" ]; then
eval qmake $args0
else
eval qmake \"COMPONENTS = $components0\" $args0
fi

sha1sum_output_before=$(sha1sum cpp/mountainprocess/bin/mountainprocess)

NPROCCMD=$(which nproc)

if [ -z "$NPROCCMD" ]
then
  $NPROCCMD="lscpu|awk '/^CPU\(s\)/ { print $2; }'"
fi

NPROC=$($NPROCCMD)

if [ -z "$NPROC" ]
then
  NPROC="2"
fi
echo "Building with $NPROC parallel jobs"
make -j $NPROC
EXIT_CODE=$?
if [[ $EXIT_CODE -ne 0 ]]; then
	echo "Problem in compilation."
else
	make install # added by jfm on 9/7/17
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

	sha1sum_output_after=$(sha1sum cpp/mountainprocess/bin/mountainprocess)
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

if [ -f mountainlab.user.json ]; then
	echo "******************************************"
	echo "It appears you are in a directory that used to"
	echo "be occupied by an older version of mountainlab."
	echo "I suspect that because mountainlab.user.json is"
	echo "in the old location. It is highly recommended that"
	echo "you do a fresh clone of mountainlab, checkout this"
	echo "branch and then compile. Questions? Ask Jeremy."
	echo "******************************************"
fi

