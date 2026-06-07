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
#ifndef SEARCHOPERATIONS_H
#define SEARCHOPERATIONS_H

#include <functional>

#include <QString>
#include <QStringList>
#include <QRegularExpression>

class QWidget;

class SearchOperations
{
public:

    enum SearchMode {
        SearchMode_Normal = 0,
        SearchMode_Case_Sensitive = 10
    };

    static QRegularExpression BuildSearchRegex(const QString &find_text,
                                               SearchMode search_mode,
                                               bool dot_all = false);

    static int CountInText(const QString &text, const QRegularExpression &search_regex);

    static int CountInFiles(const QStringList &file_paths,
                            const std::function<QString(const QString &)> &read_text,
                            const QRegularExpression &search_regex,
                            QWidget *parent = nullptr);

    static int ReplaceInText(QString &text,
                             const QRegularExpression &search_regex,
                             const QString &replacement);

    static int ReplaceInFiles(const QStringList &file_paths,
                              const std::function<QString(const QString &)> &read_text,
                              const std::function<bool(const QString &, const QString &)> &write_text,
                              const QRegularExpression &search_regex,
                              const QString &replacement,
                              QWidget *parent = nullptr);

    static bool FindMatchInText(const QString &text,
                                const QRegularExpression &search_regex,
                                int start_offset,
                                bool search_backward,
                                int &match_start,
                                int &match_length,
                                QString &matched_text);

    static QString BuildReplacementText(const QRegularExpressionMatch &match,
                                        const QString &replacement);
};

#endif // SEARCHOPERATIONS_H
