#!/bin/sh
# installs required packages on Ubuntu to compile fhDOOM.
# most packages will install additional dependencies that are also required but are not explicitly listed here.

echo "installing basic stuff..."
sudo apt-get install git cmake g++

echo "installing dev packages..."
sudo apt-get install freeglut3-dev libalut-dev libasound2-dev libopenal-dev

MACHINE_TYPE=`uname -m`
echo "machine type:"${MACHINE_TYPE}
if [ "$MACHINE_TYPE" = "x86_64" ]; then
	echo "installing 32bit libraries..."
	sudo apt-get install g++-multilib freeglut3-dev:i386 libalut-dev:i386 libasound2-dev:i386 libopenal-dev:i386
fi