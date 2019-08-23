PageEdit
========
An ePub visual XHTML editor based on Sigil's Deprecated BookView.
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
``<br>
`or edit a set of XHtml files in epub Spine Order by loading the ePub's OPF file`<br>

On Linux
--------

Make sure a minimum of Qt5.9.4(ish) is installed (with the WebEngine package) as well as cmake

`mkdir build`<br>
`cd build`<br>
`cmake "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../PageEdit`<br>
`make`<br>

To test
`cd bin`<br>
`./pageedit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

You can also install PageEdit with `make install` (defaults to the /usr/local prefix, so use sudo if required).

__NOTE__: certain features, like Inspector, will require a minimum of Qt5.11.x to function on Linux.

By default, bundled hunspell dictionaries are converted to webengine spell check dictionaries (with qt tools) and installed to the QT_INSTALL_DATA/qtwebengine_dictionaries location. This location can be found using the qmake binary (of the Qt you're building PageEdit with) with the following command `-qmake -query QT_INSTALL_DATA` . If using the system Qt, this will typically be /usr/share/qt/qtwebengine_dictionaries. To disable the conversion/installation of these bundled dictionaries, use -DNSTALL_BUNDLED_DICTS=0 when configuring PageEdit with cmake.

On Windows:
-----------

Make sure Qt5.12.3 (with the WebEngine component) and Visual Studio 2017 are installed. Install the x64 version of Qt5.12.3 for VS2017. Install cmake 3.0+ and make sure its bin directory is added to your path.

Make sure that Qt's bin directory is also added to your PATH. Take note of the path for your Qt's prefix (the directory right above the bin directory). Open an "x64 Native Tools Command Prompt for VS2017" from the Start menu and cd to wherever you want to build.

`mkdir build`<br>
`cd build`<br>
`cmake -G "NMake Makefiles" -DQt5_DIR="C:\path\to\your\Qt5\prefix\lib\cmake\Qt5 -DCMAKE_BUILD_TYPE=Release "C:\path\to\PageEdit's\cloned\repository"`<br>
`nmake`<br>

To test:
`cd bin`<br>
`PageEdit.exe \FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB\FILENAME.xhtml`

Use `nmake deploy` to package PageEdit and all of its dependencies into a zip archive (in a newly created 'deploy' folder in the build folder)

__NOTE__: if you configure PageEdit with the -DDEPLOY_SFX=1 cmake option before compiling, 'nmake deploy` will attempt to create a 7-Zip self-extracting archive. So naturally, make sure 7-Zip is installed before trying to use it.


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



