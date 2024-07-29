/************************************************************************
**
**  Copyright (C) 2019-2024  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2024  Doug Massay
**  Copyright (C) 2012       John Schember <john@nachtimwald.com>
**  Copyright (C) 2012       Grant Drake
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

#include <QString>
#include <QColorDialog>
#include <QListWidget>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QtWebEngineWidgets>
#include <QWebEngineSettings>
#include <QFontDialog>

#include "AppearanceWidget.h"
#include "SettingsStore.h"
#include "Utility.h"

AppearanceWidget::AppearanceWidget()
{

    ui.setupUi(this);

    // Hide the Windows only preference from all other OSes
#ifndef Q_OS_WIN32
    ui.grpCustomDarkStyle->setVisible(false);
#endif

    m_uiFontResetFlag = false;

    readSettings();
    connectSignalsToSlots();
}

PreferencesWidget::ResultActions AppearanceWidget::saveSettings()
{
    SettingsStore settings;
    settings.setAppearancePrefsTabIndex(ui.tabAppearance->currentIndex());
    settings.setPreviewDark(ui.PreviewDarkInDM->isChecked() ? 1 : 0);
    // This setting has no effect on other OSes, but it won't hurt to set it.
    settings.setUiUseCustomDarkTheme(ui.chkDarkStyle->isChecked() ? 1 : 0);
    settings.setUIFont(m_currentUIFont);

    SettingsStore::WebViewAppearance WVAppearance;
    WVAppearance.font_family_standard     = ui.cbWebViewFontStandard->currentText();
    WVAppearance.font_family_serif        = ui.cbWebViewFontSerif->currentText();
    WVAppearance.font_family_sans_serif   = ui.cbWebViewFontSansSerif->currentText();
    WVAppearance.font_size                = ui.webViewFontSizeSpin->value();
    settings.setWebViewAppearance(WVAppearance);

    SettingsStore::SpecialCharacterAppearance specialCharacterAppearance;
    specialCharacterAppearance.font_family = ui.cbSpecialCharacterFont->currentText();
    specialCharacterAppearance.font_size   = ui.specialCharacterFontSizeSpin->value();
    settings.setSpecialCharacterAppearance(specialCharacterAppearance);

    settings.setMainMenuIconSize(double(ui.iconSizeSlider->value())/10);

    // WV settings can be globally changed and will take effect immediately
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize,   WVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont,    WVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont,       WVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont,   WVAppearance.font_family_sans_serif);

    settings.setSkipPrintPreview(ui.chkSkipPrintPreview->isChecked());
    settings.setPrintDPI(ui.cboPrintDPI->currentText().toInt());

    PreferencesWidget::ResultActions results = PreferencesWidget::ResultAction_None;

    if (m_PreviewDark != (ui.PreviewDarkInDM->isChecked() ? 1 : 0)) {
        results = results | PreferencesWidget::ResultAction_ReloadPreview;
    }
    if ((m_currentUIFont != m_initUIFont) || m_uiFontResetFlag) {
        results = results | PreferencesWidget::ResultAction_RestartPageEdit;
    }
    // if dark style changed on Windows, set need for restart.
    // This setting has no effect on other OSes so no need to prompt for a restart.
#ifdef Q_OS_WIN32
    if (m_UseCustomDarkTheme != (ui.chkDarkStyle->isChecked() ? 1 : 0)) {
        results = results | PreferencesWidget::ResultAction_RestartPageEdit;
    }
#endif
    m_uiFontResetFlag = false;
    results = results & PreferencesWidget::ResultAction_Mask;
    return results;
}


void AppearanceWidget::readSettings()
{
    SettingsStore settings;
    ui.tabAppearance->setCurrentIndex(settings.appearancePrefsTabIndex());
    // This setting has no effect on other OSes, but it won't hurt to read it.
    m_UseCustomDarkTheme = settings.uiUseCustomDarkTheme();
    ui.chkDarkStyle->setChecked(settings.uiUseCustomDarkTheme());

    SettingsStore::WebViewAppearance WVAppearance = settings.webViewAppearance();
    SettingsStore::SpecialCharacterAppearance specialCharacterAppearance = settings.specialCharacterAppearance();
    loadComboValueOrDefault(ui.cbWebViewFontStandard,  WVAppearance.font_family_standard,    "Arial");
    loadComboValueOrDefault(ui.cbWebViewFontSerif,     WVAppearance.font_family_serif,       "Times New Roman");
    loadComboValueOrDefault(ui.cbWebViewFontSansSerif, WVAppearance.font_family_sans_serif,  "Arial");
    loadComboValueOrDefault(ui.cbSpecialCharacterFont,  specialCharacterAppearance.font_family,     "Helvetica");
    ui.webViewFontSizeSpin->setValue(WVAppearance.font_size);
    ui.specialCharacterFontSizeSpin->setValue(specialCharacterAppearance.font_size);
    ui.iconSizeSlider->setValue(int(settings.mainMenuIconSize()*10));
    if (!settings.uiFont().isEmpty()) {
        m_initUIFont = settings.uiFont();
    } else {
        m_initUIFont = settings.originalUIFont();
    }
    m_currentUIFont = m_initUIFont;
    updateUIFontDisplay();
    m_PreviewDark = settings.previewDark();
    ui.PreviewDarkInDM->setChecked(settings.previewDark());

    ui.cboPrintDPI->addItems({ "72", "96", "168", "300", "600", "1200" });
    int index = ui.cboPrintDPI->findText(QString::number(settings.printDPI()));
    if (index == -1) {
        index = ui.cboPrintDPI->findText("300");
        if (index == -1) {
            index = 0;
        }
    }
    ui.cboPrintDPI->setCurrentIndex(index);
}

void AppearanceWidget::updateUIFontDisplay()
{
    QFont f;
    f.fromString(m_currentUIFont);
    ui.editUIFont->setText(f.family() + QString(" - %1pt").arg(f.pointSize()));
    ui.editUIFont->setReadOnly(true);
}

void AppearanceWidget::changeUIFontButtonClicked()
{
    bool ok;
    QFont f;
    f.fromString(m_currentUIFont);
    QFont font = QFontDialog::getFont(&ok, f, this);
    if (ok) {
        m_currentUIFont = font.toString();
        updateUIFontDisplay();
    }
}

void AppearanceWidget::loadComboValueOrDefault(QFontComboBox *fontComboBox, const QString &value, const QString &defaultValue)
{
    int index = fontComboBox->findText(value);

    if (index == -1) {
        index = fontComboBox->findText(defaultValue);

        if (index == -1) {
            index = 0;
        }
    }

    fontComboBox->setCurrentIndex(index);
}

void AppearanceWidget::resetAllButtonClicked()
{
    SettingsStore settings;
    settings.clearAppearanceSettings();
    m_uiFontResetFlag = true;
    m_initUIFont = settings.originalUIFont();
    updateUIFontDisplay();
}

void AppearanceWidget::newSliderValue(int value) {
    SettingsStore settings;
    settings.setMainMenuIconSize(double(ui.iconSizeSlider->value())/10);
    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = dynamic_cast<MainWindow *>(mainWindow_w);
    mainWindow->sizeMenuIcons();
}

void AppearanceWidget::connectSignalsToSlots()
{
    connect(ui.resetAllButton,    SIGNAL(clicked()), this, SLOT(resetAllButtonClicked()));
    connect(ui.iconSizeSlider,    SIGNAL(valueChanged(int)), this, SLOT(newSliderValue(int)));
    connect(ui.changeUIFontButton, SIGNAL(clicked()), this, SLOT(changeUIFontButtonClicked()));
}
