PageEdit
========
A single page visual XHTML editor based on Sigil's Deprecated BookView.
It uses WebEngine instead of WebKit.

This app is working and fundamentally complete.

It requires Qt 5.12.3 or later.

To build after cloning the repo

You will need cmake version 3.0 to build:

Make sure the qt tools and libraries from Qt 5.12.3 are in your path<br>
`export PATH=~/QT512/bin:${PATH}`

You build in a separate directory and not in the source directory.

After checking out the PageEdit github repo into a PageEdit directory

Building on macOS:
------------------

`export MACOSX_DEPLOYMENT_TARGET=10.12`<br>
`export MYQTHOME=~/Qt512`<br>
`export PATH=${PATH}:${MYQTHOME}/bin`<br>
`mkdir build`<br>
`cd build`<br>
`cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=10.12 -DCMAKE_BUILD_TYPE=Release \`<br>
`      -DCMAKE_CXX_FLAGS=-Wno-inconsistent-missing-override \`<br>
`      -DCMAKE_PREFIX_PATH=${MYQTHOME}/lib/cmake \ `<br>
`      -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/\`<br>
`                   MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk/ ../PageEdit`<br>
`make`<br>
`make addframeworks`<br>
`cd bin`<br>
`open -a /FULL_PATH_TO_THIS_DIRECTORY/PageEdit.app /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`<br>
`or simply drag and drop an xhtml file from an unzipped epub onto the PageEdit.app`<br>


On Linux
--------

Make sure Qt5.12.3 (installed with the WebEngine package) and cmake are installed

`mkdir build`<br>
`cd build`<br>
`cmake -DCMAKE_BUILD_TYPE=Release ../PageEdit`<br>
`make`<br>
`cd bin`<br>
`./pageedit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

You can also install PageEdit with `make install` (defaults to the /usr/local prefix, so use sudo if required).

On Windows:
-----------

Make sure Qt5.12.3 (with the WebEngine component) and Visual Studio 2017 are installed. Install the x64 version of Qt5.12.3 for VS2017. Install cmake 3.0+ and make sure its bin directory is added to your path.

Make sure that Qt's bin directory is also added to your PATH. Take note of the path for your Qt's prefix (the directory right above the bin directory). Open an "x64 Native Tools Command Prompt for VS2017" from the Start menu and cd to wherever you want to build.

`mkdir build`<br>
`cd build`<br>
`cmake -G "NMake Makefiles" -DQt5_DIR="C:\path\to\your\Qt5\prefix\lib\cmake\Qt5 -DCMAKE_BUILD_TYPE=Release "C:\path\to\PageEdit's\cloned\repository"`<br>
`nmake`<br>
`cd bin`<br>
`PageEdit.exe \FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB\FILENAME.xhtml`




