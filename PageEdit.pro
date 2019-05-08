QT += widgets \
    core \
    gui \
    webengine \
    webenginewidgets

TARGET = PageEdit
TEMPLATE = app

SOURCES += \
    main.cpp\
    MainApplication.cpp\
    MainWindow.cpp\
    Inspector.cpp\
    SelectCharacter.cpp\
    SettingsStore.cpp\
    Utility.cpp\
    WebPageEdit.cpp\
    WebViewEdit.cpp

HEADERS += \
    MainApplication.h\
    MainWindow.h\
    ElementIndex.h\
    Inspector.h\
    pageedit_exception.h\
    SelectCharacter.h\
    SettingsStore.h\
    Utility.h\
    WebPageEdit.h\
    WebViewEdit.h\
    Zoomable.h\
    Viewer.h

FORMS += \
    main.ui\
    SelectCharacter.ui

RESOURCES += \
    icons/icons.qrc\
    javascript/javascript.qrc
	
	


