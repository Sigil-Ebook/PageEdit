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

#include <tuple>

#include <QApplication>
#include <QProgressDialog>

#include "SearchOperations.h"

// Skip XHTML tags so literal search/replace targets text content only (from Sigil).
static const QString REGEX_OPTION_TEXT_ONLY = QStringLiteral("<[^<>]*>(*SKIP)(*F)|");

QRegularExpression SearchOperations::BuildSearchRegex(const QString &find_text,
                                                      SearchMode search_mode,
                                                      bool dot_all)
{
    QString pattern = find_text;
    QRegularExpression::PatternOptions opts;

    // Literal text search only — patterns are always escaped (no regex mode).
    if (search_mode == SearchMode_Normal) {
        opts |= QRegularExpression::CaseInsensitiveOption;
    }

    pattern = QRegularExpression::escape(pattern);
    pattern = REGEX_OPTION_TEXT_ONLY + pattern;

    if (dot_all) {
        opts |= QRegularExpression::DotMatchesEverythingOption;
    }

    return QRegularExpression(pattern, opts);
}

int SearchOperations::CountInText(const QString &text, const QRegularExpression &search_regex)
{
    int count = 0;
    QRegularExpressionMatchIterator it = search_regex.globalMatch(text);

    while (it.hasNext()) {
        it.next();
        count++;
    }

    return count;
}

int SearchOperations::CountInFiles(const QStringList &file_paths,
                                   const std::function<QString(const QString &)> &read_text,
                                   const QRegularExpression &search_regex,
                                   QWidget *parent)
{
    QProgressDialog progress(QObject::tr("Counting occurrences..."), QString(), 0, file_paths.count(), parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    int total = 0;

    for (int i = 0; i < file_paths.count(); ++i) {
        progress.setValue(i);
        qApp->processEvents();
        total += CountInText(read_text(file_paths.at(i)), search_regex);
    }

    progress.setValue(file_paths.count());
    return total;
}

QString SearchOperations::BuildReplacementText(const QRegularExpressionMatch &match,
                                               const QString &replacement)
{
    Q_UNUSED(match)
    return replacement;
}

int SearchOperations::ReplaceInText(QString &text,
                                    const QRegularExpression &search_regex,
                                    const QString &replacement)
{
    int count = 0;
    QList<QRegularExpressionMatch> matches;
    QRegularExpressionMatchIterator it = search_regex.globalMatch(text);

    while (it.hasNext()) {
        matches.append(it.next());
    }

    for (int i = matches.count() - 1; i >= 0; --i) {
        const QRegularExpressionMatch &match = matches.at(i);
        const QString replacement_text = BuildReplacementText(match, replacement);
        text.replace(match.capturedStart(), match.capturedLength(), replacement_text);
        count++;
    }

    return count;
}

int SearchOperations::ReplaceInFiles(const QStringList &file_paths,
                                       const std::function<QString(const QString &)> &read_text,
                                       const std::function<bool(const QString &, const QString &)> &write_text,
                                       const QRegularExpression &search_regex,
                                       const QString &replacement,
                                       QWidget *parent)
{
    QProgressDialog progress(QObject::tr("Replacing search term..."), QString(), 0, file_paths.count(), parent);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(500);

    int total = 0;

    for (int i = 0; i < file_paths.count(); ++i) {
        progress.setValue(i);
        qApp->processEvents();

        const QString path = file_paths.at(i);
        QString text = read_text(path);
        const int count = ReplaceInText(text, search_regex, replacement);

        if (count > 0) {
            write_text(path, text);
        }

        total += count;
    }

    progress.setValue(file_paths.count());
    return total;
}

bool SearchOperations::FindMatchInText(const QString &text,
                                       const QRegularExpression &search_regex,
                                       int start_offset,
                                       bool search_backward,
                                       int &match_start,
                                       int &match_length,
                                       QString &matched_text)
{
    if (!search_backward) {
        QRegularExpressionMatch match = search_regex.match(text, start_offset);

        if (match.hasMatch()) {
            match_start = match.capturedStart();
            match_length = match.capturedLength();
            matched_text = match.captured(0);
            return true;
        }

        return false;
    }

    if (start_offset < 0) {
        start_offset = text.length();
    }

    int last_start = -1;
    int last_length = 0;
    QString last_text;
    QRegularExpressionMatchIterator it = search_regex.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        if (match.capturedStart() < start_offset) {
            last_start = match.capturedStart();
            last_length = match.capturedLength();
            last_text = match.captured(0);
        } else {
            break;
        }
    }

    if (last_start >= 0) {
        match_start = last_start;
        match_length = last_length;
        matched_text = last_text;
        return true;
    }

    return false;
}
