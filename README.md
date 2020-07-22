# PortaDrop

PortaDrop is a portable digital microfluidic system. 


## Installation-Guide
### 1) Install Raspberry Pi OS
- Install [Win32DiskImager](https://sourceforge.net/projects/win32diskimager/)
- Download [Raspberry Pi OS](https://www.raspberrypi.org/downloads/raspberry-pi-os/)
- Write Raspberry Pi OS Image to SD Card using Win32DiskImager
- Plugin SD Card into the Raspberry Pi and launch Raspberry Pi
- Finish Raspberry Pi OS installation

### 2) Activate communication protocols
Activate CSI, I2C and UART using raspi-config

	sudo raspi-config

### 3) Install GNU Image Manipulation Program Toolkit, GTK minus minus and Glade
gtkmm is a library which is used to build graphical user interfaces (GUI). glade can be used to design the interface.

  	sudo apt-get update
	sudo apt-get install libgtkmm-3.0-dev
	sudo apt-get install glade

### 4) Install "Seabreeze" for Ocean Optics HR2000+

Download and install "Seabreeze" library

	sudo apt-get install unzip libusb-dev
	cd ~
	wget https://downloads.sourceforge.net/project/seabreeze/SeaBreeze/source/seabreeze-3.0.11.zip
	unzip seabreeze-3.0.11.zip -d ./seabreeze/
	cd seabreeze/seabreeze-3.0.11/SeaBreeze/
	
Uncomment line from common.mk -> Add "#" in front of line containing "-Werror \" and save
	
	nano common.mk

Continue installation	
	
	make
	sudo cp ./libseabreeze.so /usr/local/lib/libseabreeze.so
	sudo cp -r ./include/api /usr/local/include/api
	sudo cp -r ./include/common /usr/local/include/common
	sudo cp -r ./include/native /usr/local/include/native
	sudo cp -r ./include/vendors /usr/local/include/vendors
	sudo ldconfig -v
	sudo cp ./os-support/linux/10-oceanoptics.rules /etc/udev/rules.d/10-oceanoptics.rules

### 5) Install NI GPIB

	sudo apt-get install bc libncurses5-dev tk-dev build-essential texinfo texi2html libcwidget-dev libncurses5-dev libx11-dev binutils-dev bison flex libusb-1.0-0 libusb-dev libmpfr-dev libexpat1-dev tofrodos subversion autoconf automake libtool mercurial
	sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/bin/rpi-source && sudo chmod +x /usr/bin/rpi-source && /usr/bin/rpi-source -q --tag-update
	sudo rpi-source
	cd ~
	mkdir linux-gpib
	cd linux-gpib
	wget https://sourceforge.net/projects/linux-gpib/files/linux-gpib%20for%203.x.x%20and%202.6.x%20kernels/4.2.0/linux-gpib-4.2.0.tar.gz/download -O linux-gpib.tar.gz
	tar -xvzf linux-gpib.tar.gz
	cd linux-gpib-4.2.0/
	tar -xvzf linux-gpib-kernel-4.2.0.tar.gz
	tar -xvzf linux-gpib-user-4.2.0.tar.gz
	cd ./linux-gpib-kernel-4.2.0/
	mkdir m4
	sudo ./bootstrap
	sudo ./configure
	sudo make
	sudo make install
	cd ../linux-gpib-user-4.2.0/
	sudo ./bootstrap
	sudo ./configure --sysconfdir="/etc"
	sudo make
	sudo make install
	sudo addgroup gpib
	sudo usermod -aG gpib pi

### 6) Install Raspicam
	git clone https://github.com/cedricve/raspicam.git
	cd raspicam
	mkdir build
	cd build
	cmake ..
	make
	sudo make install
	sudo ldconfig

### 7) Install "opencv" (takes a few hours) 
	sudo apt-get update
	sudo apt-get install build-essential cmake unzip pkg-config libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libxvidcore-dev libx264-dev libgtk-3-dev libcanberra-gtk* libatlas-base-dev gfortran python3-dev
	git clone https://github.com/Itseez/opencv.git && cd opencv && git checkout 3.4
	git clone https://github.com/opencv/opencv_contrib.git && cd opencv && git checkout 3.4
	cd ~ && wget https://bootstrap.pypa.io/get-pip.py && sudo python3 get-pip.py
	cd ~/opencv && mkdir build && cd build
	cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=/usr/local -DINSTALL_PYTHON_EXAMPLES=OFF -DINSTALL_C_EXAMPLES=OFF -DOPENCV_EXTRA_MODULES_PATH=~/opencv_contrib /modules -DENABLE_PRECOMPILED_HEADERS=OFF -DBUILD_EXAMPLES=OFF ..

Change size of the swapfile to avoid issues during compilation. Change value to 2048 MB and remember the old value (normally 100MB) 

	sudo nano /etc/dphys-swapfile

Save file and restart the service

	sudo /etc/init.d/dphys-swapfile restart

Continue installation (this step takes a few hours)
	
	cd ~/opencv/build
	make -j4
	sudo make install && sudo ldconfig

Reset size of the swapfile (normally 100MB) 

	sudo nano /etc/dphys-swapfile

Save file and restart the service

	sudo /etc/init.d/dphys-swapfile restart

Load drivers

	sudo modprobe bcm2835-v4l2
	sudo sh -c 'echo bcm2835-v4l2 >> /etc/modules-load.d/modules.conf' 

### Install PortaDrop Software
	git clone https://github.com/ewodac/PortaDrop.git
	cd PortaDrop
	mkdir BUILD
	cd BUILD
	cmake ../ -DCMAKE_INSTALL_PREFIX=~/portaDrop
	make
	make install
	
Starting the appication can be done via terminal by

	cd ~/portaDrop/bin
	./ewodInterface
