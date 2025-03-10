/****************************************************************************
**
**  Copyright (C) 2021-2024  Doug Massay
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


#include "webviewprinter.h"
#include "SettingsStore.h"
#include <QEventLoop>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QPrintPreviewDialog>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QStandardPaths>
#include <QDebug>

#define DBG if(0)

WebViewPrinter::WebViewPrinter(QObject *parent)
    : QObject(parent)
{

}

WebViewPrinter::~WebViewPrinter()
{
    if (m_view){
        delete m_view;
        m_view = nullptr;
        m_skipPreview = false;
    }
    DBG qDebug() << "WebViewPrinter destroyed";
}

void WebViewPrinter::setPage(QUrl url, bool skipPrev)
{
    // destroy previous QWebEngineView to avoid leaking
    if (m_view){
        delete m_view;
        m_view = nullptr;
        DBG qDebug() << "Cleaning up old WebEngineView";
    }
    m_skipPreview = skipPrev;
    m_inPrintPreview = false;
    DBG qDebug() << "URL of page to print: " << url;

    // Create new (undisplayed) QWebEngineView to which the EPUB's
    // page url is loaded without any potential darkmode injections
    m_view = new QWebEngineView();
    connect(m_view, &QWebEngineView::loadFinished, this, &WebViewPrinter::loadFinished);
    m_view->setUrl(url);
}

QString WebViewPrinter::getPrintToFilePath(QFileInfo &fi) {
    QString filename = fi.baseName() + ".pdf";
    QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(filename);
    return path;
}

void WebViewPrinter::print()
{
    DBG qDebug() << "Skipping Print Preview.";
    SettingsStore ss;
    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(ss.printDPI());
    DBG qDebug() << "Print DPI = " << printer.resolution();
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QFileInfo fi = QFileInfo(m_view->page()->url().fileName());
    QString path = getPrintToFilePath(fi);
    DBG qDebug() << "Default Print to PDF file name on Linux: " << path;
    printer.setOutputFileName(path);
#endif
    //m_view->setAttribute(Qt::WA_DontShowOnScreen);
    //m_view->show();
    QPrintDialog dialog(&printer);
    if (dialog.exec() != QDialog::Accepted)
        return;
    printDocument(&printer);

}

void WebViewPrinter::printDocument(QPrinter *printer)
{
    QEventLoop loop;
    bool result;
    auto printCallback = [&](bool success) { result = success; loop.quit(); };
    connect(m_view, &QWebEngineView::printFinished, std::move(printCallback)); 
    m_view->print(printer);
    loop.exec();
    if (!result) {
        DBG qDebug() << "Could not print document.";
        QPainter painter;
        if (painter.begin(printer)) {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(QPointF(10,25),
                             QStringLiteral("Could not print document."));

            painter.end();
        }
    }
}

void WebViewPrinter::printPreview()
{
    DBG qDebug() << "Launching Print Preview.";
    if (!m_view->page())
        return;
    if (m_inPrintPreview)
        return;
    m_inPrintPreview = true;

    SettingsStore ss;
    QPrinter printer(QPrinter::HighResolution);
    printer.setResolution(ss.printDPI());
    DBG qDebug() << "Print Preview DPI = " << printer.resolution();
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QFileInfo fi = QFileInfo(m_view->page()->url().fileName());
    QString path = getPrintToFilePath(fi);
    DBG qDebug() << "Default Print to PDF file name on Linux: " << path;
    printer.setOutputFileName(path);
#endif
    QPrintPreviewDialog preview(&printer, m_view);
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &WebViewPrinter::printDocument);
    preview.exec();
    m_inPrintPreview = false;
}

void WebViewPrinter::loadFinished(bool ok)
{
    if (ok) {
        // load QPrintPreview dialog or go directly to Print dialog
        if (!m_skipPreview) {
            printPreview();
        } else {
            print();
        }
    }
    else {
       DBG  qDebug() << "not successfully loaded.";
    }
}
