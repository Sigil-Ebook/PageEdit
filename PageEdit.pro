QT += widgets \
    core \
    gui \
    webengine \
    webenginewidgets

TARGET = PageEdit
TEMPLATE = app

SOURCES += \
    main.cpp\
    pageedit_constants.cpp\
    MainApplication.cpp\
    MainWindow.cpp\
    SelectCharacter.cpp\
    WebPageEdit.cpp\
    WebViewEdit.cpp\
    Inspector.cpp\
    Preferences.cpp\
    AppearanceWidget.cpp\
    LanguageWidget.cpp\
    UILanguage.cpp\
    SettingsStore.cpp\
    Utility.cpp

HEADERS += \
    pageedit_constants.h\
    pageedit_exception.h\
    MainApplication.h\
    MainWindow.h\
    Zoomable.h\
    Viewer.h\
    ElementIndex.h\
    SelectCharacter.h\
    WebPageEdit.h\
    WebViewEdit.h\
    Inspector.h\
    Preferences.h\
    PreferencesWidget.h\
    AppearanceWidget.h\
    LanguageWidget.h\
    UILanguage.h\
    SettingsStore.h\
    Utility.h

FORMS += \
    main.ui\
    SelectCharacter.ui\
    Preferences.ui\
    PAppearanceWidget.ui\
    PLanguageWidget.ui

RESOURCES += \
    icons/icons.qrc\
    javascript/javascript.qrc

