/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#pragma once
#ifndef UTILITY_H
#define UTILITY_H

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QStringRef>

class QWidget;

struct ExceptionBase;

class Utility
{
    Q_DECLARE_TR_FUNCTIONS(Utility)

public:

    enum Casing {
        Casing_Uppercase,     /**< Change characters to uppercase */
        Casing_Lowercase,     /**< Change characters to lowercase */
        Casing_Titlecase,    /**< Change first character of each word to uppercase, reset to lowercase. */
        Casing_Capitalize,    /**< Change first character of sentence to uppercase, rest to lowercase. */
    };

    static QString ChangeCase(const QString &text, const Casing &casing);

    // Define the user preferences location to be used
    static QString DefinePrefsDir();

    // Uses QUuid to generate a random UUID but also removes
    // the curly braces that QUuid::createUuid() adds
    static QString CreateUUID();

    // Returns true if the string is mixed case, false otherwise.
    // For instance, "test" and "TEST" return false, "teSt" returns true.
    static bool IsMixedCase(const QString &string);

    // Returns a substring of a specified string;
    // the characters included are in the interval:
    // [ start_index, end_index >
    static QString Substring(int start_index, int end_index, const QString &string);

    // Returns a substring of a specified string;
    // the characters included are in the interval:
    // [ start_index, end_index >
    static QStringRef SubstringRef(int start_index, int end_index, const QString &string);

    // Replace the first occurrence of string "before"
    // with string "after" in string "string"
    static QString ReplaceFirst(const QString &before, const QString &after, const QString &string);

    static QStringList GetAbsolutePathsToFolderDescendantFiles(const QString &fullfolderpath);

    // Copies every file and folder in the source folder
    // to the destination folder; the paths to the folders are submitted;
    // the destination folder needs to be created in advance
    static void CopyFiles(const QString &fullfolderpath_source, const QString &fullfolderpath_destination);

    // Johns own recursive directory removal code
    static bool removeDir(const QString &dirName);

    // Deletes the specified file if it exists
    static bool SDeleteFile(const QString &fullfilepath);

    static bool ForceCopyFile(const QString &fullinpath, const QString &fulloutpath);

    static bool RenameFile(const QString &oldfilepath, const QString &newfilepath);

    // Returns true if the file can be read;
    // shows an error dialog if it can't
    // with a message elaborating what's wrong
    static bool IsFileReadable(const QString &fullfilepath);

    // Reads the text file specified with the full file path;
    // text needs to be in UTF-8 or UTF-16; if the file cannot
    // be read, an error dialog is shown and an empty string returned
    static QString ReadUnicodeTextFile(const QString &fullfilepath);

    // Writes the provided text variable to the specified
    // file; if the file exists, it is truncated
    static void WriteUnicodeTextFile(const QString &text, const QString &fullfilepath);

    // Converts Mac and Windows style line endings to Unix style
    // line endings that are expected throughout the Qt framework
    static QString ConvertLineEndings(const QString &text);

    // Decodes XML escaped string to normal text
    // &amp; -> &    &apos; -> '  &quot; -> "   &lt; -> <  &gt; -> >
    static QString DecodeXML(const QString &text);

    // Encodes (Escapes) XML string
    // & -> &amp;    ' -> &apos;    " -> &quot;   < -> &lt;     > ->  &gt;
    static QString EncodeXML(const QString &text);


    /**
     * URL encodes the provided path string.
     * The path separator ('/') and the ID hash ('#') are left alone.
     *
     * @param path The path to encode.
     * @return The encoded path string.
     */
    static bool NeedToPercentEncode(uint32_t cp);
    static QString URLEncodePath(const QString &path);

    /**
     * URL decodes the provided path string.
     *
     * @param path The path to decode.
     * @return The decoded path string.
     */
    static QString URLDecodePath(const QString &path);

    static void DisplayStdErrorDialog(const QString &error_message, const QString &detailed_text = QString());

    static void DisplayStdWarningDialog(const QString &warning_message, const QString &detailed_text = QString());

    static void DisplayExceptionErrorDialog(const QString &error_info);

    // Returns a value for the environment variable name passed;
    // if the env var isn't set, it returns an empty string
    static QString GetEnvironmentVar(const QString &variable_name);

    // Returns the same number, but rounded to one decimal place
    static float RoundToOneDecimal(float number);

    static QWidget *GetMainWindow();

    static QString getSpellingSafeText(const QString &raw_text);

    static bool has_non_ascii_chars(const QString &str);
    static bool use_filename_warning(const QString &filename);

#if defined(Q_OS_WIN32)
    static std::wstring QStringToStdWString(const QString &str);
    static QString stdWStringToQString(const std::wstring &str);
#endif

    static QString longestCommonPath(const QStringList& filepaths, const QString& sep);
    static QString resolveRelativeSegmentsInFilePath(const QString& file_path, const QString &sep);

    static QString buildBookPath(const QString& dest_relpath, const QString& start_folder);
    static QString startingDir(const QString &file_bookpath);
    static QString relativePath(const QString & destination, const QString & start_dir);
    static QString buildRelativePath(const QString &from_file_bkpath, const QString & to_file_bkpath);
    static std::pair<QString, QString> parseHREF(const QString &relative_href);

    static void AboutBox();


    // Indicates if application palette in dark mode
    static bool IsDarkMode();

    // Indicates Windows system dark mode is enabled
    static bool IsWindowsSysDarkMode();

    // Should Windows Sigil be using dark mode?
    static bool WindowsShouldUseDarkMode();

    // inject dark mode css into html
    static QString AddDarkCSS(const QString &html);

    // return the proper background color for QWebEngineView
    static QColor WebViewBackgroundColor(bool followpref = false);

};

#endif // UTILITY_H


