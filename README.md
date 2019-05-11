PageEdit
========
A simple single page visual XHTML editor based on Sigil's Deprecated BookView.
It uses WebEngine instead of WebKit.

This app is undergoing rapid development. It is working but incomplete.

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
`open ./PageEdit.app --args /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml``<br>


On Linux

`./PageEdit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

On Windows:

Although the PageEdit can be built by issuing similar command-line options used in the macOS and on Linux instructions above, it's probably much easier to configure QtCreator with the Qt5.12.3 kit (and VS2017) and open the the PageEdit.pro file. From there, you'l be able to configure/build/debug PageEdit without having to manually configure the Qt environment for the command prompt.
