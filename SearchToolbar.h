/************************************************************************
 **
 **  Copyright (C) 2019-2024 Kevin B. Hendricks, Stratford Ontario Canada
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
* QupZilla - modified version of searchtoolbar.h
* Copyright (C) 2010-2017 David Rosca <nowrep@gmail.com>
* GPL 3
* ============================================================= */

#ifndef SEARCHTOOLBAR_H
#define SEARCHTOOLBAR_H

#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEnginePage>

namespace Ui
{
class SearchToolbar;
}

class QLineEdit;

class WebViewEdit;

class SearchToolbar : public QWidget
{
    Q_OBJECT
public:
    explicit SearchToolbar(WebViewEdit* view, QWidget* parent = 0);
    ~SearchToolbar();

    QString getNeedle();
    void focusSearchLine();
    bool eventFilter(QObject* obj, QEvent* event);

public Q_SLOTS:
    void setText(const QString &text);
    void searchText(const QString &text);
    void updateFindFlags();
    void caseSensitivityChanged();

    void findNext();
    void findPrevious();

    void close();

private:
    Ui::SearchToolbar* ui;
    WebViewEdit* m_view;

    QWebEnginePage::FindFlags m_findFlags;
};

#endif // SEARCHTOOLBAR_H
