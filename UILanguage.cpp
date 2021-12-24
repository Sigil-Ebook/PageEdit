/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario Canada 
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include <QDir>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QString>
#include "UILanguage.h"
#include "pageedit_constants.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
#include <stdlib.h>
#endif

static QString TRANSLATION_FILE_PREFIX = "pageedit_";
static QString TRANSLATION_FILE_SUFFIX = ".qm";

QStringList UILanguage::GetPossibleTranslationPaths()
{
    // There are a few different places translations can be stored depending
    // on the platform and where they were installed.
    QStringList possible_qm_locations;
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    possible_qm_locations.append(pageedit_share_root + "/translations/");
#endif
    
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    possible_qm_locations.append(QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#else
    possible_qm_locations.append(QLibraryInfo::path(QLibraryInfo::TranslationsPath));
#endif
    

#ifdef Q_OS_MAC
    possible_qm_locations.append(QCoreApplication::applicationDirPath() + "/../translations");
#else
    possible_qm_locations.append(QCoreApplication::applicationDirPath() + "/translations");
#endif

    return possible_qm_locations;
}

QStringList UILanguage::GetUILanguages()
{
    QStringList ui_languages;
    QStringList checked_dirs;
    foreach(QString path, GetPossibleTranslationPaths()) {
        // Find all translation files and add them to the avaliable list.
        QDir translationDir(path);

        if (translationDir.exists() && !checked_dirs.contains(translationDir.absolutePath())) {
            QStringList filters;
            // Look for all pageedit_*.qm files.
            filters << TRANSLATION_FILE_PREFIX + "*" + TRANSLATION_FILE_SUFFIX;
            translationDir.setNameFilters(filters);
            QStringList translation_files = translationDir.entryList();
            foreach(QString file, translation_files) {
                QFileInfo fileInfo(file);
                QString basename = fileInfo.baseName();
                QString language = basename.right(basename.length() - TRANSLATION_FILE_PREFIX.length());
                ui_languages.append(language);
            }
            checked_dirs.append(translationDir.absolutePath());
        }
    }
    return ui_languages;
}
