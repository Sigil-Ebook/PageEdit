/************************************************************************
**
**  Copyright (C) 2026 Kevin B. Hendricks, Stratford Ontario Canada
**
**  This file is part of PageEdit.
**
**  PageEdit is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
*************************************************************************/

#pragma once
#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QWidget>
#include <QRegularExpression>
#include <QWebEnginePage>

class QKeyEvent;

#include "SearchOperations.h"

namespace Ui {
class FindReplace;
}

class MainWindow;
class WebViewEdit;

class FindReplace : public QWidget
{
    Q_OBJECT

public:
    explicit FindReplace(MainWindow *main_window, WebViewEdit *view, QWidget *parent = nullptr);
    ~FindReplace();

    void focusFindField();
    void SetUpFindText();

    enum LookWhere {
        LookWhere_CurrentFile = 0,
        LookWhere_AllHTMLFiles = 10
    };

    enum SearchDirection {
        SearchDirection_Down = 0,
        SearchDirection_Up = 10
    };

public slots:
    void show();
    void close();

    void Find();
    void FindNext();
    void FindPrevious();
    void FindNextInFile();
    void FindPreviousInFile();

    void ReplaceCurrent();
    void Replace();
    void ReplacePrevious();
    void ReplaceNextInFile();
    void ReplaceAll();
    void ReplaceAllInFile();
    void Count();
    void CountInFile();

private slots:
    void FindClicked();
    void ReplaceFindClicked();
    void ReplaceCurrentClicked();
    void ReplaceAllClicked();
    void CountClicked();
    void SearchModeChanged();
    void LookWhereChanged();
    void DirectionChanged();
    void WrapChanged();
    void UpdateFindFlags();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    QString GetFindText() const;
    QString GetReplaceText() const;
    QRegularExpression GetSearchRegex() const;
    LookWhere GetLookWhere() const;
    SearchDirection GetSearchDirection() const;
    bool GetWrap() const;
    bool IsCurrentFileScope() const;
    QStringList GetTargetFilePaths() const;

    void ReadSettings();
    void WriteSettings();
    void UpdatePreviousFindStrings();
    void UpdatePreviousReplaceStrings();
    void ClearMessage();
    void ShowMessage(const QString &message);
    void ShowNotFound();
    bool IsValidFindText() const;

    bool FindText(bool search_backward, bool force_current_file = false);
    bool FindInCurrentFileSource(bool search_backward);
    bool FindInAllFiles(bool search_backward);
    bool FileContainsMatch(const QString &relative_path, const QRegularExpression &search_regex) const;

    int ReplaceText(bool search_backward, bool replace_current_only);
    int ReplaceAllInScope(bool force_current_file = false);
    int CountInScope(bool force_current_file = false);

    void HighlightMatch(const QString &matched_text);
    void ScrollToMatchOffset(int offset, const QString &source);
    void ResetSearchOffset();

    Ui::FindReplace *ui;
    MainWindow *m_main_window;
    WebViewEdit *m_view;

    QWebEnginePage::FindFlags m_findFlags;
    int m_lastSearchOffset;
    QString m_lastSearchFile;
    bool m_forceCurrentFile;
};

#endif // FINDREPLACE_H
