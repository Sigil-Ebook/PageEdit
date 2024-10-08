/************************************************************************
**
**  Copyright (C) 2019-2024 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <stdio.h>
#include <time.h>
#include <string>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>
#include <QStringView>
#include <QTextStream>
#include <QtGlobal>
#include <QUrl>
#include <QUuid>
#include <QMainWindow>
#include <QSettings>
#include <QTextEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QtSvg/QSvgRenderer>
#include <QVector>
#include <QDebug>

#include "SettingsStore.h"
#include "MainApplication.h"
#include "QuickParser.h"
#include "pageedit_constants.h"
#include "pageedit_exception.h"


#ifndef MAX_PATH
// Set Max length to 256 because that's the max path size on many systems.
#define MAX_PATH 256
#endif
// This is the same read buffer size used by Java and Perl.
#define BUFF_SIZE 8192

// Subclass QMessageBox for our StdWarningDialog to make any Details Resizable
class PageEditMessageBox: public QMessageBox
{
    public:
        PageEditMessageBox(QWidget* parent) : QMessageBox(parent) 
        {
        }
    private:
        virtual void resizeEvent(QResizeEvent * e) {
            QMessageBox::resizeEvent(e);
        }
};

#include "Utility.h"


static const QString URL_SAFE = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-/~";


static const QString DARK_STYLE =
    "<style id=\"PageEdit_Injected\">"
    "  :root { background-color: %1; color: %2; }\n"
    "  ::-webkit-scrollbar { display: none; }"
    "</style>"
    "<link rel=\"stylesheet\" type=\"text/css\" "
    "href=\"%3\" />";

// Define the user preferences location to be used
QString Utility::DefinePrefsDir()
{
        return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}


// Uses QUuid to generate a random UUID but also removes
// the curly braces that QUuid::createUuid() adds
QString Utility::CreateUUID()
{
    return QUuid::createUuid().toString().remove("{").remove("}");
}


// Convert the casing of the text, returning the result.
QString Utility::ChangeCase(const QString &text, const Utility::Casing &casing)
{
    if (text.isEmpty()) {
        return text;
    }

    switch (casing) {
        case Utility::Casing_Lowercase: {
            return text.toLower();
        }

        case Utility::Casing_Uppercase: {
            return text.toUpper();
        }

        case Utility::Casing_Titlecase: {
            // This is a super crude algorithm, could be replaced by something more clever.
            QString new_text = text.toLower();
            // Skip past any leading spaces
            int i = 0;

            while (i < text.length() && new_text.at(i).isSpace()) {
                i++;
            }

            while (i < text.length()) {
                if (i == 0 || new_text.at(i - 1).isSpace()) {
                    new_text.replace(i, 1, new_text.at(i).toUpper());
                }

                i++;
            }

            return new_text;
        }

        case Utility::Casing_Capitalize: {
            // This is a super crude algorithm, could be replaced by something more clever.
            QString new_text = text.toLower();
            // Skip past any leading spaces
            int i = 0;

            while (i < text.length() && new_text.at(i).isSpace()) {
                i++;
            }

            if (i < text.length()) {
                new_text.replace(i, 1, new_text.at(i).toUpper());
            }

            return new_text;
        }

        default:
            return text;
    }
}


// Returns true if the string is mixed case, false otherwise.
// For instance, "test" and "TEST" return false, "teSt" returns true.
// If the string is empty, returns false.
bool Utility::IsMixedCase(const QString &string)
{
    if (string.isEmpty() || string.length() == 1) {
        return false;
    }

    bool first_char_lower = string[ 0 ].isLower();

    for (int i = 1; i < string.length(); ++i) {
        if (string[ i ].isLower() != first_char_lower) {
            return true;
        }
    }

    return false;
}

// Returns a substring from a specified QStringView as a real QString;
// the characters included are in the interval:
// [ start_index, end_index >
QString Utility::Substring(int start_index, int end_index, const QStringView string)
{
    return string.sliced(start_index, end_index - start_index).toString();
}

// [ start_index, end_index >
QString Utility::Substring(int start_index, int end_index, const QString &string)
{
    return string.mid(start_index, end_index - start_index);
}

// Returns a substring of a specified string as a QStringView
// the characters included are in the interval:
// [ start_index, end_index >
QStringView Utility::SubstringView(int start_index, int end_index, const QString &string)
{
    return QStringView(string).sliced(start_index, end_index - start_index);
}

// Replace the first occurrence of string "before"
// with string "after" in string "string"
QString Utility::ReplaceFirst(const QString &before, const QString &after, const QString &string)
{
    int start_index = string.indexOf(before);
    int end_index   = start_index + before.length();
    return Substring(0, start_index, string) + after + Substring(end_index, string.length(), string);
}


QStringList Utility::GetAbsolutePathsToFolderDescendantFiles(const QString &fullfolderpath)
{
    QDir folder(fullfolderpath);
    QStringList files;
    foreach(QFileInfo file, folder.entryInfoList()) {
        if ((file.fileName() != ".") && (file.fileName() != "..")) {
            // If it's a file, add it to the list
            if (file.isFile()) {
                files.append(Utility::URLEncodePath(file.absoluteFilePath()));
            }
            // Else it's a directory, so
            // we add all files from that dir
            else {
                files.append(GetAbsolutePathsToFolderDescendantFiles(file.absoluteFilePath()));
            }
        }
    }
    return files;
}


// Copies every file and folder in the source folder
// to the destination folder; the paths to the folders are submitted;
// the destination folder needs to be created in advance
void Utility::CopyFiles(const QString &fullfolderpath_source, const QString &fullfolderpath_destination)
{
    QDir folder_source(fullfolderpath_source);
    QDir folder_destination(fullfolderpath_destination);
    // Erase all the files in this folder
    foreach(QFileInfo file, folder_source.entryInfoList()) {
        if ((file.fileName() != ".") && (file.fileName() != "..")) {
            // If it's a file, copy it
            if (file.isFile()) {
                QString destination = fullfolderpath_destination + "/" + file.fileName();
                bool success = QFile::copy(file.absoluteFilePath(), destination);

                if (!success) {
                    std::string msg = file.absoluteFilePath().toStdString() + ": " + destination.toStdString();
                    throw(CannotCopyFile(msg));
                }
            }
            // Else it's a directory, copy everything in it
            // to a new folder of the same name in the destination folder
            else {
                folder_destination.mkpath(file.fileName());
                CopyFiles(file.absoluteFilePath(), fullfolderpath_destination + "/" + file.fileName());
            }
        }
    }
}



//
//   Delete a directory along with all of its contents.
//
//   \param dirName Path of directory to remove.
//   \return true on success; false on error.
//
bool Utility::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}



// Deletes the specified file if it exists
bool Utility::SDeleteFile(const QString &fullfilepath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(fullfilepath).exists()) {
        return false;
    }

    QFile file(fullfilepath);
    bool deleted = file.remove();
    // Some multiple file deletion operations fail on Windows, so we try once more.
    if (!deleted) {
        qApp->processEvents();
        deleted = file.remove();
    }
    return deleted;
}


// Copies File from full Inpath to full OutPath with overwrite if needed
bool Utility::ForceCopyFile(const QString &fullinpath, const QString &fulloutpath)
{
    if (!QFileInfo(fullinpath).exists()) {
        return false;
    }
    if (QFileInfo::exists(fulloutpath)) {
        Utility::SDeleteFile(fulloutpath);
    }
    return QFile::copy(fullinpath, fulloutpath);
}


bool Utility::RenameFile(const QString &oldfilepath, const QString &newfilepath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(oldfilepath).exists()) {
        return false;
    }

    // Ensure that the newfilepath doesn't already exist but due to case insenstive file systems
    // check if we are actually renaming to an identical path with a different case.
    if (QFileInfo(newfilepath).exists() && QFileInfo(oldfilepath) != QFileInfo(newfilepath)) {
        return false;
    }

    // On case insensitive file systems, QFile::rename fails when the new name is the
    // same (case insensitive) to the old one. This is workaround for that issue.
    int ret = -1;
#if defined(Q_OS_WIN32)
    ret = _wrename(Utility::QStringToStdWString(oldfilepath).data(), Utility::QStringToStdWString(newfilepath).data());
#else
    ret = rename(oldfilepath.toUtf8().data(), newfilepath.toUtf8().data());
#endif

    if (ret == 0) {
        return true;
    }

    return false;
}


// Returns true if the file can be read;
// shows an error dialog if it can't
// with a message elaborating what's wrong
bool Utility::IsFileReadable(const QString &fullfilepath)
{
    // Qt has <QFileInfo>.exists() and <QFileInfo>.isReadable()
    // functions, but then we would have to create our own error
    // message for each of those situations (and more). Trying to
    // actually open the file enables us to retrieve the exact
    // reason preventing us from reading the file in an error string.
    QFile file(fullfilepath);

    // Check if we can open the file
    if (!file.open(QFile::ReadOnly)) {
        Utility::DisplayStdErrorDialog(
            QObject::tr("Cannot read file %1:\n%2.")
            .arg(fullfilepath)
            .arg(file.errorString())
        );
        return false;
    }

    file.close();
    return true;
}


// Reads the text file specified with the full file path;
// text needs to be in UTF-8 or UTF-16; if the file cannot
// be read, an error dialog is shown and an empty string returned
QString Utility::ReadUnicodeTextFile(const QString &fullfilepath)
{
    // TODO: throw an exception instead of
    // returning an empty string
    QFile file(fullfilepath);

    // Check if we can open the file
    if (!file.open(QFile::ReadOnly)) {
        std::string msg = fullfilepath.toStdString() + ": " + file.errorString().toStdString();
        throw(CannotOpenFile(msg));
    }

    QTextStream in(&file);
    // Input should be UTF-8
    // This will automatically switch reading from
    // UTF-8 to UTF-16 if a BOM is detected
    in.setAutoDetectUnicode(true);
    return ConvertLineEndingsAndNormalize(in.readAll());
}


// Writes the provided text variable to the specified
// file; if the file exists, it is truncated
void Utility::WriteUnicodeTextFile(const QString &text, const QString &fullfilepath)
{
    QString newtext = Utility::UseNFC(text);
    QFile file(fullfilepath);

    if (!file.open(QIODevice::WriteOnly |
                   QIODevice::Truncate  |
                   QIODevice::Text
                  )
       ) {
        std::string msg = file.fileName().toStdString() + ": " + file.errorString().toStdString();
        throw(CannotOpenFile(msg));
    }

    QTextStream out(&file);
    // We ALWAYS output in UTF-8
    out << text;
}


// Converts Mac and Windows style line endings to Unix style
// line endings that are expected throughout the Qt framework
QString Utility::ConvertLineEndingsAndNormalize(const QString &text)
{
    QString newtext = Utility::UseNFC(text);
    return newtext.replace("\x0D\x0A", "\x0A").replace("\x0D", "\x0A");
}


// Decodes XML escaped string to normal text                                                                                
// &amp; -> "&"    &apos; -> "'"  &quot; -> "\""   &lt; -> "<"  &gt; -> ">"
QString Utility::DecodeXML(const QString &text)
{
    QString newtext(text);
    newtext.replace("&apos;", "'");
    newtext.replace("&quot;", "\"");
    newtext.replace("&lt;", "<");
    newtext.replace("&gt;", ">");
    newtext.replace("&amp;", "&");
    return newtext;
}

QString Utility::EncodeXML(const QString &text)
{
    QString newtext(text);
    return newtext.toHtmlEscaped();
}

// From the IRI spec rfc3987
// iunreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~" / ucschar
// 
//    ucschar        = %xA0-D7FF / %xF900-FDCF / %xFDF0-FFEF
//                   / %x10000-1FFFD / %x20000-2FFFD / %x30000-3FFFD
//                   / %x40000-4FFFD / %x50000-5FFFD / %x60000-6FFFD
//                   / %x70000-7FFFD / %x80000-8FFFD / %x90000-9FFFD
//                   / %xA0000-AFFFD / %xB0000-BFFFD / %xC0000-CFFFD
//                   / %xD0000-DFFFD / %xE1000-EFFFD
// But currently nothing *after* the 0x30000 plane is even defined

bool Utility::NeedToPercentEncode(uint32_t cp)
{
    // sequence matters for both correctness and speed
    if (cp < 128) {
        if (URL_SAFE.contains(QChar(cp))) return false;
        return true;
    }
    if (cp < 0xA0) return true;
    if (cp <= 0xD7FF) return false;
    if (cp < 0xF900) return true;
    if (cp <= 0xFDCF) return false;
    if (cp < 0xFDF0) return true;
    if (cp <= 0xFFEF) return false;
    if (cp < 0x10000) return true;
    if (cp <= 0x1FFFD) return false;
    if (cp < 0x20000) return true;
    if (cp <= 0x2FFFD) return false;
    if (cp < 0x30000) return true;
    if (cp <= 0x3FFFD) return false;
    return true;
}


// this is meant to work on paths, not paths and fragments and schemes
// therefore do not leave # chars unencoded
QString Utility::URLEncodePath(const QString &path)
{
   // some very poorly written software uses xml escaping of the 
   // "&" instead of url encoding when building hrefs
   // So run xmldecode first to convert them to normal characters before 
   // url encoding them
   QString newpath = DecodeXML(path);

   // The epub spec says all paths must use Unicode Normalization Form C (NFC)
    // So do NOT Use Utility::UseNFC which conditionalizes things
    newpath = newpath.normalized(QString::NormalizationForm_C);

   // then undo any existing url encoding
   newpath = URLDecodePath(newpath);

   QString result = "";
   QVector<uint32_t> codepoints = newpath.toUcs4();
   for (int i = 0; i < codepoints.size(); i++) {
       uint32_t cp = codepoints.at(i);
       QString s = QString::fromUcs4(reinterpret_cast<char32_t *>(&cp), 1);
       if (NeedToPercentEncode(cp)) {
           QByteArray b = s.toUtf8();
           for (int j = 0; j < b.size(); j++) {
               uint8_t bval = b.at(j);
               QString val = QString::number(bval,16);
               val = val.toUpper();
               if (val.size() == 1) val.prepend("0");
               val.prepend("%");
               result.append(val);
           }
       } else {
           result.append(s);
       }
    }
    // qDebug() << "In Utility URLEncodePath: " << result;
    // Previously was:
    // encoded_url = QUrl::toPercentEncoding(newpath, QByteArray("/"), QByteArray("#"));
    // encoded_path = scheme + QString::fromUtf8(encoded_url.constData(), encoded_url.count());
    return result;
}


QString Utility::URLDecodePath(const QString &path)
{
    QString apath(path);
    // some very poorly written software uses xml-escape on hrefs
    // instead of properly url encoding them, so look for the
    // the "&" character which should *not* exist if properly
    // url encoded and if found try to xml decode them first
    apath = DecodeXML(apath);
    QString newpath = QUrl::fromPercentEncoding(apath.toUtf8());
    // epub spec says all paths must use Normalization Form C (NFC)
    // Do Not use Utility::UseNFC as it conditionalizes it
    newpath = newpath.normalized(QString::NormalizationForm_C);
    return newpath;
}


void Utility::DisplayExceptionErrorDialog(const QString &error_info)
{
    QWidget* parent = QApplication::activeWindow();
    QMessageBox message_box(parent);
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle("PageEdit");
    // Spaces are added to the end because otherwise the dialog is too small.
    message_box.setText(QObject::tr("PageEdit has encountered a problem."));
    message_box.setInformativeText(QObject::tr("PageEdit may need to close."));
    message_box.setStandardButtons(QMessageBox::Close);
    QStringList detailed_text;
    detailed_text << "Error info: "       + error_info
                  << "PageEdit version: " + QString(PAGEEDIT_VERSION)
                  << "Qt Runtime Version: " + QString(qVersion())
                  << "Qt Compiled Version: " + QString(QT_VERSION_STR)
                  << "System: " + QSysInfo::prettyProductName()
                  << "Architecture: " + QSysInfo::currentCpuArchitecture();
    message_box.setDetailedText(detailed_text.join("\n"));
    message_box.exec();
#ifdef Q_OS_MAC
    if (parent) parent->activateWindow();
#endif
}


void Utility::DisplayStdErrorDialog(const QString &error_message, const QString &detailed_text)
{
    QWidget* parent = QApplication::activeWindow();
    QMessageBox message_box(parent);
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Critical);
    message_box.setWindowTitle("PageEdit");
    message_box.setText(error_message);

    if (!detailed_text.isEmpty()) {
        message_box.setDetailedText(detailed_text);
    }

    message_box.setStandardButtons(QMessageBox::Close);
    message_box.exec();
#ifdef Q_OS_MAC
    if (parent) parent->activateWindow();
#endif
}


void Utility::DisplayStdWarningDialog(const QString &warning_message, const QString &detailed_text)
{
    QWidget* parent = QApplication::activeWindow();
    PageEditMessageBox message_box(parent);
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setIcon(QMessageBox::Warning);
    message_box.setWindowTitle("PageEdit");
    message_box.setText(warning_message);
    message_box.setTextFormat(Qt::RichText);

    if (!detailed_text.isEmpty()) {
        message_box.setDetailedText(detailed_text);
    }
    message_box.setStandardButtons(QMessageBox::Close);
    message_box.exec();
#ifdef Q_OS_MAC
    if (parent) parent->activateWindow();
#endif
}


// Returns a value for the environment variable name passed;
// if the env var isn't set, it returns an empty string
QString Utility::GetEnvironmentVar(const QString &variable_name)
{
    // The only time this might fall down is on Linux when an
    // environment variable holds bytedata. Don't use this
    // utility function for retrieval if that's the case.
    return qEnvironmentVariable(variable_name.toUtf8().constData(), "").trimmed();
}


// Returns the same number, but rounded to one decimal place
float Utility::RoundToOneDecimal(float number)
{
    return QString::number(number, 'f', 1).toFloat();
}


QWidget *Utility::GetMainWindow()
{
    QWidget *parent_window = QApplication::activeWindow();

    while (parent_window && !(dynamic_cast<QMainWindow *>(parent_window))) {
        parent_window = parent_window->parentWidget();
    }

    return parent_window;
}

bool Utility::has_non_ascii_chars(const QString &str)
{
    QRegularExpression not_ascii("[^\\x00-\\x7F]");
    QRegularExpressionMatch mo = not_ascii.match(str);
    return mo.hasMatch();
}

bool Utility::use_filename_warning(const QString &filename)
{
    if (has_non_ascii_chars(filename)) {
        return QMessageBox::Apply == QMessageBox::warning(QApplication::activeWindow(),
                tr("PageEdit"),
                tr("The requested file name contains non-ASCII characters. "
                   "You should only use ASCII characters in filenames. "
                   "Using non-ASCII characters can prevent the EPUB from working "
                   "with some readers.\n\n"
                   "Continue using the requested filename?"),
                QMessageBox::Cancel|QMessageBox::Apply);
    }
    return true;
}

#if defined(Q_OS_WIN32)
std::wstring Utility::QStringToStdWString(const QString &str)
{
    return std::wstring((const wchar_t *)str.utf16());
}

QString Utility::stdWStringToQString(const std::wstring &str)
{
    return QString::fromUtf16((const ushort *)str.c_str());
}
#endif


#if 0
// brute force method
QString Utility::longestCommonPath(const QStringList& filepaths, const QString& sep)
{
    // handle special cases
    if (filepaths.isEmpty()) return QString();
    if (filepaths.length() == 1) return QFileInfo(filepaths.at(0)).absolutePath() + sep;
    
    // split each path into its component segments
    QList<QStringList> fpaths;
    int minlen = -1;
    foreach(QString apath, filepaths) {
        QStringList segs = apath.split(sep);
        int n = segs.length();
        if (minlen == -1) minlen = n;
        if (n < minlen) minlen = n;
        fpaths.append(segs);
    }

    // now build up the results
    QStringList res;
    int numpaths = fpaths.length();
    for(int i=0; i < minlen; i++) {
        QString aseg = fpaths.at(0).at(i);
        bool amatch = true;
        int j = 1;
        while(amatch && j < numpaths) {
            amatch = (aseg == fpaths.at(j).at(i));
            j++;
        }
        if (amatch) {
            res << aseg;
        } else {
            break;
        }
    }
    if (res.isEmpty()) return "";
    return res.join(sep) + sep;
}

#else
QString Utility::longestCommonPath(const QStringList& filepaths, const QString& sep)
{
    if (filepaths.isEmpty()) return QString();
    if (filepaths.length() == 1) return QFileInfo(filepaths.at(0)).absolutePath() + sep;
    QStringList fpaths(filepaths);
    fpaths.sort();
    const QStringList segs1 = fpaths.first().split(sep);
    const QStringList segs2 = fpaths.last().split(sep);
    QStringList res;
    int i = 0;
    while((i < segs1.length()) && (i < segs2.length()) && (segs1.at(i) == segs2.at(i))) {
        res.append(segs1.at(i));
        i++;
    }
    if (res.length() == 0) return sep;
    return res.join(sep) + sep;
}
#endif


// fixme this should also remove multiple path separators in a row after the first
QString Utility::resolveRelativeSegmentsInFilePath(const QString& file_path, const QString &sep)
{
    const QStringList segs = file_path.split(sep);
    QStringList res;
    for (int i = 0; i < segs.length(); i++) {
        if (segs.at(i) == ".") continue;
        if (segs.at(i) == "..") {
            if (!res.isEmpty()) {
	        res.removeLast();
            } else {
	        qDebug() << "Utility.cpp: Error resolving relative path segments";
            }
        } else {
            res << segs.at(i);
        }
    }
    return res.join(sep);
}


// dest_relpath is the relative path to the destination file
// start_folder is the *book path* (path internal to the epub) to the starting folder
QString Utility::buildBookPath(const QString& dest_relpath, const QString& start_folder)
{
    QString bookpath(start_folder);
    while (bookpath.endsWith("/")) bookpath.chop(1);
    if (!bookpath.isEmpty()) { 
        bookpath = bookpath + "/" + dest_relpath;
    } else {
        bookpath = dest_relpath;
    }
    bookpath = resolveRelativeSegmentsInFilePath(bookpath, "/");
    return bookpath;
}

// no ending path separator
QString Utility::startingDir(const QString &file_bookpath)
{
    QString start_dir(file_bookpath);
    int pos = start_dir.lastIndexOf('/');
    if (pos > -1) { 
        start_dir = start_dir.left(pos);
    } else {
        start_dir = "";
    }
    return start_dir;
}


// Generate relative path to destination from starting directory path
// Both paths should be cannonical
QString Utility::relativePath(const QString & destination, const QString & start_dir)
{
    QString dest(destination);
    QString start(start_dir);

    // first handle the special case
    if (start_dir.isEmpty()) return destination;

    QChar sep = '/';

    // remove any trailing path separators from both paths
    while (dest.endsWith(sep)) dest.chop(1);
    while (start.endsWith(sep)) start.chop(1);

    QStringList dsegs = dest.split(sep, Qt::KeepEmptyParts);
    QStringList ssegs = start.split(sep, Qt::KeepEmptyParts);
    QStringList res;
    int i = 0;
    int nd = dsegs.size();
    int ns = ssegs.size();
    // skip over starting common path segments in both paths
    while (i < ns && i < nd && (dsegs.at(i) == ssegs.at(i))) {
        i++;
    }
    // now "move up" for each remaining path segment in the starting directory
    int p = i;
    while (p < ns) {
        res.append("..");
        p++;
    }
    // And append the remaining path segments from the destination
    p = i;
    while(p < nd) {
        res.append(dsegs.at(p));
        p++;
    }
    return res.join(sep);
}


// This is the equivalent of Resource.cpp's GetRelativePathFromResource but using book paths
QString Utility::buildRelativePath(const QString &from_file_bkpath, const QString & to_file_bkpath)
{
    // handle special case of "from" and "to" being identical
    if (from_file_bkpath == to_file_bkpath) return "";

    // convert start_file_bkpath to start_dir by stripping off existing filename component
    return relativePath(to_file_bkpath, startingDir(from_file_bkpath));
}   


std::pair<QString, QString> Utility::parseHREF(const QString &relative_href)
{
    QString fragment;
    QString attpath = relative_href;
    int fragpos = attpath.lastIndexOf("#");
    // fragment will include any # if one exists
    if (fragpos != -1) {
        fragment = attpath.mid(fragpos, -1);
        attpath = attpath.mid(0, fragpos);
    }
    if (attpath.startsWith("./")) attpath = attpath.mid(2,-1);
    return std::make_pair(attpath, fragment);
}

void Utility::AboutBox()
{
    QMessageBox message_box(QApplication::activeWindow());
    message_box.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    message_box.setModal(true);
    message_box.setWindowTitle(QObject::tr("About PageEdit"));
    message_box.setStandardButtons(QMessageBox::Close);
    QStringList about_text;
    about_text    << "<h1>PageEdit</h1>"
                  << "<ul>"
                  << "<li><b>" + QObject::tr("Version") + ":</b>             " + QString(PAGEEDIT_VERSION) + "</li>"
	          << "<li><b>" + QObject::tr("Build Date") + ":</b>          " + QString::fromLatin1(__DATE__) + "</li>"
	          << "<li><b>" + QObject::tr("Build Time") + ":</b>          " + QString::fromLatin1(__TIME__) + "</li>"
                  << "<li><b>" + QObject::tr("Qt Runtime Version")+ ":</b>   " + QString(qVersion()) + "</li>"
                  << "<li><b>" + QObject::tr("Qt Compiled Version") + ":</b> " + QString(QT_VERSION_STR) + "</li>"
                  << "<li><b>" + QObject::tr("System") + ":</b>              " + QSysInfo::prettyProductName() + "</li>"
                  << "<li><b>" + QObject::tr("Architecture") + ":</b>        " + QSysInfo::currentCpuArchitecture() + "</li>";
    about_text << "</ul>";
    message_box.setText(about_text.join("\n"));
    message_box.setIconPixmap(QPixmap(":/icons/app_icon_128.png"));
    message_box.exec();
}

bool Utility::IsDarkMode()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    return mainApplication->isDarkMode();
#else
    // Windows, Linux and Other platforms
    QPalette app_palette = qApp->palette();
    bool isdark = app_palette.color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    return isdark;
#endif
}

bool Utility::IsWindowsSysDarkMode()
{
    QSettings s("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if (s.status() == QSettings::NoError) {
        return s.value("AppsUseLightTheme") == 0;
    }
    return false;
}

bool Utility::WindowsShouldUseDarkMode()
{
    QString override(GetEnvironmentVar("PAGEEDIT_USES_DARK_MODE"));
    if (override.isEmpty()) {
        //Env var unset - use system registry setting.
        return IsWindowsSysDarkMode();
    }
    // Otherwise use the env var: anything other than "0" is true.
    return (override == "0" ? false : true);
}

QString Utility::AddDarkCSS(const QString &html)
{
    QString text = html;
    int endheadpos = text.indexOf("</head>");
    if (endheadpos == -1) return text;
    QPalette pal = qApp->palette();
    QString back = pal.color(QPalette::Base).name();
    QString fore = pal.color(QPalette::Text).name();
#ifdef Q_OS_MAC
    QString dark_css_url = "qrc:///dark/mac_dark_scrollbar.css";
#elif defined(Q_OS_WIN32)
    QString dark_css_url = "qrc:///dark/win_dark_scrollbar.css";
#else
    // Linux
    QString dark_css_url = "qrc:///dark/lin_dark_scrollbar.css";
#endif
    QString inject_dark_style = DARK_STYLE.arg(back).arg(fore).arg(dark_css_url);
    text.insert(endheadpos, inject_dark_style);
    return text;
}


QColor Utility::WebViewBackgroundColor(bool followpref)
{
    QColor back_color = Qt::white;
    if (IsDarkMode()) {
        if (followpref) {
            SettingsStore ss;
            if (!ss.previewDark()) {
                return back_color;    
            }
        }
        QPalette pal = qApp->palette();
        back_color = pal.color(QPalette::Base);
    }
    return back_color; 
}


QMessageBox::StandardButton Utility::warning(QWidget* parent, const QString &title, const QString &text,
                                             QMessageBox::StandardButtons buttons,
                                             QMessageBox::StandardButton defaultButton)
{
  QMessageBox::StandardButton result = QMessageBox::warning(parent, title, text, buttons, defaultButton);
#ifdef Q_OS_MAC
  if (parent) parent->activateWindow();
#endif
  return result;
}


QMessageBox::StandardButton Utility::question(QWidget* parent, const QString &title, const QString &text,
                                              QMessageBox::StandardButtons buttons,
                                              QMessageBox::StandardButton defaultButton)
{
  QMessageBox::StandardButton result = QMessageBox::question(parent, title, text, buttons, defaultButton);
#ifdef Q_OS_MAC
  if (parent) parent->activateWindow();
#endif
  return result;
}


QMessageBox::StandardButton Utility::information(QWidget* parent, const QString &title, const QString &text,
                                                 QMessageBox::StandardButtons buttons,
                                                 QMessageBox::StandardButton defaultButton)
{
  QMessageBox::StandardButton result = QMessageBox::information(parent, title, text, buttons, defaultButton);
#ifdef Q_OS_MAC
  if (parent) parent->activateWindow();
#endif
  return result;
}


QMessageBox::StandardButton Utility::critical(QWidget* parent, const QString &title, const QString &text,
                                              QMessageBox::StandardButtons buttons,
                                              QMessageBox::StandardButton defaultButton)
{
  QMessageBox::StandardButton result = QMessageBox::critical(parent, title, text, buttons, defaultButton);
#ifdef Q_OS_MAC
  if (parent) parent->activateWindow();
#endif
  return result;
}

// QtSvg is broken with respect to desc and title tags used
// inside text tags and does not support flowRoot and its children
// so strip them all out before trying to render using QSvgRenderer
QString Utility::FixupSvgForRendering(const QString& data)
{
    QStringList svgdata;
    QuickParser qp(data);
    bool skip = false;
    bool in_svg = false;
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
	if (mi.tname == "svg" && mi.ttype == "begin") {
	    in_svg = true;
        }
	if (mi.tname == "svg" && mi.ttype == "end") {
	    in_svg = false;
	    skip = false;
	}
	if (in_svg) {
            if (mi.tname == "desc" || mi.tname == "title" || mi.tname == "flowRoot") {
	        if (mi.ttype == "single") continue; // no need to update skip since open self-closing
	        // allow for arbitrary nesting of these tags which is not explicitly ruled out by the spec
	        QStringList tagpath = mi.tpath.split(".");
	        skip = tagpath.contains("desc")  || tagpath.contains("title") || tagpath.contains( "flowRoot");
		if (mi.ttype == "end") continue; // do not output end tags after updating skip
	    }
	}
        if (!skip) {
            svgdata << qp.serialize_markup(mi);
        }
    }
    return svgdata.join("");
}

QImage Utility::RenderSvgToImage(const QString& filepath)
{
    QString svgdata = Utility::ReadUnicodeTextFile(filepath);
    // QtSvg has many issues with desc, title, and flowRoot tags
    svgdata = Utility::FixupSvgForRendering(svgdata);
    QSvgRenderer renderer;
    renderer.load(svgdata.toUtf8());
    QSize sz = renderer.defaultSize();
    QImage svgimage(sz, QImage::Format_ARGB32);
    // **must** fill it with pixels BEFORE trying to render anything
    // transparent fill will not work with light and dark modes: svgimage.fill(qRgba(0,0,0,0));
    svgimage.fill(QColor("white"));
    // was svgimage.fill(qRgba(0,0,0,0));
    QPainter painter(&svgimage);
    renderer.render(&painter);
    return svgimage;

}

QString Utility::UseNFC(const QString& text)
{
    QString txt;
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    if (mainApplication->AlwaysUseNFC()) {
        txt = text.normalized(QString::NormalizationForm_C);
    } else {
        txt = text;
    }
    return txt;
}
