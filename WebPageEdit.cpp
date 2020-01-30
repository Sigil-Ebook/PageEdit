/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2020      Doug Massay
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
#include <QTimer>
#include <QtWebEngineWidgets/QWebEnginePage>
#include <QDebug>
#include "Utility.h"
#include "WebPageEdit.h"

#define DBG if(0)

static const QString BASIC_HTML =
    "<html><head><title></title></head>"
    "<body style=\"background-color: %1\"></body></html>";
 
WebPageEdit::WebPageEdit(QObject *parent)
    : QWebEnginePage(parent)
{

    setBackgroundColor(Utility::WebViewBackgroundColor(true));

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // Qt 5.14 seems to have broken setBackgroundColor completely
    // Linux is the only one there now but this may be needed for all platforms
    // in the future unless Qt is fixed
    setHtml(BASIC_HTML.arg(backgroundColor().name()));
#else
    setUrl(QUrl("about:blank"));
#endif
}

// Because you can not delegate all links in QtWebEngine we must override 
// here and generate our own link requests

// BUT a loadStarted signal is emitted by this page **before** this is called
// Even **before** it knows how we want to handle it!
// Once we "return false" from this a loadFinished with okay **false** is generated.

// These false loadStart and LoadFinished greatly confuse our model

// Therefore do NOT emit a signal from this method as it can create huge delays in when
// loadFinished(okay) returns (with okay as false) 

bool WebPageEdit::acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if ((type == QWebEnginePage::NavigationTypeLinkClicked) || (type == QWebEnginePage::NavigationTypeOther)) {
        DBG qDebug() << "acceptNavigationRequest " << url.toString() << " , " << type << " , " << isMainFrame;
        m_url = url;
	QTimer::singleShot(20,this,SLOT(EmitLinkClicked()));
        return false;
    }
    return true;
}

void WebPageEdit::EmitLinkClicked()
{
    emit LinkClicked(m_url);
}

void WebPageEdit::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, 
				  const QString & message, int lineNumber, const QString & sourceID)
{
  const QString logEntry = message + " on line: " + QString::number(lineNumber) + " Source:" + sourceID;
  qDebug() << "Javascript error: " << level << logEntry;
}

