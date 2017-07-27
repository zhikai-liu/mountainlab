#!/bin/bash

DIR=$(dirname $0)
rand_id=$(cat /dev/urandom | tr -dc 'a-z' | fold -w 16 | head -n 1)
tmp='/tmp'
script_path="$tmp/run_in_octave_$rand_id.m"
echo $1 > $script_path
eval $DIR/run_octave_script.sh $script_path; rm $script_path


