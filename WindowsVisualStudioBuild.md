# Introduction #

Starting with version 0.20, companion9x is compiled for Windows by using Microsoft Visual C++ Express 2010

This is a walk though describing how to build companion9x on a Windows XP box, should work on Windows 7 to.

All software was installed in paths with no spaces. I tried installing in paths with spaces but it created no end of problems. It is not needed for all packages but it is much safer this way.

I also don't like software that installs in the C:\ root directory so I put everything in C:\Programs\

The only exceptions to this where Visual Studio and Qt. These exceptions were made to remain the same as Bertrand and therefore stop problems with the installer creator failing to find files.


# Useful tools #

This software is very handy for editing your PATH environmental variable:

http://www.redfernplace.com/software-projects/patheditor/

Or this for all environmental variables:

http://sourceforge.net/projects/env-man/

If you don't want to use either of these tools (and I really recommend you do) then you can edit environmental variables by
  * Right clicking on My Computer
  * Select Properties
  * Click the Advanced tab on the window that pops up
  * Click the Environment Variables button
  * You will find PATH in the System Variables list

This file <a href='http://companion9x.googlecode.com/svn/wiki/doshere.reg'>doshere.reg</a> gives you the ability to right click on a file or folder and have a DOS prompt pop up already CDed to that location. Just download it and double click it to add it to the registry. WARNING only tested on XP.


# Get the needed tools #

## Visual Studio 2010 C++ ##

http://www.microsoft.com/visualstudio/en-us/products/2010-editions/visual-cpp-express

From the above page you download a web installer.
Run vc-web.exe

**Installer choices**
  * Install path C:\Program Files\Microsoft Visual Studio 10.0
  * Install Microsoft Silverlight - You can decide (not needed for comanpion9x)
  * Install Microsoft SQL Server - You can decide (not needed for comanpion9x)

## CMake ##

http://www.cmake.org/cmake/resources/software.html

Run cmake-x.x.x-win32-x86.exe
(x.x.x represents version number, get the latest)

**Installer choices**
  * Add CMake to the system PATH for all users
  * Install path C:\Programs\CMake

Check it has added the following to the PATH environmental variable

C:\Programs\CMake\bin

## Nokia Qt ##

http://get.qt.nokia.com/qt/source/qt-win-opensource-4.8.0-vs2010.exe

This is the Qt Libraries for Visual Studio 2010 (not the SDK)

Run qt-win-opensource-4.8.0-vs2010.exe

**Installer choices**
  * Install path c:\Qt\4.8.0-vc
  * File Associations - You can decide (not needed for comanpion9x)
Add the following to the PATH environmental variable

C:\Qt\4.8.0-vc\bin

## TortoiseSVN ##

http://tortoisesvn.net/downloads.html

Run TortoiseSVN-x.x.x.xxxxx-win32-svn-x.x.x.msi
(x.x.x represents version number, get the latest)

**Installer choices**
  * Install "command line client tools"
  * Install path c:\Programs\TortiseSVN

Add the following to the PATH environmental variable

C:\Programs\TortiseSVN\bin

## NSIS ##

http://nsis.sourceforge.net/Download

Run nsis-x.xx-setup.exe
(x.xx represents version number, get the latest)

**Installer choices**
  * Install path c:\Programs\NSIS

Add the following to the PATH environmental variable

C:\Programs\NSIS\bin

## Xerces ##

http://xerces.apache.org/mirrors.cgi#binary

You want V3.1.1 for Visual Studio 10

Unzip the contents of the zip to "C:\Programs\"

Rename the folder "xerces-c-3.1.1" to "xerces-vc" so you have
"C:\Programs\xerces-vc\"

## CodeSynthesis XSD ##

http://www.codesynthesis.com/download/xsd/3.3/windows/i686/xsd-3.3.0-i686-windows.zip

Unzip xsd-3.3.0-i686-windows.zip to "C:\Programs\"

Rename the "folder xsd-3.3.0-i686-windows" to "xsd-3.3.0" so you have "C:\Programs\xsd-3.3.0\"

Navigate to "C:\Programs\xsd-3.3.0\bin"
Make a copy of xsd.exe and call it xsd-cxx.exe (because there is another program named xsd.exe (Microsoft) on the system.

## PThreads ##

[ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.exe](ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-8-0-release.exe)

Run pthreads-w32-2-8-0-release.exe

Set extraction directory to "C:\Programs\pthreads\"

## MSinttypes ##

http://msinttypes.googlecode.com/files/msinttypes-r26.zip

Unzip the contents to "C:\Programs\msinttypes\"

## Patch ##

http://gnuwin32.sourceforge.net/packages/patch.htm

Get the "Complete package, except sources"

**Installer choices**
  * Install directory C:\Programs\patch\
  * Full installation
  * Start Menu folder - You can decide (not needed for comanpion9x)
  * Additional tasks - You can decide (not needed for comanpion9x)

Add the following to the PATH environmental variable

C:\Programs\patch\bin

## LibSDL ##

http://www.libsdl.org/download-1.2.php

Download the Win32 Visual C++ Development Libraries

Unzip SDL-x.x.xx-win32.zip to "C:\Programs\SDL\"
(x.x.xx represents version number, get the latest)

Navigate to "C:\Programs\SDL\lib\x86" or if you have a 64bit system "C:\Programs\SDL\lib\x64". Select all the files in this directory, copy them, go one directory level up, paste them.

# Get companion9x via SVN #

In a path somewhere with no spaces in it create yourself a directory called companion9x.

Right click on this directory and select SVNCheckout in the menu.

In the dialog box that pops up set the **URL of repository:** to http://companion9x.googlecode.com/svn/
Leave the other options as is.
Click OK.

Some files will download.


# Building #

Open a command prompt in the directory trunk/src

Type the following at the console:

```xml

mkdir ..\build
cd ..\build
cmake -G "Visual Studio 10" ..\src ```

You should see something like:

```xml

Z>cmake -G "Visual Studio 10" ..\src
-- Check for working C compiler using: Visual Studio 10
-- Check for working C compiler using: Visual Studio 10 -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working CXX compiler using: Visual Studio 10
-- Check for working CXX compiler using: Visual Studio 10 -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Looking for XercesC
--   found xerces-c: C:/Programs/xerces-vc/lib/xerces-c_3.lib
-- Looking for Xsd
-- C:/Programs/xsd-3.3.0/bin/xsd-cxx.exe
XSD Include Path: C:/Programs/xsd-3.3.0/libxsd
-- Looking for SDL
-- Looking for include files CMAKE_HAVE_PTHREAD_H
-- Looking for include files CMAKE_HAVE_PTHREAD_H - not found.
-- Found Threads: TRUE
SDL Include Path: C:/Programs/SDL/include
svn: E200029: Couldn't perform atomic initialization
svn: E170001: Could not initialize the SASL library
-- Found Subversion: C:/Programs/TortoiseSVN/bin/svn.exe
-- Looking for Q_WS_X11
-- Looking for Q_WS_X11 - not found.
-- Looking for Q_WS_WIN
-- Looking for Q_WS_WIN - found
-- Looking for Q_WS_QWS
-- Looking for Q_WS_QWS - not found.
-- Looking for Q_WS_MAC
-- Looking for Q_WS_MAC - not found.
-- Found Qt4: C:/Qt/4.8.0-vc/bin/qmake.exe (found version "4.8.0")
-- Configuring done
-- Generating done
-- Build files have been written to: Z:/yourbuilddir/companion9x/build/trunk/build ```

If you get errors due to missing files (which you won't if you have followed the above) then after you have corrected the error and before you re-run CMake it is best to delete build/CMakeCache.txt

Now navigate with explorer to the build directory you just created and double click on the ALL\_BUILD.vcxproj file.

When Visual Studio is launched it will spend a little while scanning source files, just wait while it does this. After it is done click the "Save All" icon, this stops it doing the same thing again if you re-open the project.

If you want to build the installer you need to select the "Release" build or the installer build will fail. If you are going to go debugging then use the "Debug" build.

![http://companion9x.googlecode.com/svn/wiki/vc_build-type.jpg](http://companion9x.googlecode.com/svn/wiki/vc_build-type.jpg)

Once you have decided which way to build you have to build it. In the "Solution Explorer" right click on "ALL\_BUILD" and select "build".

![http://companion9x.googlecode.com/svn/wiki/vc_build-all_build.jpg](http://companion9x.googlecode.com/svn/wiki/vc_build-all_build.jpg)

### Useful tips ###
  * If Visual Studio seems to freeze then disable your virus checker and try again. Very bad I know, complain to Microsoft ! That said I don't like virus checkers either, necessary evil.
  * Crtl+Break will abort a build

Once it has finished copy the following files to the build/Release/ or build/Debug/ directories to run companion9x.exe in place.

C:\Programs\xerces-vc\bin\xerces-c\_3\_1.dll
C:\Programs\pthreads\Pre-built.2\lib\pthreadVC2.dll
C:\Programs\SDL-1.2.14\lib\SDL.dll

Or if you want right click on "installer" in the Visual Studio Solution Explorer and click build and it will generate the installer exe. **Remember you must already have the release build built for this to work**