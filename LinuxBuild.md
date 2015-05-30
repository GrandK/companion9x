# Building companion9X for Linux #

# Prerequisites #

You will need ...
  * g++
  * [Qt](http://qt.nokia.com/) (v4.7.4)
  * [cmake](http://www.cmake.org)
  * [Apache Xerces-C++](http://xerces.apache.org/) (v2.8.0)
  * [xsdcxx](http://www.codesynthesis.com/projects/xsd)
  * [SDL](http://www.libsdl.org/) (v1.2)
  * [Phonon](http://phonon.kde.org/)

## On Ubuntu 11.04, 11.10, 12.04: ##
```
sudo apt-get install subversion gcc-avr avr-libc avrdude build-essential ruby libqtcore4 libqt4-dev qt4-qmake g++ cmake libxerces-c-dev xsdcxx libsdl1.2-dev libusb-1.0-0 libphonon-dev phonon
```
## On Fedora: ##
```
sudo yum install qt qt-devel cmake patch xsd svn gcc-c++ sdl-devel phonon phonon-devel
```
## UsbASP ##
You need enable AVRDUDE to run without "sudo".
Plug in the programmer and run
```
lsusb | grep libusb
```
You should get a line like this:
```
Bus 003 Device 002: ID 16c0:05dc VOTI shared ID for use with libusb
```
Notice the two numbers after ID XXXX:YYYY

In next command replace XXXX and YYYY with the numbers.
```
sudo echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="XXXX", ATTR{idProduct}=="YYYY", GROUP="adm", MODE="0666"' > /etc/udev/rules.d/10-usbasp.rules
```
Then execute:
```
sudo restart udev
```

# Building companion9x #

We're ready to grab the source files and build companion9x itself.

## Check out the c9x source ##

See the [project's Source page](http://code.google.com/p/companion9x/source/checkout), as provided by Googlecode.

```
svn checkout http://companion9x.googlecode.com/svn/trunk companion9x
```
## Prepare a directory for building ##
```
cd companion9x

mkdir lbuild
```


## Prepare a Makefile using CMake ##

```
cd lbuild

cmake ../src
```

Finally, we're ready to compile ...

```
make clean && make
```

When the build process completes (with no show-stopping errors!) you will end up with a binary named `companion9x`.

You should now be able to run companion9x from your terminal, like so ...
```
./companion9x
```

## Joystick support ##
Companion9x does support joystick for firmware simulator control.
Latest version of SDL use by default a device that cannot be calibrated properly by the OS.
To avoid problems use the following command before launching companion9x
```
export SDL_JOYSTICK_DEVICE=/dev/input/js0 (where js0 is your joystick / RC Sim adapter)
```
or write
```
SDL_JOYSTICK_DEVICE=/dev/input/js0
```
in /etc/environment

In companion9x you will have two joysticks, choose the first one.
If you experience calibration problems have a look to the jscal utility.


calibrating:
```
jscal -c /dev/input/js0
```
directory for storing:
```
mkdir /var/lib/joystick
```
store the calibration:
```
jscal-store /dev/input/js0
```