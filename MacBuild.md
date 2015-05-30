# Building companion9X for OS X #

Brief notes only until I figure out how it all works -- gruvin.

I'm building on OS X 10.7 (Lion). That means my OS is 'too new' for most binary installers of most development libraries -- including Qt4.7.4, who's binary distro only supports up to OS X 10.6, still. **yawn** So it's good old 'compile your own' Unix methods being employed here.

**The biggest challenge** we seem to have right now is getting Qt et al to bundle their shared libraries into a final project that doesn't just crash out when launched. This may be an OS X Lion issue only -- I have no way to test, yet.

That said, companion9x will run happily on OS X Lion, if the following procedure is followed in full. Allow up to three hours for the first time you go through this though -- because Qt takes a long, long time to compile.

# Prerequisites #

You will need ...
  * [Xcode 4](http://developer.apple.com/xcode/index.php) (Apple Developer Tools)
  * [Qt](http://qt.nokia.com/) (v4.7.4)
  * [Apache Xerces-C++](http://xerces.apache.org/) (v2.8.0)
  * [cmake](http://www.cmake.org)

## Installing Xcode ##

If you're doing any source compiling or code development on your Mac, then you already have Xcode. If not, you can get started by registering at the [Apple Developer](http://developer.apple.com/devcenter/mac/index.action) site. Alternatively, you can also obtain Xcode 4 from the Apple App Store.

Xcode 4 is a rather large (4GB!) download. I believe it's free for OS X development, which is what we need here. If not mistaken, you only have to pay an annual membership fee to get the iOS development library. _Somebody check me on this? I'm a paid up member, so not sure._

## Installing Qt ##

I used [MacPorts](http://www.macports.org/) ...

```
sudo port install qt4-mac
```

**NOTE**: At this stage, we are not set up to be able to use Qt in frameworks mode. Despite Qt's [documentation to the contrary](http://doc.trolltech.com/4.7/deployment-mac.html), MacPorts does NOT in fact build the Qt-frameworks version by default. So we're OK.

## Installing Apache Xerces-C++ ##

MacPorts to the rescue again ...
```
sudo port install xercesc
```

## Installing CMAKE ##

MacPorts to the rescue yet again ...
```
sudo port install cmake
```


# Building companion9x #

Finally, we're ready to grab the source files and build companion9x itself.

## Check out the c9x source ##

See the [project's Source page](http://code.google.com/p/companion9x/source/checkout), as provided by Googlecode.

## Prepare a Makefile using CMAKE ##

```
cd companion9x/trunk/src

cmake -G 'Unix Makefiles'
```

Finally, we're ready to compile ...

```
make clean && make
```

When the build process completes (with no show-stopping errors!) you will end up with a folder named `companion9x.app`.

You should now be able to run companion9x from your terminal, like so ...
```
./companion9x.app/Contents/MacOSX/companion9x
```
or
```
open companion9x.app
```

You can also locate 'companion9x' (of type Application) in Finder and double-click that to launch it -- or copy the application to /Applications, if you like.

At the time of this writing, we do not yet have an icon file to make the application look pretty. I'm sure there will be one soon though.

## IMPORTANT ##

The above process does _not_ bundle the required Xerces or Qt shared library files into the companion9x application. Therefore, this compiled version of companion9x **will _only_ run on OS X systems where Xerces-C++ and Qt4 have been installed _using MacPorts_**, as detailed above. (It needs to be MacPorts rather than Fink or the Qt binary install from Nokia, because the libraries need to be located in `/opt/local/lib`.)

We are working on getting Qt bundling to work -- or to at make it an option. For now though, at least not bundling the Xerces-C++ and Qt4 library files does keep companion9x's download size much smaller and thus more portable.