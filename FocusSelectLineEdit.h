/************************************************************************
 **
 **  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
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

/* ============================================================
* QupZilla - modified version of focusselectlineedit.h
* Copyright (C) 2010-2014  David Rosca <nowrep@gmail.com>
* GPL 3
* ============================================================ */

#ifndef FOCUSSELECTLINEEDIT_H
#define FOCUSSELECTLINEEDIT_H

#include <QLineEdit>

class FocusSelectLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FocusSelectLineEdit(QWidget* parent = 0);

public slots:
    void setFocus();

protected:
    void focusInEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);

    bool m_mouseFocusReason;

};

#endif // FOCUSSELECTLINEEDIT_H
