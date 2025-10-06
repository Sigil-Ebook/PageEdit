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

It prefers Qt 6.9.3 or later but can still be built with Qt 6.4.2.

Building on macOS Ventura
-------------------------
You will need cmake version 3.18 or later to build:

You build in a separate directory and not in the source directory.

After checking out the PageEdit github repo into a PageEdit directory


`export MACOSX_DEPLOYMENT_TARGET=12.0`<br>
`export MYQTHOME=~/Qt693`<br>
`export PATH=${PATH}:${MYQTHOME}/bin:${MYQTHOME}/libexec`<br>
`mkdir build`<br>
`cd build`<br>
`cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DCMAKE_BUILD_TYPE=Release \`<br>
`      -DCMAKE_CXX_FLAGS=-Wno-inconsistent-missing-override \`<br>
`      -DCMAKE_PREFIX_PATH=${MYQTHOME}/lib/cmake \ `<br>
`      -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/\`<br>
`                   MacOSX.platform/Developer/SDKs/MacOSX15.sdk/ ../PageEdit`<br>
`make`<br>
`make addframeworks`<br>
`cd bin`<br>
`open -a /FULL_PATH_TO_THIS_DIRECTORY/PageEdit.app /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`<br>
`or simply drag and drop an xhtml file from an unzipped epub onto the PageEdit.app`<br>
``<br>
`or edit a set of XHtml files in epub Spine Order by loading the ePub's OPF file`<br>

On Linux
--------

Make sure a minimum of Qt6.4.2(ish) is installed (with the WebEngine package) as well as cmake.<br>
You build in a separate directory and not in the source directory.

Qt 6.7.2 or later is preferred.

Get the PageEdit Source:

`git clone https://github.com/sigil-ebook/PageEdit.git`<br>
(or download/extract the gzipped tarball from a PageEdit Release)<br>
`cd PageEdit`

__Ubuntu__

`sudo apt-get install build-essential`<br>
`sudo apt-get install cmake`<br>
`sudo apt-get install qt6-webengine-dev qt6-webengine-dev-tools qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qt6-svg-dev`

`mkdir build`<br>
`cd build`<br>
`cmake "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../<path/to/PageEdit/source>`<br>
`make`<br>

__Arch Linux__

sudo pacman -S cmake qt6-webengine qt6-tools qt6-svg<br>

`mkdir build`<br>
`cd build`<br>
`cmake "Unix Makefiles" -DINSTALL_BUNDLED_DICTS=0 -DCMAKE_BUILD_TYPE=Release ../<path/to/PageEdit/source>`<br>
`make`<br>

__All Flavors__:

To test
`cd bin`<br>
`./pageedit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

`make install` (defaults to the /usr/local prefix, so use sudo if required).

By default, bundled hunspell dictionaries are converted to webengine spell check dictionaries (with qt tools) and installed to the QT_INSTALL_DATA/qtwebengine_dictionaries location. This location can be found using the qmake binary (of the Qt you're building PageEdit with) with the following command `-qmake -query QT_INSTALL_DATA` . If using the system Qt, this will typically be /usr/share/qt/qtwebengine_dictionaries. To disable the conversion/installation of these bundled dictionaries, use -DNSTALL_BUNDLED_DICTS=0 when configuring PageEdit with cmake. Arch Linux already includes these dictionaries with the corresponding hunspell language dictionaries. So you'll want to skip their conversion/installation on Arch and make sure you have the correct hunspell languages installed instead.

On Windows:
-----------

Qt6.7.2 or higher is preferred, but you'll want to maintain a  minimum of Qt6.4.2 (with the WebEngine component) if you want to take advantage of all features. A minimum of Visual Studio 2022 is required if you want to use Qt6.7.2. At the time of this writing Qt6.8.2 is included in the official installer. Install cmake 3.18+ and make sure its bin directory is added to your path.

Make sure that Qt's bin directory is also added to your PATH. Take note of the path for your Qt's prefix (the directory right above the bin directory). Open an "x64 Native Tools Command Prompt for VS2022" from the Start menu and cd to wherever you want to build.

`mkdir build`<br>
`cd build`<br>
`cmake -G "NMake Makefiles" -DQt6_DIR="C:\path\to\your\Qt6\prefix\lib\cmake\Qt6 -DCMAKE_BUILD_TYPE=Release "C:\path\to\PageEdit's\cloned\repository"`<br>
`nmake`<br>

To test:
`cd bin`<br>
`PageEdit.exe \FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB\FILENAME.xhtml`

Use `nmake deployzip` to package PageEdit as a zip archive (in a newly created 'deploy' folder in the build folder)

Use `nmake deployinstaller` to package PageEdit and all of its dependencies into an Inno Setup installer (must have InnoSetup installed and on the PATH for this to work).

__NOTE__: if you configure PageEdit with the -DDEPLOY_SFX=1 cmake option before compiling, 'nmake deployzip` will attempt to create a 7-Zip self-extracting archive. So naturally, make sure 7-Zip is installed before trying to use it.

__NOTE__: as of this writing, the Official PageEdit release are built using Qt6.9.3


## For Github repository maintainers (who am I kidding, this is to keep myself from forgetting/messing up):

The upstream sigil-gumbo repository is included in PageEdit's repository (this one) as a subtree in the gumbo_subtree prefix. To pull in upstream sigil-gumbo changes (or even to safely check if there _are_ any), use the following command in the root of your local clone of the PageEdit repo:

`git subtree pull --prefix gumbo_subtree https://github.com/Sigil-Ebook/sigil-gumbo.git master --squash`

--squash because we don't need sigil-gumbo's entire history in the PageEdit repository. Push the changes to github master (with a commit message like "merge in upstream sigil-gumbo changes") if there are any.

You can also create a remote for the upstream sigil-gumbo repo to simplify the subtree pull command a bit -- BUT YOU MUST REMEMBER TO USE THE --no-tags OPTION WHEN CREATING THE REMOTE. Otherwise the --squash option may not suppress all upstream history when using the remote name. A careless `git fetch --all` could make a dog's lunch of your repo history with a very taggy remote (which sigil-gumbo is decidedly not):

`git remote add --no-tags sigil-gumbo https://github.com/Sigil-Ebook/sigil-gumbo.git`

After that, pulling in any upstream sigil-gumbo changes becomes:

`git subtree pull --prefix gumbo_subtree sigil-gumbo master --squash`

Probably safer in the long run to create a git alias specific to the PageEdit repository (without a remote added) using the full subtree pull command if you need a shortcut. From within the PageEdit repo:

`git config alias.gumbo-sub-pull 'subtree pull --prefix gumbo_subtree https://github.com/Sigil-Ebook/sigil-gumbo.git master --squash'`

Use any alias name you like. I chose "gumbo-sub-pull"  Then it's simply a matter of using `git gumbo-sub-pull` in the root of the PageEdit repository.


# <center>Building the PageEdit AppImage on Linux</center>

## General Overview

**Warning! Do not attempt to run any of these scripts outside of a Docker container. Doing so could damage your system. All scripts provided are intended to be used inside of a temporary docker container that will be removed after the process completes.**

The only requirement for building the PageEdit Linux AppImage is having Docker installed/configured and having an internet connection. How to get Docker installed and configured on your system is beyond the scope of this document. Every distro is different, so you're on your own there. Everything after this assumes that Docker is ready to go. Typing `docker info` in a terminal should verify everything is ready.

There are two separate pieces that are needed for building the AppImage. These are a custom build of Python and a custom build of QtWebEngine. The custom Python is only used to get later versions of Qt installed in the Docker container. The AppImage build process will automatically download these pieces from my [personal github repository](https://github.com/dougmassay/win-qtwebkit-5.212/releases/tag/v5.212-1). The files used from there are sigilpython3.\*.\*.tar.gz and appimagewebengine6\*.tar.gz. These two pieces can also be fairly easily built if you wish. Should you desire to build everything from scratch, the instructions to do so are in the matching document in the Sigil AppImage build instructions in the Sigil repository.

The building of the PageEdit AppImage takes place entirely within a docker container built from a stock Ubuntu 22.04 image. No system changes or updates will be made to your host system during this process. Once the Docker process completes, the PageEdit AppImage (and its corresonding zsync file) will be found in in the same directory the build process was launched from.

Everything is done by entering commands in a terminal.

### Get PageEdit's Source

You can either download PageEdit's source code from [GitHub](https://github.com/Sigil-Ebook/PageEdit), or you can clone PageEdit's repository using git `git clone https://github.com/Sigil-Ebook/PageEdit.git pageedit`. Your choice. These built pieces will be ignored by git, so don't worry about contaminating the source tree.

### CD to the root of PageEdit's source

`cd pageedit` or `cd pageedit-master` or whatever. You need to be in the same directory that PageEdit's README.md, ChangeLog.txt, and docker-compose.yml files are in.

### Build the AppImage

If you have docker-compose installed, you can build the AppImage with the following simple command:

`docker compose run --rm build_appimage`

If you don't have docker-compose installed, use the following command instead:

`docker run --rm -v $PWD:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_pageedit_appimage.sh`

Once completed, the PageEdit-\*-x86_64.AppImage file (as well as the PageEdit-\*-x86_64.AppImage.zsync file - which can be safely ignored/removed) will be located in the current directory. Depending on how your docker permissions are configured, you may need to take ownership of the file(s) with `sudo chown <user>:<user> PageEdit-*AppImage*`. The file should already be executable, but if not just fix it up with `chmod a+x PageEdit-*AppImage`.