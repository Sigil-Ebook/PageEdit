/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef APPEVENTFILTER_H
#define APPEVENTFILTER_H

#include <QObject>
#include <QString>

class QEvent;

class AppEventFilter : public QObject
{
    Q_OBJECT

public:

    // Constructor;
    // The argument is the object's parent.
    AppEventFilter(QObject *parent);

    QString getInitialFilePath();
    void    setInitialFilePath(const QString &filepath);

protected:

    // The event filter used to catch OS X's
    // QFileOpenEvents. These signal the user used the OS's
    // services to start PageEdit with an existing document
    bool eventFilter(QObject *watched_object, QEvent *event);

private:
    // any initial file passed in from OS when PageEdit first launched
    QString m_initialFilePath;

};

#endif // APPEVENTFILTER_H


