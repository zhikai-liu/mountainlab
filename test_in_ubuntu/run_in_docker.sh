#!/bin/bash

# Create user
useradd magland
mkdir /home/magland
cd /home/magland
git clone https://github.com/magland/mountainlab.git
cd mountainlab
export PATH=/home/magland/mountainlab/bin:$PATH
./compile_components.sh
cd examples/003_kron_mountainsort
./001_generate_synthetic_data.sh

# Twice? TODO: fix this bug
mp-daemon-start magland
mp-daemon-start magland

kron-run ms2 example1
cp -r output /base/
