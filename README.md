PageEdit
========
An ePub visual XHTML editor based on Sigil's Deprecated BookView.
It uses WebEngine instead of WebKit.

This app is working and fundamentally complete.

![PageEdit](screencaps/pageedit.png?raw=true) PageEdit Visual WYSIWYG XHtml Editor

![PageEdit](screencaps/dark-mode.png?raw=true) PageEdit in Dark Mode

![PageEdit](screencaps/inspector.png?raw=true) Using the Inspector

![PageEdit](screencaps/insert-image.png?raw=true) Insert Images

![PageEdit](screencaps/insert-special-chars.png?raw=true) Insert Special Characters

![PageEdit](screencaps/add-links.png?raw=true) Add Links

It prefers Qt 6.5.2 or later but can still be built with Qt 5.15.9.

Building on macOS Ventura
-------------------------
You will need cmake version 3.0 or later to build:

You build in a separate directory and not in the source directory.

After checking out the PageEdit github repo into a PageEdit directory


`export MACOSX_DEPLOYMENT_TARGET=11.0`<br>
`export MYQTHOME=~/Qt652`<br>
`export PATH=${PATH}:${MYQTHOME}/bin`<br>
`mkdir build`<br>
`cd build`<br>
`cmake -DUSE_QT6=1 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DCMAKE_BUILD_TYPE=Release \`<br>
`      -DCMAKE_CXX_FLAGS=-Wno-inconsistent-missing-override \`<br>
`      -DCMAKE_PREFIX_PATH=${MYQTHOME}/lib/cmake \ `<br>
`      -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/\`<br>
`                   MacOSX.platform/Developer/SDKs/MacOSX13.3.sdk/ ../PageEdit`<br>
`make`<br>
`make addframeworks`<br>
`cd bin`<br>
`open -a /FULL_PATH_TO_THIS_DIRECTORY/PageEdit.app /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`<br>
`or simply drag and drop an xhtml file from an unzipped epub onto the PageEdit.app`<br>
``<br>
`or edit a set of XHtml files in epub Spine Order by loading the ePub's OPF file`<br>

On Linux
--------

Make sure a minimum of Qt5.9.4(ish) is installed (with the WebEngine package) as well as cmake.<br>
You build in a separate directory and not in the source directory.

Qt 6.5.X or later is preferred but Qt5.13+ is probably a more realistic minimum to be able enjoy all of PageEdit's features.

Get the PageEdit Source:

`git clone https://github.com/sigil-ebook/PageEdit.git`<br>
(or download/extract the gzipped tarball from a PageEdit Release)<br>
`cd PageEdit`

__Ubuntu__

`sudo apt-get install build-essential`<br>
`sudo apt-get install cmake`<br>
`sudo apt-get install qtwebengine6-dev qttools6-dev qttools6-dev-tools`

`mkdir build`<br>
`cd build`<br>
`cmake "Unix Makefiles" -DUSE_QT6=1 -DCMAKE_BUILD_TYPE=Release ../<path/to/PageEdit/source>`<br>
`make`<br>

__Arch Linux__

sudo pacman -S cmake qt6-webengine qt6-tools<br>

`mkdir build`<br>
`cd build`<br>
`cmake "Unix Makefiles" -DINSTALL_BUNDLED_DICTS=0 -DUSE_QT6=1 -DCMAKE_BUILD_TYPE=Release ../<path/to/PageEdit/source>`<br>
`make`<br>

__All Flavors__:

To test
`cd bin`<br>
`./pageedit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

`make install` (defaults to the /usr/local prefix, so use sudo if required).

__NOTE__: certain features, like Inspector, will require a minimum of Qt5.11.x to function on Linux.

__NOTE__: If you want to build with Qt5, you'll obviously need to drop -DUSE_Qt6=1 and change -DQt6_DIR to -DQt5_DIR and point it to your Qt5 installation.

By default, bundled hunspell dictionaries are converted to webengine spell check dictionaries (with qt tools) and installed to the QT_INSTALL_DATA/qtwebengine_dictionaries location. This location can be found using the qmake binary (of the Qt you're building PageEdit with) with the following command `-qmake -query QT_INSTALL_DATA` . If using the system Qt, this will typically be /usr/share/qt/qtwebengine_dictionaries. To disable the conversion/installation of these bundled dictionaries, use -DNSTALL_BUNDLED_DICTS=0 when configuring PageEdit with cmake. Arch Linux already includes these dictionaries with the corresponding hunspell language dictionaries. So you'll want to skip their conversion/installation on Arch and make sure you have the correct hunspell languages installed instead.

On Windows:
-----------

Qt6.5.2 or higher is preferred, but you'll want to maintain a  minimum of Qt5.13.x (with the WebEngine component) if you want to take advantage of all features. A minimum of Visual Studio 2022 is required if you want to use Qt6.5.2. Install cmake 3.18+ and make sure its bin directory is added to your path.

Make sure that Qt's bin directory is also added to your PATH. Take note of the path for your Qt's prefix (the directory right above the bin directory). Open an "x64 Native Tools Command Prompt for VS2022" from the Start menu and cd to wherever you want to build.

`mkdir build`<br>
`cd build`<br>
`cmake -G "NMake Makefiles" -DUSE_Qt6=1 -DQt6_DIR="C:\path\to\your\Qt6\prefix\lib\cmake\Qt6 -DCMAKE_BUILD_TYPE=Release "C:\path\to\PageEdit's\cloned\repository"`<br>
`nmake`<br>

To test:
`cd bin`<br>
`PageEdit.exe \FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB\FILENAME.xhtml`

Use `nmake deploy` to package PageEdit and all of its dependencies into a zip archive (in a newly created 'deploy' folder in the build folder)

Use `nmake deployinstaller` to package PageEdit and all of its dependencies into an Inno Setup installer (must have InnoSetup installed and on the PATH for this to work).

__NOTE__: if you configure PageEdit with the -DDEPLOY_SFX=1 cmake option before compiling, 'nmake deploy` will attempt to create a 7-Zip self-extracting archive. So naturally, make sure 7-Zip is installed before trying to use it.

__NOTE__: as of this writing, the Official PageEdit release are built using Qt6.5.2

__NOTE__: If you want to build with Qt5, you'll obviously need to drop -DUSE_Qt6=1 and change -DQt6_DIR to -DQt5_DIR and point it to your Qt5 installation.


## For Github repository maintainers (who am I kidding, this is to keep myself from forgetting/messing up):

The upstream sigil-gumbo repository is included in PageEdit's repository (this one) as a subtree in the gumbo_subtree prefix. To pull in upstream sigil-gumbo changes (or even to safely check if there _are_ any), use the following command in the root of your local clone of the PageEdit repo:

`git subtree pull --prefix gumbo_subtree https://github.com/Sigil-Ebook/sigil-gumbo.git master --squash`

--squash because we don't need sigil-gumbo's entire history in the PageEdit repository. Push the changes to github master (with a commit message like "merge in upstream sigil-gumbo changes") if there are any.

You can also create a remote for the upstream sigil-gumbo repo to simply the subtree pull command a bit -- BUT YOU MUST REMEMBER TO USE THE --no-tags OPTION WHEN CREATING THE REMOTE. Otherwise the --squash option may not suppress all upstream history when using the remote name. A careless `git fetch --all` could make a dog's lunch of your repo history with a very taggy remote (which sigil-gumbo is decidedly not):

`git remote add --no-tags sigil-gumbo https://github.com/Sigil-Ebook/sigil-gumbo.git`

After that, pulling in any upstream sigil-gumbo changes becomes:

`git subtree pull --prefix gumbo_subtree sigil-gumbo master --squash`

Probably safer in the long run to create a git alias specific to the PageEdit repository (without a remote added) using the full subtree pull command if you need a shortcut. From within the PageEdit repo:

`git config alias.gumbo-sub-pull 'subtree pull --prefix gumbo_subtree https://github.com/Sigil-Ebook/sigil-gumbo.git master --squash'`

Use any alias name you like. I chose "gumbo-sub-pull"  Then it's simply a matter of using `git gumbo-sub-pull` in the root of the PageEdit repository.



