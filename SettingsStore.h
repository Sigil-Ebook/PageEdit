/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2011-2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013 Dave Heiland
**
**  This file is part of PageEdit.
**
**  PageEdit is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  PageEdit is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with PageEdit.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef SETTINGSSTORE_H
#define SETTINGSSTORE_H

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <utility>

/**
 * Provides access for reading and writing user configurable
 * settings. This should be used instead of QSettings because it
 * sets up the settings to use INI format on all platforms except
 * OS X. Also, it implements a variety of settings that are used in
 * a large number of places throughout the application.
 */
class SettingsStore : public QSettings
{
    Q_OBJECT

public:
    SettingsStore();
    SettingsStore(QString filename);

    /**
     * The langauge to use for the user interface
     *
     * @return The language as a string.
     */
    QString uiLanguage();

    QString uiDictionary();

    QString uiFont();

    QString originalUIFont();

    int highDPI();

    int previewDark();


    // bool spellCheck();

    /**
     * The zoom factor used by the component.
     *
     * @return The zoom factor.
     */
    float zoomImage();
    float zoomText();
    float zoomWeb();
    float zoomPreview();
    float zoomInspector();

    /**
     * All appearance settings related to PageEdit.
     */
    struct WebViewAppearance {
      QString font_family_standard;
      QString font_family_serif;
      QString font_family_sans_serif;
      int font_size;
    };

    /**
     * All appearance settings related to Special Characters.
     */
    struct SpecialCharacterAppearance {
        QString font_family;
        int font_size;
    };


    int appearancePrefsTabIndex();

    /**
     * The appearance settings to use for the WebView.
     */
    WebViewAppearance webViewAppearance();

    /**
     * The appearance settings to use for Special Characters.
     */
    SpecialCharacterAppearance specialCharacterAppearance();

    /**
     * The icon size to use for the main menu.
     */
    double mainMenuIconSize();
    
    void clearAppearanceSettings();

    int javascriptOn();

    int remoteOn();

    int usePrettify();

    int useWSPreWrap();

    bool skipPrintWarnings();

    bool skipPrintPreview();

public slots:

    /**
     * Set the language to use for the user interface
     *
     * @param lang The language to set.
     */
    void setUILanguage(const QString &language_code);

    void setUIDictionary(const QString &dictionary_name);

    void setUIFont(const QString &font_data);

    void setOriginalUIFont(const QString &font_data);

    void setHighDPI(int value);

    void setPreviewDark(int enabled);

    // void setSpellCheck(bool enabled);

    /**
     * Set the zoom factor used by the component.
     *
     * @param zoom The zoom factor.
     */
    void setZoomImage(float zoom);
    void setZoomText(float zoom);
    void setZoomWeb(float zoom);
    void setZoomPreview(float zoom);
    void setZoomInspector(float zoom);

    void setAppearancePrefsTabIndex(int index);

    /**
     * Set the default font settings to use for rendering Book View/Preview
     */
    void setWebViewAppearance(const WebViewAppearance &webview_appearance);

    /**
    * Set the default font settings to use for Special Characters popup window
    */
    void setSpecialCharacterAppearance(const SpecialCharacterAppearance &special_character_appearance);

    /**
     * Set the icon size to use for the main menu.
     */
    void setMainMenuIconSize(double icon_size);

    void setJavascriptOn(int on);

    void setRemoteOn(int on);

    void setUsePrettify(int on);

    void setUseWSPreWrap(int on);

    void setSkipPrintWarnings(bool skip);

    void setSkipPrintPreview(bool skip);

private:
    /**
     * Ensures there is not an open settings group which will cause the settings
     * this class implements to be set in the wrong place.
     */
    void clearSettingsGroup();
};

#endif // SETTINGSSTORE_H
