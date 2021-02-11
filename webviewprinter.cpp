
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
#include <QEventLoop>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QFileInfo>
#include <QDir>
#include <QPrintPreviewDialog>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QStandardPaths>
#include <QDebug>

WebViewPrinter::WebViewPrinter(QObject *parent)
    : QObject(parent),
      m_view(new QWebEngineView()),
      m_page(new QWebEnginePage(this))
{

}

void WebViewPrinter::setPage(QUrl url)
{
    Q_ASSERT(!m_page);
    m_page->setUrl(url);
    //m_view->setAttribute(Qt::WA_DontShowOnScreen);
    m_view->setPage(m_page);
    printPreview();
    //connect(m_page, &QWebEnginePage::loadFinished, this, &WebViewPrinter::printPreview);
}

void WebViewPrinter::print()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, m_view);
    if (dialog.exec() != QDialog::Accepted)
        return;
    printDocument(&printer);
}

void WebViewPrinter::printDocument(QPrinter *printer)
{
    QEventLoop loop;
    bool result;
    auto printPreview = [&](bool success) { result = success; loop.quit(); };
    m_page->print(printer, std::move(printPreview));
    loop.exec();
    if (!result) {
        QPainter painter;
        if (painter.begin(printer)) {
            QFont font = painter.font();
            font.setPixelSize(20);
            painter.setFont(font);
            painter.drawText(QPointF(10,25),
                             QStringLiteral("Could not generate print preview."));

            painter.end();
        }
    }
}

void WebViewPrinter::printPreview()
{
    if (!m_page)
        return;
    if (m_inPrintPreview)
        return;
    m_inPrintPreview = true;
    QFileInfo fi = QFileInfo(m_page->url().fileName());
    QString filename = fi.baseName() + ".pdf";
    QString path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath(filename);
    QPrinter printer;
    printer.setOutputFileName(path);
    printer.setOutputFormat(QPrinter::NativeFormat);
    QPrintPreviewDialog preview(&printer, m_page->view());
    connect(&preview, &QPrintPreviewDialog::paintRequested,
            this, &WebViewPrinter::printDocument);
    preview.exec();
    m_inPrintPreview = false;
}
