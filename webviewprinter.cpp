
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


#include "webviewprinter.h"
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDebug>

WebViewPrinter::WebViewPrinter(QObject *parent)
    : QObject(parent)
    , m_page(new QWebEnginePage(this))
    , m_view(new QWebEngineView())
{

}

WebViewPrinter::~WebViewPrinter()
{
    if (m_page != NULL) {
        delete m_page;
    }
    if (m_view != NULL) {
        delete m_view;
    }
}

void WebViewPrinter::setPage(QUrl url)
{
    m_url = url;
    connect(m_view, &QWebEngineView::loadFinished, this, &WebViewPrinter::loadComplete);
    connect(m_page, &QWebEnginePage::pdfPrintingFinished, this, &WebViewPrinter::pdfComplete);
    qDebug() << "setPage " << url;

}

void WebViewPrinter::run()
{
    qDebug() << "run";
    m_page->setUrl(m_url);
    //m_view->setAttribute(Qt::WA_DontShowOnScreen);
    m_view->setPage(m_page);
    //m_view->show();
}

void WebViewPrinter::printPage()
{
    qDebug() << "printToPdf";
    qDebug() << m_page->url();
    QFileInfo fi = QFileInfo(m_page->url().fileName());
    qDebug() << fi.fileName();
    QString filename = fi.baseName() + ".pdf";
    QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)).filePath(filename);
    qDebug() << path;
    m_page->printToPdf(path);
}

void WebViewPrinter::loadComplete(bool ok)
{
    if (!ok) {
        qDebug() << "loadComplete: failed to load page";
    }
    qDebug() << "loadComplete: page loaded ok";
    WebViewPrinter::printPage();
}

void WebViewPrinter::pdfComplete(const QString &filePath, bool success)
{
    qDebug() << "pdfPrintingFinished";
    if (!success) {
         qDebug() << "Failed to write pdf";
    }
    qDebug() << filePath;
    QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}
