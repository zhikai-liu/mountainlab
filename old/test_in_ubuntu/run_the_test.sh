#!/bin/bash

# I think this should be run using sudo, because it uses docker

# Build the image
docker build -t test1 .

# Run the script in a container
docker run -v $PWD:/base -t test1 bash -c "/bin/bash /base/run_in_docker.sh"

# interactive session would be this:
#sudo docker run -it test1 /bin/bash

# Now it should have created the output directory
