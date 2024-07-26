/************************************************************************
**
**  Copyright (C) 2019-2024  Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2019  Doug Massay
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
#include <QStringList>
#include "UIDictionary.h"
#include "Utility.h"

#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
#include <stdlib.h>
#endif

static QString DICTIONARY_FILE_SUFFIX = ".bdic";

QString UIDictionary::GetDictionaryPath()
{
    QString dict_path;

    // the environment variable takes precedence on all platforms
    if (qEnvironmentVariableIsSet("QTWEBENGINE_DICTIONARIES_PATH")) {
        dict_path = Utility::GetEnvironmentVar("QTWEBENGINE_DICTIONARIES_PATH");
        return dict_path;
    }

    // next look relative to the executable
#ifndef Q_OS_MAC
    dict_path = QCoreApplication::applicationDirPath() + "/qtwebengine_dictionaries";
#else
    dict_path = QCoreApplication::applicationDirPath() + "/../Resources/qtwebengine_dictionaries";
#endif

    if (QDir(dict_path).exists()) {
        return dict_path;
    }

    // finally look inside the installed Qt directories
#ifndef Q_OS_MAC
    dict_path = QLibraryInfo::path(QLibraryInfo::DataPath) + "/qtwebengine_dictionaries";
#else
    dict_path = QCoreApplication::applicationDirPath() + 
      "/../Frameworks/QtWebEngineCore.framework/Resources/qtwebengine_dictionaries";
#endif

    if (QDir(dict_path).exists()) {
        return dict_path;
    }

    return QString();
}


QStringList UIDictionary::GetUIDictionaries()
{
    QStringList dictionaries;
    QString dict_path = GetDictionaryPath();
    if (dict_path.isEmpty()) {
        return dictionaries;
    }
    QDir dictDir(dict_path);
    if (dictDir.exists()) {
        QStringList filters;
        // Look for all *.bdic files.
        filters << "*" + DICTIONARY_FILE_SUFFIX;
        dictDir.setNameFilters(filters);
        QStringList dictionary_files = dictDir.entryList();
        foreach(QString file, dictionary_files) {
            QFileInfo fileInfo(file);
            QString dname = fileInfo.baseName();
            dictionaries.append(dname);
        }
    }
    return dictionaries;
}
