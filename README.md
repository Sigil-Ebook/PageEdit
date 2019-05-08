PageEdit
A simple single page visual XHTML editor based on Sigil's Deprecated BookView.
It uses WebEngine instead of WebKit.

This app is undergoing rapid development and is incomplete and barely does anything yet.

It requires Qt 5.12.3 or later.

To build after cloning the repo

Make sure qmake from Qt 5.12.3 in your path
`export PATH=~/QT512/bin:${PATH}`

`cd PageEdit`<br>
`qmake`<br>
`make`


To test:

On macOS:

`open ./PageEdit.app --args /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`

On Linux

`./PageEdit /FULL_PATH_TO_AN_XHTML_FILE_IN_AN_UNZIPPED_EPUB/FILENAME.xhtml`
