
/****************************************************************************
**
**  Copyright (C) 2021  Doug Massay
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
****************************************************************************/


#pragma once
#ifndef WEBVIEWPRINTER_H
#define WEBVIEWPRINTER_H

#include <QObject>
#include <QUrl>

class QPrinter;
class QWebEngineView;

class WebViewPrinter: public QObject
{
    Q_OBJECT

public:
    WebViewPrinter(QObject *parent = nullptr);
    ~WebViewPrinter();

    void setPage(QUrl url);

private slots:
    void printPreview();
    void printDocument(QPrinter *printer);
    void loadFinished(bool ok);

private:
    QWebEngineView *m_view = nullptr;
    bool m_inPrintPreview = false;
};

#endif // WEBVIEWPRINTER_H