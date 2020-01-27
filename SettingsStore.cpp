/************************************************************************
**
**  Copyright (C) 2016-2020 Kevin B. Hendricks, Stratford, ON
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

#include <QtCore/QLocale>
#include <QtCore/QCoreApplication>
#include <QFile>
#include <QDir>

#include "SettingsStore.h"
#include "Utility.h"

static const float ZOOM_NORMAL = 1.0f;
static QString SETTINGS_GROUP = "user_preferences";
static QString KEY_UI_LANGUAGE = SETTINGS_GROUP + "/" + "ui_language";
static QString KEY_UI_DICTIONARY = SETTINGS_GROUP + "/" + "ui_dictionary";

static QString KEY_UI_FONT = SETTINGS_GROUP + "/" + "ui_font";
static QString KEY_ORIGINAL_UI_FONT = SETTINGS_GROUP + "/" + "original_ui_font";

static QString KEY_HIGHDPI_SETTING = SETTINGS_GROUP + "/" + "high_dpi";
static QString KEY_PREVIEW_DARK_IN_DM = SETTINGS_GROUP + "/" + "preview_dark_in_dm";

static QString KEY_SPELL_CHECK = SETTINGS_GROUP + "/" + "spellcheck";
static QString KEY_ZOOM_IMAGE = SETTINGS_GROUP + "/" + "zoom_image";
static QString KEY_ZOOM_TEXT = SETTINGS_GROUP + "/" + "zoom_text";
static QString KEY_ZOOM_WEB = SETTINGS_GROUP + "/" + "zoom_web";
static QString KEY_ZOOM_PREVIEW = SETTINGS_GROUP + "/" + "zoom_preview";
static QString KEY_ZOOM_INSPECTOR = SETTINGS_GROUP + "/" + "zoom_inspector";
static QString KEY_APPEARANCE_PREFS_TAB_INDEX = SETTINGS_GROUP + "/" + "appearance_prefs_tab_index";
static QString KEY_WEBVIEW_FONT_FAMILY_STANDARD = SETTINGS_GROUP + "/" + "webview_font_family_standard";
static QString KEY_WEBVIEW_FONT_FAMILY_SERIF = SETTINGS_GROUP + "/" + "webview_font_family_serif";
static QString KEY_WEBVIEW_FONT_FAMILY_SANS_SERIF = SETTINGS_GROUP + "/" + "webview_font_family_sans_serif";
static QString KEY_WEBVIEW_FONT_SIZE = SETTINGS_GROUP + "/" + "webview_font_size";
static QString KEY_SPECIAL_CHARACTER_FONT_FAMILY = SETTINGS_GROUP + "/" + "special_character_font_family";
static QString KEY_SPECIAL_CHARACTER_FONT_SIZE = SETTINGS_GROUP + "/" + "special_character_font_size";
static QString KEY_MAIN_MENU_ICON_SIZE = SETTINGS_GROUP + "/" + "main_menu_icon_size";
static QString KEY_JAVASCRIPT_ON = SETTINGS_GROUP + "/" + "javascript_on";
static QString KEY_REMOTE_ON = SETTINGS_GROUP + "/" + "remote_on";
static QString KEY_USE_PRETTIFY = SETTINGS_GROUP + "/" + "use_prettify";
static QString KEY_USE_WSPREWRAP = SETTINGS_GROUP + "/" + "use_white_space_pre_wrap";


SettingsStore::SettingsStore()
    : QSettings(Utility::DefinePrefsDir() + "/pageedit.ini", QSettings::IniFormat)
{  
    // See QTBUG-40796 and QTBUG-54510 as using UTF-8 as a codec for ini files is very broken
    // setIniCodec("UTF-8");
}

SettingsStore::SettingsStore(QString filename)
    : QSettings(filename, QSettings::IniFormat)
{
    // See QTBUG-40796 and QTBUG-54510 as using UTF-8 as a codec for ini files is very broken
    // setIniCodec("UTF-8");
}

QString SettingsStore::uiLanguage()
{
    clearSettingsGroup();
    return value(KEY_UI_LANGUAGE, QLocale::system().name()).toString();
}

QString SettingsStore::uiDictionary()
{
    clearSettingsGroup();
    return value(KEY_UI_DICTIONARY, "en_US").toString();
}

QString SettingsStore::uiFont()
{
    clearSettingsGroup();
    return value(KEY_UI_FONT, "").toString();
}

QString SettingsStore::originalUIFont()
{
    clearSettingsGroup();
    return value(KEY_ORIGINAL_UI_FONT, "").toString();
}

int SettingsStore::highDPI()
{
    clearSettingsGroup();
    return value(KEY_HIGHDPI_SETTING, 0).toInt();
}

int SettingsStore::previewDark()
{
    clearSettingsGroup();
    return value(KEY_PREVIEW_DARK_IN_DM, 1).toInt();
}

#if 0
bool SettingsStore::spellCheck()
{
  clearSettingsGroup();
  return static_cast<bool>(value(KEY_SPELL_CHECK, false).toBool());
}
#endif

float SettingsStore::zoomImage()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_IMAGE, ZOOM_NORMAL).toFloat();;
}

float SettingsStore::zoomText()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_TEXT, ZOOM_NORMAL).toFloat();
}

float SettingsStore::zoomWeb()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_WEB, ZOOM_NORMAL).toFloat();
}

float SettingsStore::zoomPreview()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_PREVIEW, ZOOM_NORMAL).toFloat();
}

float SettingsStore::zoomInspector()
{
    clearSettingsGroup();
    return value(KEY_ZOOM_INSPECTOR, ZOOM_NORMAL).toFloat();
}

int SettingsStore::appearancePrefsTabIndex() {
    clearSettingsGroup();
    return value(KEY_APPEARANCE_PREFS_TAB_INDEX, 0).toInt();
}

SettingsStore::WebViewAppearance SettingsStore::webViewAppearance()
{
    clearSettingsGroup();
    SettingsStore::WebViewAppearance appearance;
    appearance.font_family_standard = value(KEY_WEBVIEW_FONT_FAMILY_STANDARD, "Arial").toString();
    appearance.font_family_serif = value(KEY_WEBVIEW_FONT_FAMILY_SERIF, "Times New Roman").toString();
    appearance.font_family_sans_serif = value(KEY_WEBVIEW_FONT_FAMILY_SANS_SERIF, "Arial").toString();
    appearance.font_size = value(KEY_WEBVIEW_FONT_SIZE, 16).toInt();
    return appearance;
}

SettingsStore::SpecialCharacterAppearance SettingsStore::specialCharacterAppearance()
{
    clearSettingsGroup();
    SettingsStore::SpecialCharacterAppearance appearance;
    appearance.font_family = value(KEY_SPECIAL_CHARACTER_FONT_FAMILY, "Arial").toString();
    appearance.font_size = value(KEY_SPECIAL_CHARACTER_FONT_SIZE, 14).toInt();
    return appearance;
}

double SettingsStore::mainMenuIconSize()
{
    clearSettingsGroup();
    return value(KEY_MAIN_MENU_ICON_SIZE, 1.8).toDouble();
}

int SettingsStore::javascriptOn()
{
  clearSettingsGroup();
  return value(KEY_JAVASCRIPT_ON, 0).toInt();
}

int SettingsStore::remoteOn()
{
  clearSettingsGroup();
  return value(KEY_REMOTE_ON, 0).toInt();
}

int SettingsStore::usePrettify()
{
  clearSettingsGroup();
  return value(KEY_USE_PRETTIFY, 0).toInt();
}

int SettingsStore::useWSPreWrap()
{
  clearSettingsGroup();
  return value(KEY_USE_WSPREWRAP, 0).toInt();
}

void SettingsStore::setUILanguage(const QString &language_code)
{
    clearSettingsGroup();
    setValue(KEY_UI_LANGUAGE, language_code);
}

void SettingsStore::setUIDictionary(const QString &dictionary_name)
{
    clearSettingsGroup();
    setValue(KEY_UI_DICTIONARY, dictionary_name);
}

void SettingsStore::setUIFont(const QString &font_data)
{
    clearSettingsGroup();
    setValue(KEY_UI_FONT, font_data);
}

void SettingsStore::setOriginalUIFont(const QString &font_data)
{
    clearSettingsGroup();
    setValue(KEY_ORIGINAL_UI_FONT, font_data);
}

void SettingsStore::setHighDPI(int value)
{
    clearSettingsGroup();
    setValue(KEY_HIGHDPI_SETTING, value);
}

void SettingsStore::setPreviewDark(int enabled)
{
    clearSettingsGroup();
    setValue(KEY_PREVIEW_DARK_IN_DM, enabled);
}

#if 0
void SettingsStore::setSpellCheck(bool enabled)
{
  clearSettingsGroup();
  setValue(KEY_SPELL_CHECK, enabled);
}
#endif

void SettingsStore::setZoomImage(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_IMAGE, zoom);
}

void SettingsStore::setZoomText(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_TEXT, zoom);
}

void SettingsStore::setZoomWeb(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_WEB, zoom);
}

void SettingsStore::setZoomPreview(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_PREVIEW, zoom);
}

void SettingsStore::setZoomInspector(float zoom)
{
    clearSettingsGroup();
    setValue(KEY_ZOOM_INSPECTOR, zoom);
}

void SettingsStore::setAppearancePrefsTabIndex(int index) {
    clearSettingsGroup();
    setValue(KEY_APPEARANCE_PREFS_TAB_INDEX, index);
}

void SettingsStore::setWebViewAppearance(const SettingsStore::WebViewAppearance &webview_appearance)
{
    clearSettingsGroup();
    setValue(KEY_WEBVIEW_FONT_FAMILY_STANDARD, webview_appearance.font_family_standard);
    setValue(KEY_WEBVIEW_FONT_FAMILY_SERIF, webview_appearance.font_family_serif);
    setValue(KEY_WEBVIEW_FONT_FAMILY_SANS_SERIF, webview_appearance.font_family_sans_serif);
    setValue(KEY_WEBVIEW_FONT_SIZE, webview_appearance.font_size);
}

void SettingsStore::setSpecialCharacterAppearance(const SettingsStore::SpecialCharacterAppearance &special_character_appearance)
{
    clearSettingsGroup();
    setValue(KEY_SPECIAL_CHARACTER_FONT_FAMILY, special_character_appearance.font_family);
    setValue(KEY_SPECIAL_CHARACTER_FONT_SIZE, special_character_appearance.font_size);
}

void SettingsStore::setMainMenuIconSize(double icon_size)
{
    clearSettingsGroup();
    setValue(KEY_MAIN_MENU_ICON_SIZE, icon_size);
}

void SettingsStore::setJavascriptOn(int on)
{
  clearSettingsGroup();
  setValue(KEY_JAVASCRIPT_ON, on);
}

void SettingsStore::setRemoteOn(int on)
{
  clearSettingsGroup();
  setValue(KEY_REMOTE_ON, on);
}

void SettingsStore::setUsePrettify(int on)
{
  clearSettingsGroup();
  setValue(KEY_USE_PRETTIFY, on);
}
 
void SettingsStore::setUseWSPreWrap(int on)
{
  clearSettingsGroup();
  setValue(KEY_USE_WSPREWRAP, on);
}

void SettingsStore::clearAppearanceSettings()
{
    clearSettingsGroup();
    remove(KEY_WEBVIEW_FONT_FAMILY_STANDARD);
    remove(KEY_WEBVIEW_FONT_FAMILY_SERIF);
    remove(KEY_WEBVIEW_FONT_FAMILY_SANS_SERIF);
    remove(KEY_WEBVIEW_FONT_SIZE);
    remove(KEY_SPECIAL_CHARACTER_FONT_FAMILY);
    remove(KEY_SPECIAL_CHARACTER_FONT_SIZE);
    remove(KEY_MAIN_MENU_ICON_SIZE);
}

void SettingsStore::clearSettingsGroup()
{
    while (!group().isEmpty()) {
        endGroup();
    }
}
