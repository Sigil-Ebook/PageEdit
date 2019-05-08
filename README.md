PageEdit
A simple single page visual XHTML editor based on Sigil's Deprecated BookView.
It uses WebEngine instead of WebKit.

This app is undergoing rapid development and is incomplete and barely does anything yet.

It requires Qt 5.12.3 or later.

To build after cloning the repo

Make sure qmake from Qt 5.12.3 in your path<br>
`export PATH=~/QT512/bin:${PATH}`

`cd PageEdit`<br>
`qmake`<br>
`make`


To test:

On macOS:

`open ./PageEdit.app --args /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

On Linux

`./PageEdit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

On Windows:

Although the PageEdit can be built by issuing similar command-line options used in the macOS and on Linux instructions above, it's probably much easier to configure QtCreator with the Qt5.12.3 kit (and VS2017) and open the the PageEdit.pro file. From there, you'l be able to configure/build/debug PageEdit without having to manually configure the Qt environment for the command prompt.
