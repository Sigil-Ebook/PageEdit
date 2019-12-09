/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
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

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

#include "SettingsStore.h"
#include "Utility.h"
#include "GumboInterface.h"
#include "SelectId.h"

static QString SETTINGS_GROUP = "select_id";

SelectId::SelectId(QString id, QString &source, QWidget *parent)
    :
    QDialog(parent),
    m_SelectedText(id),
    m_CurrentSource(source)
{
    ui.setupUi(this);

    QCompleter *qc = ui.id->completer();
    qc->setCaseSensitivity(Qt::CaseSensitive);

    connectSignalsSlots();
    ReadSettings();
    SetList();
}

void SelectId::SetList()
{
    QStringList ids;
    {
        GumboInterface gi(m_CurrentSource, "any_version");
	QList<GumboNode*> nodes = gi.get_all_nodes_with_attribute(QString("id"));
	nodes.append(gi.get_all_nodes_with_attribute(QString("name")));
	foreach(GumboNode * node, nodes) {
	    QString element_name = QString::fromStdString(gi.get_tag_name(node));
	    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
	    if (attr) {
		ids.append(QString::fromUtf8(attr->value));
	    } else {
		// This is supporting legacy html of <a name="xxx"> (deprecated).
		// Make sure we don't return names of other elements like <meta> tags.
		if (element_name == "a") {
		    attr = gumbo_get_attribute(&node->v.element.attributes, "name");
		    if (attr) {
			ids.append(QString::fromUtf8(attr->value));
		    }
		}
	    }
	}
    }
    foreach(QString id, ids) {
        ui.id->addItem(id);
    }
    // Set default id name
    ui.id->setEditText(m_SelectedText);
}


QString SelectId::GetId()
{
    return m_SelectedText;
}

void SelectId::SetSelectedText()
{
    m_SelectedText = ui.id->currentText();
}

void SelectId::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void SelectId::WriteSettings()
{
    SetSelectedText();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SelectId::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
