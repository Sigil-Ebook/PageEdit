/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario, Canada
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <iostream>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>
#include <QFileInfo>
#include <QDebug>

#include "MainApplication.h"
#include "MainWindow.h"
#include "Utility.h"

// Creates a MainWindow instance depending
// on command line arguments
static MainWindow *GetMainWindow(const QStringList &arguments)
{
    // We use the first argument as the file to load after starting
    QString filepath;
    if (arguments.size() > 1 && Utility::IsFileReadable(arguments.at(1))) {
        filepath = arguments.at(1);
    }
    return new MainWindow(filepath);
}

// fix me program icons


// Application entry point
int main(int argc, char *argv[])
{
    QT_REQUIRE_VERSION(argc, argv, "5.12.3");

    // Set application information for easier use of QSettings classes
    QCoreApplication::setOrganizationName("sigil-ebook");
    QCoreApplication::setOrganizationDomain("sigil-ebook.com");
    QCoreApplication::setApplicationName("pageedit");
    QCoreApplication::setApplicationVersion("0.0.1");
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache);

    MainApplication app(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

// fix me translations

// fix me icons

    const QStringList &arguments = QCoreApplication::arguments();
    MainWindow *widget = GetMainWindow(arguments);
    widget->show();
    return app.exec();
    
}
