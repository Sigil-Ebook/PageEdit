
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
#include <QFileInfo>

class QPrinter;
class QWebEngineView;

class WebViewPrinter: public QObject
{
    Q_OBJECT

public:
    WebViewPrinter(QObject *parent = nullptr);
    ~WebViewPrinter();

    /**
    ** Initialize new vanilla QWebEngineView and
    ** and load the page to printed url into it
    **/
    void setPage(QUrl url, bool skipPrev);

private slots:
    /**
    ** Called when QWebEngineView::loadFinished signal fires
    **/
    void loadFinished(bool ok);

private:
    /**
    ** set default print-to-file path on linux to keep CUPS dialog from defaulting
    ** to (and showing) the ebook's location in PageEdit's temporary scratch area
    **/
    QString getPrintToFilePath(QFileInfo &fi);

    /**
    ** Go directly to Printer Dialog
    **/
    void print();

    /**
    ** Launch a QPrintPreview Dialog before printing
    **/
    void printPreview();

    /**
    ** QWebEnginePage::print statement and callback
    **/
    void printDocument(QPrinter *printer);

    /**
    ** New QwebEngineView that loads the page url
    ** without any PageEdit dark mode injections
    **/
    QWebEngineView *m_view = nullptr;

    bool m_inPrintPreview = false;

    /**
    ** holds preference setting to bypass Print Preview
    **/
    bool m_skipPreview = false;
};

#endif // WEBVIEWPRINTER_H