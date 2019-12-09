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

#pragma once
#ifndef SELECTHYPERLINK_H
#define SELECTHYPERLINK_H

#include <QtWidgets/QDialog>
#include <QtGui/QStandardItemModel>

#include "ui_SelectHyperlink.h"

class SelectHyperlink: public QDialog
{
    Q_OBJECT

public:

    struct TargetInfo {
        QString element;
        QString filename;
        QString id;
        QString text;
    };

    SelectHyperlink(QString href, QString &source, QString &currentpath, QString &base, 
                    QStringList &other_files, QWidget *parent = 0);

    void SetList();

    QList<SelectHyperlink::TargetInfo> CollectTargets(const QString& source, const QString& filepath);

    QString GetTarget();

private slots:
    void WriteSettings();

    void FilterEditTextChangedSlot(const QString &text);

    void DoubleClicked(const QModelIndex &);
    void Clicked(const QModelIndex &);

private:
    void SetSelectedText();

    void ReadSettings();
    void connectSignalsSlots();

    void AddEntry(const QList<SelectHyperlink::TargetInfo> &targets);

    QString GetSelectedText();

    void SelectText(QString &text);

    QString m_DefaultTarget;
    QString m_Source;
    QString m_CurrentFile;
    QString m_Base;
    QStringList m_OtherFiles;
    QString m_SavedTarget;
    QStandardItemModel *m_SelectHyperlinkModel;

    Ui::SelectHyperlink ui;
};

#endif // SELECTHYPERLINK_H
