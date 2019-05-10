/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Grant Drake
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
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QListWidget>
#include <QtGui/QPainter>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWebEngineWidgets/QWebEngineSettings>

#include "AppearanceWidget.h"
#include "SettingsStore.h"
#include "Utility.h"

AppearanceWidget::AppearanceWidget()
{
    ui.setupUi(this);
    readSettings();
    connectSignalsToSlots();
}

PreferencesWidget::ResultAction AppearanceWidget::saveSettings()
{
    SettingsStore settings;
    settings.setAppearancePrefsTabIndex(ui.tabAppearance->currentIndex());

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
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize,   WVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont,    WVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont,       WVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont,   WVAppearance.font_family_sans_serif);

    return PreferencesWidget::ResultAction_None;
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
    // Read and apply the settings without changing our m_codeViewAppearance
    // instance holding the last persisted values.
    readSettings();
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
}
