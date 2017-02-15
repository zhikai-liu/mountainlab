#!/bin/bash

opts="--firing_rate_min=0.5 --firing_rate_max=3 --M=4 --duration=1200"

./generate_synth_examples.node.js $opts --dsname=example1 --session=session1 --K=14 --noise_level=1
./generate_synth_examples.node.js $opts --dsname=example1 --session=session2 --K=6 --noise_level=1
