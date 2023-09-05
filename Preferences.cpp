/************************************************************************
**
**  Copyright (C) 2019-2023 Kevin B, Hendricks, Stratford Ontario Canada
**  Copyright (C) 2011      John Schember <john@nachtimwald.com>
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

#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QMessageBox>
#include <QScrollArea>
#include <QWidget>

#include "Preferences.h"
#include "SettingsStore.h"
#include "Utility.h"
#include "PreferencesWidget.h"
#include "AppearanceWidget.h"
#include "GeneralSettings.h"

static const QString SETTINGS_GROUP = "preferences_dialog";

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    m_restartPageEdit(false),
    m_reloadPreview(false)
{
    ui.setupUi(this);
    extendUI();
    // Create and load all of our preference widgets.;
    appendPreferenceWidget(new AppearanceWidget);
    appendPreferenceWidget(new GeneralSettings);
    connectSignalsSlots();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    readSettings();
    QApplication::restoreOverrideCursor();
}

void Preferences::selectPWidget(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous)
    int index = ui.availableWidgets->row(current);
    ui.pWidget->setCurrentIndex(index);
}

void Preferences::saveSettings()
{
    PreferencesWidget::ResultActions widgetResult;
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastpreference", ui.availableWidgets->currentRow());

    for (int i = 0; i < ui.pWidget->count(); ++i) {
        PreferencesWidget *pw = qobject_cast<PreferencesWidget *>(ui.pWidget->widget(i));

        if (pw != 0) {
            widgetResult = pw->saveSettings();

            if (widgetResult & PreferencesWidget::ResultAction_RestartPageEdit) {
                m_restartPageEdit = true;
            }
            if (widgetResult & PreferencesWidget::ResultAction_ReloadPreview) {
                m_reloadPreview = true;
	    }
        }
    }

    QApplication::restoreOverrideCursor();
    settings.endGroup();

    if (m_restartPageEdit) {
        Utility::warning(this, tr("PageEdit"), tr("Changes will take effect when you restart PageEdit."));
    }
}


void Preferences::makeActive(int index)
{
    if ((index > -1) || (index <  ui.availableWidgets->count())) {
        ui.availableWidgets->setCurrentRow(index);
    }
}


void Preferences::readSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Ensure the previous item selected in the available preferences widgets list
    // is highlighted.
    int last_preference_index = settings.value("lastpreference", 0).toInt();

    if (last_preference_index > ui.availableWidgets->count() - 1) {
        last_preference_index = 0;
    }

    ui.availableWidgets->setCurrentRow(last_preference_index);
    settings.endGroup();
}

void Preferences::appendPreferenceWidget(PreferencesWidget *widget)
{
    // Add the PreferencesWidget to the stack view area.
    ui.pWidget->addWidget(widget);
    // Add an entry to the list of available preference widgets.
    ui.availableWidgets->addItem(widget->windowTitle());
}

bool Preferences::isRestartRequired()
{
    return m_restartPageEdit;
}

bool Preferences::isReloadPreviewRequired()
{
    return m_reloadPreview;
}

void Preferences::openPreferencesLocation()
{
    QUrl location = QUrl::fromLocalFile(Utility::DefinePrefsDir());
    QDesktopServices::openUrl(location);
}

void Preferences::extendUI()
{
    QPushButton *open_button = ui.buttonBox->button(QDialogButtonBox::Reset);
    open_button->setText(tr("Open Preferences Location"));
    open_button->setToolTip(QDir::toNativeSeparators(Utility::DefinePrefsDir()));
}

void Preferences::connectSignalsSlots()
{
    connect(ui.availableWidgets, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(selectPWidget(QListWidgetItem *, QListWidgetItem *)));
    connect(this, SIGNAL(finished(int)), this, SLOT(saveSettings()));
    connect(ui.buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(openPreferencesLocation()));
}
