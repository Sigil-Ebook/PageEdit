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

#include "SettingsStore.h"
#include "GumboInterface.h"
#include "Utility.h"
#include "SelectHyperlink.h"
#include "pageedit_constants.h"

static QString SETTINGS_GROUP = "select_hyperlink";

SelectHyperlink::SelectHyperlink(QString default_href, QString &source, QString &currentpath,
                                 QString &base, QStringList &other_files, QWidget *parent)
    :
    QDialog(parent),
    m_DefaultTarget(default_href),
    m_Source(source),
    m_CurrentFile(currentpath),
    m_Base(base),
    m_OtherFiles(other_files),
    m_SavedTarget(QString()),
    m_SelectHyperlinkModel(new QStandardItemModel)
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    SetList();
    // Set default href name
    ui.href->setText(m_DefaultTarget);

    if (!m_DefaultTarget.isEmpty()) {
        SelectText(m_DefaultTarget);
    } else {
        SelectText(m_SavedTarget);
    }

    // Default entry on the target url not the filter
    ui.href->setFocus();
}

QList<SelectHyperlink::TargetInfo> SelectHyperlink::CollectTargets(const QString& source, const QString& filepath) 
{
    QList<SelectHyperlink::TargetInfo> targets;
    GumboInterface gi(source, "any_version");
    QList<GumboNode*> nodes = gi.get_all_nodes_with_attribute(QString("id"));
    nodes.append(gi.get_all_nodes_with_attribute(QString("name")));
    foreach(GumboNode * node, nodes) {
	SelectHyperlink::TargetInfo atarget;
        atarget.element = QString::fromStdString(gi.get_tag_name(node));
        atarget.filename = filepath;
        atarget.text = gi.get_local_text_of_node(node);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr) {
            atarget.id = QString::fromUtf8(attr->value);
        } else {
            // This is supporting legacy html of <a name="xxx"> (deprecated).
            // Make sure we don't return names of other elements like <meta> tags.
            if (atarget.element == "a") {
                attr = gumbo_get_attribute(&node->v.element.attributes, "name");
                if (attr) {
                    atarget.id = QString::fromUtf8(attr->value);
                }
            }
        }
        targets << atarget;
    }
    return targets;
}

void SelectHyperlink::SetList()
{
    m_SelectHyperlinkModel->clear();
    QStringList header;
    header.append(tr("Targets in the Book"));
    header.append(tr("Text"));
    m_SelectHyperlinkModel->setHorizontalHeaderLabels(header);
    ui.list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.list->setModel(m_SelectHyperlinkModel);

    // first get the list of valid targets in the current source file
    QList<SelectHyperlink::TargetInfo> targets = CollectTargets(m_Source, m_CurrentFile);
    AddEntry(targets);
    targets.clear();

    // then build the list of targets for the other source files
    foreach(QString apath, m_OtherFiles) {
        QString fullpath = m_Base + apath;
        QString source;
	try {
	    source = Utility::ReadUnicodeTextFile(fullpath);
	} catch (std::exception &e) {
	    source = "<html><head><title></title></head><body></body></html>";
	}
	targets = CollectTargets(source, apath);
	AddEntry(targets);
        targets.clear();
    }
    ui.list->resizeColumnToContents(0);
    ui.list->resizeColumnToContents(1);
}

void SelectHyperlink::AddEntry(const QList<SelectHyperlink::TargetInfo> &targets)
{
    foreach(SelectHyperlink::TargetInfo tinfo, targets) {
        QString target;
        QString id = tinfo.id;
        QString fragment;
        if (!id.isEmpty()) {
            fragment = "#" + id;
	}
        if (m_CurrentFile == tinfo.filename) {
            // link is local to this file
            target = fragment;
            if (target.isEmpty()) target = "#";
	} else {
	    target = tinfo.filename + fragment;
	}
        // abbreviate any local text    
        QString text = tinfo.text;
        text = text.left(50);
        QString filepath = m_Base + tinfo.filename + fragment;
        QList<QStandardItem *> rowItems;
        QStandardItem *target_item = new QStandardItem();
        target_item->setText(target);
        target_item->setData(filepath);
        rowItems << target_item;
        target_item = new QStandardItem();
        target_item->setText(text);
        rowItems << target_item;
        m_SelectHyperlinkModel->appendRow(rowItems);
        rowItems[0]->setEditable(false);
        rowItems[1]->setEditable(false);
    }
}

QString SelectHyperlink::GetSelectedText()
{
    QString text;

    if (ui.list->selectionModel()->hasSelection()) {
        QModelIndexList selected_indexes = ui.list->selectionModel()->selectedRows(0);

        if (!selected_indexes.isEmpty()) {
            QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(selected_indexes.last());

            if (item) {
                text = item->text();
            }
        }
    }

    return text;
}

void SelectHyperlink::SelectText(QString &text)
{
    if (!text.isEmpty()) {
        QModelIndex parent_index;
        QStandardItem *root_item = m_SelectHyperlinkModel->invisibleRootItem();
        // Convert search text to filename#fragment
        QString target = text;

        if (target.startsWith("#") && !m_CurrentFile.isEmpty()) {
            target = m_CurrentFile + text;
        }

        if (target.contains("/")) {
            target = target.right(target.length() - target.lastIndexOf("/") - 1);
        }

        for (int row = 0; row < root_item->rowCount(); row++) {
            QStandardItem *child = root_item->child(row, 0);
            // Convert selection text to filename#fragment
            QString selection = child->data().toString();

            if (selection.contains("/")) {
                selection = selection.right(selection.length() - selection.lastIndexOf("/") - 1);
            }

            if (target == selection) {
                ui.list->selectionModel()->select(m_SelectHyperlinkModel->index(row, 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                ui.list->setFocus();
                ui.list->setCurrentIndex(child->index());
            }
        }
    }
}

void SelectHyperlink::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_SelectHyperlinkModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.list->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.list->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.list->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.list->setCurrentIndex(QModelIndex());
    }
}

QString SelectHyperlink::GetTarget()
{
    QString target;
    target = ui.href->text();
    return target;
}

void SelectHyperlink::DoubleClicked(const QModelIndex &index)
{
    Clicked(index);
    accept();
}

void SelectHyperlink::Clicked(const QModelIndex &index)
{
    QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(index);

    if (item->text().startsWith("#")) {
        ui.href->setText(m_CurrentFile + item->text());
    } else {
        ui.href->setText(item->text());
       // ui.href->setText(item->data().toString());
    }
}

void SelectHyperlink::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    m_SavedTarget = settings.value("lastselectedentry").toString();
    settings.endGroup();
}

void SelectHyperlink::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastselectedentry", GetSelectedText());
    settings.endGroup();
}

void SelectHyperlink::connectSignalsSlots()
{
    connect(this,      SIGNAL(accepted()),           this, SLOT(WriteSettings()));
    connect(ui.filter, SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));

    connect(ui.list,   SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(DoubleClicked(const QModelIndex &)));
    connect(ui.list,   SIGNAL(clicked(const QModelIndex &)),       this, SLOT(Clicked(const QModelIndex &)));
}
