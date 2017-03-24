#!/bin/bash

opts="--firing_rate_min=0.5 --firing_rate_max=3 --M=4 --duration=3600"

./generate_synth_examples.node.js $opts --dsname=example1 --K=8 --noise_level=1
