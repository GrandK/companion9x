# Building companion9X on OS X #

# Prerequisites #
  * [xcode 4.1](https://developer.apple.com/)
  * [Qt](http://qt.nokia.com/downloads/qt-for-open-source-cpp-development-on-mac-os-x/)
  * [xsd](http://www.codesynthesis.com/products/xsd/download.xhtml)
  * [SDL](http://www.libsdl.org/download-1.2.php)
  * [cmake](http://www.cmake.org/cmake/resources/software.html)
  * [xerces](http://xerces.apache.org/xerces-c/download.cgi)
  * [avr](http://www.obdev.at/products/crosspack/download.html)

## OS X 10.7.2 Lion ##
Note: Links to SDL package is for Intel CPU based machines. If you have powerPC find and download appropriate package from links above.

  1. install [installxcode\_41\_lion.dmg](https://developer.apple.com/) and after running xcode go to _Preferences, Documentation_ and press _"Check and Install Now"_.
  1. install [qt-mac-opensource-4.8.2.dmg](http://releases.qt-project.org/qt4/source/qt-mac-opensource-4.8.2.dmg)
  1. extract [xsd-3.3.0-i686-macosx.tar.bz2](http://www.codesynthesis.com/download/xsd/3.3/macosx/i686/xsd-3.3.0-i686-macosx.tar.bz2) to _/opt/local_
  1. unpack [SDL-1.2.15.tar.gz](http://www.libsdl.org/release/SDL-1.2.15.tar.gz) sources, configure, make and install them
```
cd SDL-1.2.15
./configure
make
sudo make install
```

  1. install [cmake-2.8.8-Darwin64-universal.dmg](http://www.cmake.org/files/v2.8/cmake-2.8.8-Darwin64-universal.dmg)
  1. unpack [xerces-c-3.1.1.tar.gz](http://tux.rainside.sk/apache//xerces/c/3/sources/xerces-c-3.1.1.tar.gz) souces, configure, make and install them
```
cd xerces-c-3.1.1
./configure
make
sudo make install
```

## UsbASP ##
Last needed thing is support for usb programer.

Just install [CrossPack-AVR-20120217.dmg](http://www.obdev.at/downloads/crosspack/CrossPack-AVR-20120217.dmg)

# Building companion9x #
Now let's download and make latest companion9x
```
cd ~
svn checkout http://companion9x.googlecode.com/svn/ companion9x
cd companion9x/trunk
mkdir xcode
cd xcode
cmake -G Xcode ../src
```

Start Xcode and open _companion9x.xcodeproj_ file located in _~/companion9x/trunk/xcode/_

**Run build scheme companion9x**

now you can run companion9x application located in _~/companion9x/trunk/xcode/Debug/companion9x.app_

or you can move _copmanion9x.app_ to your usual Application directory.