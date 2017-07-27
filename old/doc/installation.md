### Supported operating systems:
Linux/Ubuntu and Debian are the currently supported development platforms. Other Linux flavors should also work. There are plans to support Mac, but Windows is not currently supported.

### There are four steps to installing MountainLab

* Install the prerequisites
* Clone and compile MountainLab
* Test the installation
* If necessary, contact Jeremy

### Step 1: Install the prerequisites

The prerequisites are:

* Qt5 (version 5.5 or later) – see below
* NodeJS
* FFTW
* Recommended: wget, rsync
* Optionally you can install Matlab or Octave

See below for details on installing these packages.

### Step 2: Clone and compile MountainLab

(You should be a regular user when you perform this step -- do not use sudo here or your files will be owned by root)

First time:

```bash
git clone https://github.com/magland/mountainlab.git
cd mountainlab
# check out a specific branch by name
git checkout ms3
./compile_components.sh
```

See also nogui_compile.sh to compile only the non-gui components, for example if you are running processing on a non-gui server.

Subsequent updates:

```bash
cd mountainlab
git pull
./compile_components.sh
```

Important: Add mountainlab/bin to your PATH environment variable. For example append the following to your ~/.bashrc file, and open a new terminal (or, source .bashrc):

```bash
export PATH=/path/to/mountainlab/bin:$PATH
```

Note: depending on how nodejs is named on your system, you may need to do the following (or something like it):
```bash
sudo  ln -s /usr/bin/node /usr/bin/nodejs
```

### Step 3: Test the installation

If you have Matlab or Octave installed, prepare the example spike sorting:

```bash
cd examples/003_kron_mountainsort
./001_generate_synthetic_data.sh
```

This will use matlab if you have it installed, otherwise it will use Octave. It will generate 5 example synthetic datasets in an examples subdirectory, and the raw data are written to the BIGFILES subdirectory. Thus we begin following the principle of separating large files from their contents, as is described in more detail elsewhere.

Note: if this command fails with error `/usr/bin/env: nodejs: No such file or directory`, you will need to do the symbolic nodejs link given above.

Run the standard mountainsort processing pipeline:

```bash
kron-run ms3 example1 --_nodaemon
```

Finally, launch the viewer:

```bash
kron-view results ms3 example1
```

See the other docs for details on what's going on here, and try to [sort your own data](the_first_sort.md).

### Step 4: If necessary, contact Jeremy

I'm happy to help, and we can improve the docs. I'm also happy to invite you to the slack team for troubleshooting, feedback, etc.

### Prerequisite: Install Qt5

If you are on a later version of Ubuntu (such as 16.04), you can get away with installing Qt5 using the package manager (which is great news). Otherwise, that method may not give a recent enough Qt5 version. In that case (for example if you are on 14.04, or using Mac or other Linux flavor), you should install Qt5 via qt.io website (see below).

If you've got Ubuntu 16.04 or later (good news):

```bash
sudo apt-get install software-properties-common
sudo apt-add-repository ppa:ubuntu-sdk-team/ppa
sudo apt-get update
sudo apt-get install qtdeclarative5-dev
sudo apt-get install qt5-default qtbase5-dev qtscript5-dev make g++
```

### Prerequisite: Install FFTW, NodeJS, and (optionally) octave

```bash
sudo apt-get install libfftw3-dev nodejs npm
sudo apt-get install octave

```

### Prerequisite: If necessary, install Qt5 from qt.io

As mentioned above, if you are not using a later version of Ubuntu, you probably won't get a recent enough version from the package manager. In that case follow these directions to install a recent version of Qt5:

Go to https://www.qt.io/download/ and click through that you want to install the open source version. Download and run the appropriate installer. Note that you do not need to set up a Qt account -- you can skip that step in the install wizard.

We recommend installing this in your home directory, which does not require admin privileges.

Once installed you will need to prepend the path to qmake to your PATH environment variable. On my system that is `/home/magland/Qt/5.7/gcc_64/bin`.
You may instead do `sudo ln -s /home/magland/Qt/5.7/gcc_64/bin/qmake /usr/local/bin/qmake`.
