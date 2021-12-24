/************************************************************************
 **
 **  Copyright (C) 2019-2021 Kevin B. Hendricks, Stratford Ontario Canada
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
* QupZilla - Modified version of searchtoolbar.cpp
* Copyright (C) 2010-2018 David Rosca <nowrep@gmail.com>
* GPL 3
* ============================================================ */

#include <QApplication>
#include <QHBoxLayout>
#include <QStyle>
#include <QIcon>
#include <QKeyEvent>
#include <QShortcut>
#include <QPointer>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineScript>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QWebEngineFindTextResult>
#endif


#include "SearchToolbar.h"
#include "WebViewEdit.h"
#include "WebPageEdit.h"
#include "FocusSelectLineEdit.h"
#include "ui_SearchToolbar.h"


SearchToolbar::SearchToolbar(WebViewEdit* view, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SearchToolbar)
    , m_view(view)
    , m_findFlags(QWebEnginePage::FindFlags())
{
    ui->setupUi(this);

    ui->closeButton->setIcon(QIcon(QString::fromUtf8(":/icons/widget-close.svg")));

    ui->next->setIcon(QIcon(QString::fromUtf8(":/icons/find-next.svg")));
    ui->next->setShortcut(QKeySequence("Ctrl+G"));

    ui->previous->setIcon(QIcon(QString::fromUtf8(":/icons/find-previous.svg")));
    ui->previous->setShortcut(QKeySequence("Ctrl+Shift+G"));

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->lineEdit, SIGNAL(textEdited(QString)), this, SLOT(findNext()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(findNext()));
    connect(ui->next, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(ui->previous, SIGNAL(clicked()), this, SLOT(findPrevious()));
    connect(ui->caseSensitive, SIGNAL(clicked()), this, SLOT(caseSensitivityChanged()));

    QShortcut* findNextAction = new QShortcut(QKeySequence("F3"), this);
    connect(findNextAction, SIGNAL(activated()), this, SLOT(findNext()));

    QShortcut* findPreviousAction = new QShortcut(QKeySequence("Shift+F3"), this);
    connect(findPreviousAction, SIGNAL(activated()), this, SLOT(findPrevious()));

}

void SearchToolbar::focusSearchLine()
{
    ui->lineEdit->setFocus();
}

void SearchToolbar::close()
{
    hide();
    searchText(QString());
    ui->lineEdit->setText(QString());
    m_view->setFocus();
}

void SearchToolbar::findNext()
{
    m_findFlags = QWebEnginePage::FindFlags();
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolbar::findPrevious()
{
    m_findFlags = QWebEnginePage::FindBackward;
    updateFindFlags();

    searchText(ui->lineEdit->text());
}

void SearchToolbar::updateFindFlags()
{
    if (ui->caseSensitive->isChecked()) {
        m_findFlags = m_findFlags | QWebEnginePage::FindCaseSensitively;
    }
    else {
        m_findFlags = m_findFlags & ~QWebEnginePage::FindCaseSensitively;
    }
}

void SearchToolbar::caseSensitivityChanged()
{
    updateFindFlags();

    searchText(QString());
    searchText(ui->lineEdit->text());
}

void SearchToolbar::setText(const QString &text)
{
    ui->lineEdit->setText(text);
}


void SearchToolbar::searchText(const QString &text)
{
    QPointer<SearchToolbar> guard = this;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_view->findText(text, m_findFlags, [=](bool found) {
#else
    m_view->findText(text, m_findFlags, [=](const QWebEngineFindTextResult& result) {
    bool found = result.numberOfMatches() > 0;
#endif
        if (!guard) {
            return;
        }
        if (ui->lineEdit->text().isEmpty())
            found = true;

        if (!found)
            ui->results->setText(tr("No results found."));
        else
            ui->results->clear();

        ui->lineEdit->setProperty("notfound", QVariant(!found));
        ui->lineEdit->style()->unpolish(ui->lineEdit);
        ui->lineEdit->style()->polish(ui->lineEdit);

        // Clear selection
        m_view->page()->runJavaScript("window.getSelection().empty();", QWebEngineScript::ApplicationWorld);
    });
}

bool SearchToolbar::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj);

    if (event->type() == QEvent::KeyPress && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape) {
        close();
    }

    return false;
}

SearchToolbar::~SearchToolbar()
{
    delete ui;
}
