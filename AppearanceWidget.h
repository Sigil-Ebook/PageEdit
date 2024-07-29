/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2016-2024  Doug Massay
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Grant Drake
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
#ifndef APPEARANCEWIDGET_H
#define APPEARANCEWIDGET_H

#include "MainWindow.h"
#include "SettingsStore.h"
#include "PreferencesWidget.h"
#include "ui_PAppearanceWidget.h"

/**
 * Preferences widget for font/appearance related preferences.
 */
class AppearanceWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    AppearanceWidget();
    void readSettings();
    PreferencesWidget::ResultActions saveSettings();

private slots:
    void changeUIFontButtonClicked();
    void resetAllButtonClicked();
    void newSliderValue(int value);

private:
    void updateUIFontDisplay();
    void loadComboValueOrDefault(QFontComboBox *fontComboBox, const QString &value, const QString &defaultValue);
    void connectSignalsToSlots();

    Ui::AppearanceWidget ui;
    int m_PreviewDark;
    QString m_initUIFont;
    QString m_currentUIFont;
    bool m_uiFontResetFlag;
    bool m_UseCustomDarkTheme;
};

#endif // APPEARANCEWIDGET_H
