/************************************************************************
**
**  Copyright (C) 2019-2020  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2020  Doug Massay
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
    : m_isHighDPIComboEnabled(true)
{

#if defined(Q_OS_MAC) || QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Disable the HighDPI combobox on Mac
    // Effectively an isMacOS runtime check
    // Also needed if Qt >= 6.0.0
    m_isHighDPIComboEnabled = false;
#endif

    ui.setupUi(this);

    // setup the HighDPI combo box here
    ui.comboHighDPI->addItems({tr("Detect"), tr("On"), tr("Off")});
    QString highdpi_tooltip = "<p><dt><b>" + tr("Detect") + "</b><dd>" + tr("Detect whether any high dpi scaling should take place.");
    highdpi_tooltip += " " + tr("Defers to any Qt environment variables that are set to control high dpi behavior.") + "</dd>";
    highdpi_tooltip += "<dt><b>" + tr("On") + "</b><dd>" + tr("Turns on high dpi scaling and ignores any Qt environment variables");
    highdpi_tooltip += " " + tr("that are set controlling high dpi behavior.") + "</dd>";
    highdpi_tooltip += "<dt><b>" + tr("Off") + "</b><dd>" + tr("Turns off high dpi scaling regardless if any Qt environment");
    highdpi_tooltip += " " + tr("variables controlling high dpi behavior are set.") + "</dd>";
    ui.comboHighDPI->setToolTip(highdpi_tooltip);

    // The HighDPI setting is unused/unnecessary on Mac
    ui.comboHighDPI->setEnabled(m_isHighDPIComboEnabled);

    m_uiFontResetFlag = false;

    readSettings();
    connectSignalsToSlots();
}

PreferencesWidget::ResultActions AppearanceWidget::saveSettings()
{
    SettingsStore settings;
    settings.setAppearancePrefsTabIndex(ui.tabAppearance->currentIndex());
    settings.setPreviewDark(ui.PreviewDarkInDM->isChecked() ? 1 : 0);
    // Don't try to get the index of a disabled combobox
    if (m_isHighDPIComboEnabled) {
        settings.setHighDPI(ui.comboHighDPI->currentIndex());
    }
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();
#else
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
#endif
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
    // Don't try to get the index of a disabled combobox
    if (m_isHighDPIComboEnabled) {
        if (m_HighDPI != (ui.comboHighDPI->currentIndex())) {
            results = results | PreferencesWidget::ResultAction_RestartPageEdit;
        }
    }
    if ((m_currentUIFont != m_initUIFont) || m_uiFontResetFlag) {
        results = results | PreferencesWidget::ResultAction_RestartPageEdit;
    }
    m_uiFontResetFlag = false;
    results = results & PreferencesWidget::ResultAction_Mask;
    return results;
}


void AppearanceWidget::readSettings()
{
    SettingsStore settings;
    ui.tabAppearance->setCurrentIndex(settings.appearancePrefsTabIndex());
    SettingsStore::WebViewAppearance WVAppearance = settings.webViewAppearance();
    SettingsStore::SpecialCharacterAppearance specialCharacterAppearance = settings.specialCharacterAppearance();
    loadComboValueOrDefault(ui.cbWebViewFontStandard,  WVAppearance.font_family_standard,    "Arial");
    loadComboValueOrDefault(ui.cbWebViewFontSerif,     WVAppearance.font_family_serif,       "Times New Roman");
    loadComboValueOrDefault(ui.cbWebViewFontSansSerif, WVAppearance.font_family_sans_serif,  "Arial");
    loadComboValueOrDefault(ui.cbSpecialCharacterFont,  specialCharacterAppearance.font_family,     "Helvetica");
    ui.webViewFontSizeSpin->setValue(WVAppearance.font_size);
    ui.specialCharacterFontSizeSpin->setValue(specialCharacterAppearance.font_size);
    ui.iconSizeSlider->setValue(int(settings.mainMenuIconSize()*10));
    // Don't try to set the index of a disabled combobox
    if (m_isHighDPIComboEnabled) {
        m_HighDPI = settings.highDPI();
        ui.comboHighDPI->setCurrentIndex(m_HighDPI);
    }
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
