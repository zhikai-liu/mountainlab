#!/bin/bash

# remove the output
rm -r /base/output

# Create user
useradd magland
mkdir /home/magland
cd /home/magland

# clone the repo
git clone https://github.com/magland/mountainlab.git

# Compile mountainlab
cd mountainlab
export PATH=/home/magland/mountainlab/bin:$PATH
./compile_components.sh

# Generate the synthetic data
cd examples/003_kron_mountainsort
./001_generate_synthetic_data.sh

# Start the processing daemon
mp-daemon-start magland
mp-daemon-start magland

# run the processing
kron-run ms2 example1

# create the output
cp -r output /base/
