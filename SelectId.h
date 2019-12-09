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
#ifndef SELECTID_H
#define SELECTID_H

#include <QtWidgets/QDialog>

#include "ui_SelectId.h"

class SelectId: public QDialog
{
    Q_OBJECT

public:
    SelectId(QString id, QString &source, QWidget *parent = 0);

    void SetList();

    QString GetId();

private slots:
    void WriteSettings();

private:
    void SetSelectedText();

    void ReadSettings();
    void connectSignalsSlots();

    QString m_SelectedText;

    QString m_CurrentSource;

    Ui::SelectId ui;
};

#endif // SELECTID_H
