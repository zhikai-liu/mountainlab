#!/bin/bash

octave $1

# Be sure to install docker (sudo apt install docker.io)
# And add the user to the docker group so that sudo is not required to run docker containers

#container_name=$(cat /dev/urandom | tr -dc 'a-z' | fold -w 16 | head -n 1)
#DIR=$(dirname $0)
#image_tag=$(cat /dev/urandom | tr -dc 'a-z' | fold -w 16 | head -n 1)
#docker build -t $image_tag $DIR
#docker run -v /:/host --name=$container_name $image_tag /bin/bash -c "useradd -u $UID -m user; su user -c 'octave $1'"
#docker rm $container_name # clean up
#docker tag $image_tag reserve # important to do this before removing image
#docker rmi $image_tag # clean up
