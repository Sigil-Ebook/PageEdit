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

#include <QApplication>
#include <QComboBox>
#include <QIcon>
#include <QKeyEvent>
#include <QLineEdit>
#include <QShortcut>
#include <QStyle>

#include "FindReplace.h"
#include "MainWindow.h"
#include "SettingsStore.h"
#include "Utility.h"
#include "WebViewEdit.h"
#include "ui_FindReplace.h"

static const QString SETTINGS_GROUP = "find_replace";

FindReplace::FindReplace(MainWindow *main_window, WebViewEdit *view, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindReplace)
    , m_main_window(main_window)
    , m_view(view)
    , m_findFlags(QWebEnginePage::FindFlags())
    , m_lastSearchOffset(-1)
    , m_forceCurrentFile(false)
{
    ui->setupUi(this);

    ui->closeButton->setIcon(QIcon(QString::fromUtf8(":/icons/widget-close.svg")));

    ui->cbSearchMode->addItem(tr("Normal"), SearchOperations::SearchMode_Normal);
    ui->cbSearchMode->addItem(tr("Case Sensitive"), SearchOperations::SearchMode_Case_Sensitive);

    ui->cbLookWhere->addItem(tr("Current File"), LookWhere_CurrentFile);
    ui->cbLookWhere->addItem(tr("All HTML Files"), LookWhere_AllHTMLFiles);

    ui->cbSearchDirection->addItem(tr("Down"), SearchDirection_Down);
    ui->cbSearchDirection->addItem(tr("Up"), SearchDirection_Up);

    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->findNext, SIGNAL(clicked()), this, SLOT(FindClicked()));
    connect(ui->replaceFind, SIGNAL(clicked()), this, SLOT(ReplaceFindClicked()));
    connect(ui->replaceCurrent, SIGNAL(clicked()), this, SLOT(ReplaceCurrentClicked()));
    connect(ui->replaceAll, SIGNAL(clicked()), this, SLOT(ReplaceAllClicked()));
    connect(ui->countAll, SIGNAL(clicked()), this, SLOT(CountClicked()));
    connect(ui->cbSearchMode, SIGNAL(currentIndexChanged(int)), this, SLOT(SearchModeChanged()));
    connect(ui->cbLookWhere, SIGNAL(currentIndexChanged(int)), this, SLOT(LookWhereChanged()));
    connect(ui->cbSearchDirection, SIGNAL(currentIndexChanged(int)), this, SLOT(DirectionChanged()));
    connect(ui->chkWrap, SIGNAL(toggled(bool)), this, SLOT(WrapChanged()));

    QLineEdit *find_edit = ui->cbFind->lineEdit();
    QLineEdit *replace_edit = ui->cbReplace->lineEdit();
    connect(find_edit, SIGNAL(returnPressed()), this, SLOT(FindClicked()));
    connect(replace_edit, SIGNAL(returnPressed()), this, SLOT(ReplaceFindClicked()));

    new QShortcut(QKeySequence("F3"), this, SLOT(FindNext()));
    new QShortcut(QKeySequence("Shift+F3"), this, SLOT(FindPrevious()));

    installEventFilter(this);
    find_edit->installEventFilter(this);
    replace_edit->installEventFilter(this);

    ReadSettings();
    UpdateFindFlags();
}

FindReplace::~FindReplace()
{
    WriteSettings();
    delete ui;
}

void FindReplace::show()
{
    QWidget::show();
    focusFindField();
}

void FindReplace::close()
{
    hide();
    m_view->findText(QString());
    ClearMessage();
    m_view->setFocus();
    WriteSettings();
}

void FindReplace::focusFindField()
{
    ui->cbFind->setFocus();
    if (ui->cbFind->lineEdit()) {
        ui->cbFind->lineEdit()->selectAll();
    }
}

void FindReplace::SetUpFindText()
{
    QString selected = m_view->GetSelectedText();
    if (!selected.isEmpty()) {
        ui->cbFind->setEditText(selected);
    }
    ResetSearchOffset();
}

void FindReplace::Find()
{
    if (GetSearchDirection() == SearchDirection_Up) {
        FindPrevious();
    } else {
        FindNext();
    }
}

void FindReplace::FindNext()
{
    FindText(false, m_forceCurrentFile);
    m_forceCurrentFile = false;
}

void FindReplace::FindPrevious()
{
    FindText(true, m_forceCurrentFile);
    m_forceCurrentFile = false;
}

void FindReplace::FindNextInFile()
{
    m_forceCurrentFile = true;
    FindNext();
}

void FindReplace::FindPreviousInFile()
{
    m_forceCurrentFile = true;
    FindPrevious();
}

void FindReplace::ReplaceCurrent()
{
    ReplaceText(GetSearchDirection() == SearchDirection_Up, true);
}

void FindReplace::Replace()
{
    ReplaceText(GetSearchDirection() == SearchDirection_Up, false);
}

void FindReplace::ReplacePrevious()
{
    ReplaceText(true, false);
}

void FindReplace::ReplaceNextInFile()
{
    m_forceCurrentFile = true;
    Replace();
}

void FindReplace::ReplaceAll()
{
    ReplaceAllInScope(m_forceCurrentFile);
    m_forceCurrentFile = false;
}

void FindReplace::ReplaceAllInFile()
{
    m_forceCurrentFile = true;
    ReplaceAllInScope(true);
    m_forceCurrentFile = false;
}

void FindReplace::Count()
{
    CountInScope(m_forceCurrentFile);
    m_forceCurrentFile = false;
}

void FindReplace::CountInFile()
{
    m_forceCurrentFile = true;
    CountInScope(true);
    m_forceCurrentFile = false;
}

void FindReplace::FindClicked()
{
    Find();
}

void FindReplace::ReplaceFindClicked()
{
    Replace();
}

void FindReplace::ReplaceCurrentClicked()
{
    ReplaceCurrent();
}

void FindReplace::ReplaceAllClicked()
{
    ReplaceAll();
}

void FindReplace::CountClicked()
{
    Count();
}

void FindReplace::SearchModeChanged()
{
    ResetSearchOffset();
    WriteSettings();
}

void FindReplace::LookWhereChanged()
{
    ResetSearchOffset();
    WriteSettings();
}

void FindReplace::DirectionChanged()
{
    WriteSettings();
}

void FindReplace::WrapChanged()
{
    WriteSettings();
}

void FindReplace::UpdateFindFlags()
{
    m_findFlags = QWebEnginePage::FindFlags();

    if (ui->cbSearchMode->currentData().toInt() == SearchOperations::SearchMode_Case_Sensitive) {
        m_findFlags |= QWebEnginePage::FindCaseSensitively;
    }
}

bool FindReplace::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast<QKeyEvent *>(event);

        if (key_event->key() == Qt::Key_Escape) {
            close();
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void FindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }

    QWidget::keyPressEvent(event);
}

QString FindReplace::GetFindText() const
{
    return Utility::UseNFC(ui->cbFind->currentText());
}

QString FindReplace::GetReplaceText() const
{
    return Utility::UseNFC(ui->cbReplace->currentText());
}

QRegularExpression FindReplace::GetSearchRegex() const
{
    const SearchOperations::SearchMode mode =
        static_cast<SearchOperations::SearchMode>(ui->cbSearchMode->currentData().toInt());
    return SearchOperations::BuildSearchRegex(GetFindText(), mode, false);
}

FindReplace::LookWhere FindReplace::GetLookWhere() const
{
    return static_cast<LookWhere>(ui->cbLookWhere->currentData().toInt());
}

FindReplace::SearchDirection FindReplace::GetSearchDirection() const
{
    return static_cast<SearchDirection>(ui->cbSearchDirection->currentData().toInt());
}

bool FindReplace::GetWrap() const
{
    return ui->chkWrap->isChecked();
}

bool FindReplace::IsCurrentFileScope() const
{
    return m_forceCurrentFile || GetLookWhere() == LookWhere_CurrentFile;
}

QStringList FindReplace::GetTargetFilePaths() const
{
    return m_main_window->GetSearchableFilePaths(IsCurrentFileScope());
}

void FindReplace::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    const QStringList find_strings = settings.value("find_strings").toStringList();
    const QStringList replace_strings = settings.value("replace_strings").toStringList();

    foreach (const QString &entry, find_strings) {
        ui->cbFind->addItem(entry);
    }

    foreach (const QString &entry, replace_strings) {
        ui->cbReplace->addItem(entry);
    }

    ui->cbFind->setEditText(settings.value("find").toString());
    ui->cbReplace->setEditText(settings.value("replace").toString());

    int search_mode = settings.value("search_mode", SearchOperations::SearchMode_Normal).toInt();
    const int look_where = settings.value("look_where", LookWhere_CurrentFile).toInt();
    const int direction = settings.value("direction", SearchDirection_Down).toInt();

    const int mode_index = ui->cbSearchMode->findData(search_mode);
    if (mode_index < 0) {
        search_mode = SearchOperations::SearchMode_Normal;
    }

    ui->cbSearchMode->setCurrentIndex(ui->cbSearchMode->findData(search_mode));
    ui->cbLookWhere->setCurrentIndex(ui->cbLookWhere->findData(look_where));
    ui->cbSearchDirection->setCurrentIndex(ui->cbSearchDirection->findData(direction));
    ui->chkWrap->setChecked(settings.value("wrap", true).toBool());

    settings.endGroup();
}

void FindReplace::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("find", GetFindText());
    settings.setValue("replace", GetReplaceText());
    settings.setValue("search_mode", ui->cbSearchMode->currentData().toInt());
    settings.setValue("look_where", ui->cbLookWhere->currentData().toInt());
    settings.setValue("direction", ui->cbSearchDirection->currentData().toInt());
    settings.setValue("wrap", ui->chkWrap->isChecked());
    settings.endGroup();
}

void FindReplace::UpdatePreviousFindStrings()
{
    const QString text = GetFindText();
    if (text.isEmpty()) {
        return;
    }

    const int index = ui->cbFind->findText(text);
    if (index != -1) {
        ui->cbFind->removeItem(index);
    }

    ui->cbFind->insertItem(0, text);
    ui->cbFind->setCurrentIndex(0);

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QStringList entries;

    for (int i = 0; i < ui->cbFind->count() && i < 25; ++i) {
        entries << ui->cbFind->itemText(i);
    }

    settings.setValue("find_strings", entries);
    settings.endGroup();
}

void FindReplace::UpdatePreviousReplaceStrings()
{
    const QString text = GetReplaceText();
    if (text.isEmpty()) {
        return;
    }

    const int index = ui->cbReplace->findText(text);
    if (index != -1) {
        ui->cbReplace->removeItem(index);
    }

    ui->cbReplace->insertItem(0, text);
    ui->cbReplace->setCurrentIndex(0);

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QStringList entries;

    for (int i = 0; i < ui->cbReplace->count() && i < 25; ++i) {
        entries << ui->cbReplace->itemText(i);
    }

    settings.setValue("replace_strings", entries);
    settings.endGroup();
}

void FindReplace::ClearMessage()
{
    ui->message->clear();
}

void FindReplace::ShowMessage(const QString &message)
{
    ui->message->setText(message);
    m_main_window->ShowMessageOnStatusBar(message);
}

void FindReplace::ShowNotFound()
{
    ShowMessage(tr("No matches found."));
    if (ui->cbFind->lineEdit()) {
        ui->cbFind->lineEdit()->setProperty("notfound", true);
        ui->cbFind->lineEdit()->style()->unpolish(ui->cbFind->lineEdit());
        ui->cbFind->lineEdit()->style()->polish(ui->cbFind->lineEdit());
    }
}

bool FindReplace::IsValidFindText() const
{
    return !GetFindText().isEmpty();
}

void FindReplace::ResetSearchOffset()
{
    m_lastSearchOffset = -1;
    m_lastSearchFile.clear();
}

void FindReplace::HighlightMatch(const QString &matched_text)
{
    if (matched_text.isEmpty()) {
        return;
    }

    UpdateFindFlags();
    m_view->findText(matched_text, m_findFlags);
}

void FindReplace::ScrollToMatchOffset(int offset, const QString &source)
{
    m_main_window->ScrollSourceToOffset(offset, source);
}

bool FindReplace::FileContainsMatch(const QString &relative_path,
                                    const QRegularExpression &search_regex) const
{
    const QString text = m_main_window->GetSearchableFileText(relative_path);
    return SearchOperations::CountInText(text, search_regex) > 0;
}

bool FindReplace::FindInCurrentFileSource(bool search_backward)
{
    const QString relative_path = m_main_window->GetCurrentSpineRelativePath();
    const QString text = m_main_window->GetSearchableFileText(relative_path);
    const QRegularExpression search_regex = GetSearchRegex();

    int start_offset = m_lastSearchOffset;
    if (m_lastSearchFile != relative_path) {
        start_offset = -1;
    }

    if (!search_backward) {
        start_offset = (start_offset < 0) ? 0 : start_offset + 1;
    } else if (start_offset < 0) {
        start_offset = text.length();
    }

    int match_start = 0;
    int match_length = 0;
    QString matched_text;

    if (!SearchOperations::FindMatchInText(text,
                                           search_regex,
                                           start_offset,
                                           search_backward,
                                           match_start,
                                           match_length,
                                           matched_text)) {
        if (GetWrap() && start_offset > 0) {
            const int wrap_start = search_backward ? text.length() : 0;
            if (SearchOperations::FindMatchInText(text,
                                                  search_regex,
                                                  wrap_start,
                                                  search_backward,
                                                  match_start,
                                                  match_length,
                                                  matched_text)) {
                ShowMessage(tr("Search wrapped."));
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    m_lastSearchOffset = match_start;
    m_lastSearchFile = relative_path;
    ScrollToMatchOffset(match_start, text);
    HighlightMatch(matched_text);
    ClearMessage();

    if (ui->cbFind->lineEdit()) {
        ui->cbFind->lineEdit()->setProperty("notfound", false);
        ui->cbFind->lineEdit()->style()->unpolish(ui->cbFind->lineEdit());
        ui->cbFind->lineEdit()->style()->polish(ui->cbFind->lineEdit());
    }

    UpdatePreviousFindStrings();
    WriteSettings();
    return true;
}

bool FindReplace::FindInAllFiles(bool search_backward)
{
    const QStringList files = m_main_window->GetSearchableFilePaths(false);
    if (files.isEmpty()) {
        return false;
    }

    const QRegularExpression search_regex = GetSearchRegex();
    const int current_index = m_main_window->GetCurrentSpineIndex();
    const int file_count = files.count();

    for (int step = 0; step < file_count; ++step) {
        int file_index;

        if (!search_backward) {
            file_index = (current_index + step) % file_count;
        } else {
            file_index = (current_index - step + file_count) % file_count;
        }

        const QString relative_path = files.at(file_index);
        const bool same_file = (relative_path == m_main_window->GetCurrentSpineRelativePath());

        if (same_file) {
            if (FindInCurrentFileSource(search_backward)) {
                return true;
            }
            continue;
        }

        if (!FileContainsMatch(relative_path, search_regex)) {
            continue;
        }

        m_main_window->GoToSpineFile(file_index);
        ResetSearchOffset();
        m_lastSearchFile = relative_path;

        if (FindInCurrentFileSource(search_backward)) {
            if (step > 0) {
                ShowMessage(tr("Continued search in another file."));
            }
            return true;
        }
    }

    if (GetWrap()) {
        ShowMessage(tr("Search wrapped."));
    }

    return false;
}

bool FindReplace::FindText(bool search_backward, bool force_current_file)
{
    if (!IsValidFindText()) {
        return false;
    }

    ClearMessage();
    const bool found = (force_current_file || IsCurrentFileScope())
                       ? FindInCurrentFileSource(search_backward)
                       : FindInAllFiles(search_backward);

    if (!found) {
        ShowNotFound();
    }

    return found;
}

int FindReplace::ReplaceText(bool search_backward, bool replace_current_only)
{
    if (!IsValidFindText()) {
        return 0;
    }

    ClearMessage();

    const QString relative_path = m_main_window->GetCurrentSpineRelativePath();
    QString text = m_main_window->GetSearchableFileText(relative_path);
    const QRegularExpression search_regex = GetSearchRegex();

    int start_offset = m_lastSearchOffset;
    if (m_lastSearchFile != relative_path) {
        start_offset = -1;
    }

    if (!search_backward) {
        start_offset = (start_offset < 0) ? 0 : start_offset;
    } else if (start_offset < 0) {
        start_offset = text.length();
    }

    int match_start = 0;
    int match_length = 0;
    QString matched_text;

    if (!SearchOperations::FindMatchInText(text,
                                           search_regex,
                                           start_offset,
                                           search_backward,
                                           match_start,
                                           match_length,
                                           matched_text)) {
        ShowNotFound();
        return 0;
    }

    QRegularExpressionMatch match = search_regex.match(text, match_start);
    const QString replacement_text = SearchOperations::BuildReplacementText(match, GetReplaceText());
    text.replace(match_start, match_length, replacement_text);

    if (!m_main_window->SetSearchableFileText(relative_path, text)) {
        ShowMessage(tr("Replace failed."));
        return 0;
    }

    m_lastSearchOffset = match_start + replacement_text.length() - 1;
    m_lastSearchFile = relative_path;

    ScrollToMatchOffset(match_start, text);
    HighlightMatch(replacement_text);

    int count = 1;
    ShowMessage(tr("Replacements made: %1").arg(count));
    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
    WriteSettings();

    if (!replace_current_only) {
        FindText(search_backward, true);
    }

    return count;
}

int FindReplace::ReplaceAllInScope(bool force_current_file)
{
    if (!IsValidFindText()) {
        return 0;
    }

    ClearMessage();

    const QStringList file_paths = m_main_window->GetSearchableFilePaths(force_current_file || IsCurrentFileScope());
    const QRegularExpression search_regex = GetSearchRegex();
    const QString replacement = GetReplaceText();

    auto read_text = [this](const QString &relative_path) {
        return m_main_window->GetSearchableFileText(relative_path);
    };

    auto write_text = [this](const QString &relative_path, const QString &text) {
        return m_main_window->SetSearchableFileText(relative_path, text);
    };

    const int count = SearchOperations::ReplaceInFiles(file_paths,
                                                       read_text,
                                                       write_text,
                                                       search_regex,
                                                       replacement,
                                                       this);

    if (count == 0) {
        ShowNotFound();
    } else {
        ShowMessage(tr("Replacements made: %1").arg(count));
    }

    ResetSearchOffset();
    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
    WriteSettings();
    return count;
}

int FindReplace::CountInScope(bool force_current_file)
{
    if (!IsValidFindText()) {
        return 0;
    }

    ClearMessage();

    const QStringList file_paths = m_main_window->GetSearchableFilePaths(force_current_file || IsCurrentFileScope());
    const QRegularExpression search_regex = GetSearchRegex();

    auto read_text = [this](const QString &relative_path) {
        return m_main_window->GetSearchableFileText(relative_path);
    };

    const int count = SearchOperations::CountInFiles(file_paths, read_text, search_regex, this);

    if (count == 0) {
        ShowNotFound();
    } else {
        ShowMessage(tr("Matches found: %1").arg(count));
    }

    UpdatePreviousFindStrings();
    WriteSettings();
    return count;
}
