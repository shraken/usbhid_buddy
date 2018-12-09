#!/bin/bash

sudo apt-get update > /dev/null 2>&1

sudo apt-get install -y build-essential autoconf automake picocom swig \
    python python-dev libudev-dev libusb-1.0-0-dev libfox-1.6-dev \
    python-pip srecord cmake libusb-1.0-0-dev > /dev/null 2>&1
echo "Installed base apt-get packages"

sudo apt-get install -y linux-image-extra-$(uname -r) > /dev/null 2>&1
echo "Installed Linux headers apt-get package"

sudo pip install intelhex > /dev/null 2>&1
echo "Installed intelhex pip python package"

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test > /dev/null 2>&1
sudo apt-get update > /dev/null 2>&1
#sudo apt-get upgrade > /dev/null 2>&1
#sudo apt-get dist-upgrade > /dev/null 2>&1

sudo dpkg --add-architecture i386 > /dev/null 2>&1
sudo apt-get update > /dev/null 2>&1
sudo apt-get install -y libc6:i386 libncurses5:i386 libstdc++6:i386 > /dev/null 2>&1
echo "Installed prerequisite i386 apt-get libraries"

wget https://www.python.org/ftp/python/2.7.15/Python-2.7.15.tgz > /dev/null 2>&1
tar xfvz Python-2.7.15.tgz > /dev/null 2>&1
pushd Python-2.7.15 > /dev/null 2>&1
./configure > /dev/null 2>&1
make > /dev/null 2>&1
sudo make install > /dev/null 2>&1
popd
rm -rf Python-2.7.15
rm -rf Python-2.7.15.tgz
echo "Installed python2.7"

wget https://versaweb.dl.sourceforge.net/project/sdcc/sdcc-linux-x86/3.6.0/sdcc-3.6.0-i386-unknown-linux2.5.tar.bz2 > /dev/null 2>&1
tar xjf sdcc-3.6.0-i386-unknown-linux2.5.tar.bz2 > /dev/null 2>&1
sudo cp -r sdcc-3.6.0/* /usr/local
rm -rf sdcc-3.6.0
rm -rf sdcc-3.6.0-i386-unknown-linux2.5.tar.bz2

echo "Installed sdcc compiler by a source build"

if [ -f /vagrant/JLink_Linux_*.deb ]; then
    sudo dpkg -i /vagrant/JLink_Linux_*.deb > /dev/null 2>&1
    echo "Installed JLink flash and debug software"
fi

sudo modprobe cdc_acm

sudo echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"10C4\", ATTR{idProduct}==\"82CD\", MODE=\"0666\"" > /etc/udev/rules.d/45-buddy.rules
sudo service udev restart > /dev/null 2>&1

echo "All Finished!"