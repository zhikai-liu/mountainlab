#/bin/bash

sudo docker build .
sudo docker run -v /home/magland/prvdata:/prvdata -v $PWD/tmp:/tmp -v $PWD/mountainlab.user.json:/mountainlab/mountainlab.user.json test1 bash -c "prv locate --checksum=b8c71ab199c0ca14b8e40e786423307a2cd1cee6 --size=506925076 --local-only"
