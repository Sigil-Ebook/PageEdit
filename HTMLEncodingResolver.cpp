/************************************************************************
**
**  Copyright (C) 2016-2024 Kevin B. Hendricks Stratford, Ontario, Canada
**  Copyright (C) 2013      John Schember <john@nachtimwald.com>
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

#include <string>

#include <QFile>
#include <QString>
#include <QRegularExpression>
#include <QDebug>
#include "HTMLEncodingResolver.h"
#include "Utility.h"
#include "pageedit_constants.h"
#include "pageedit_exception.h"

const QString ENCODING_ATTRIBUTE   = "encoding\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')";
const QString CHARSET_ATTRIBUTE    = "charset\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')";
const QString STANDALONE_ATTRIBUTE = "standalone\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')";
const QString VERSION_ATTRIBUTE    = "<\\?xml[^>]*version\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')[^>]*>";

// Accepts a full path to an HTML file.
// Reads the file, detects the encoding
// and returns the text converted to Unicode.
QString HTMLEncodingResolver::ReadHTMLFile(const QString &fullfilepath)
{
    QFile file(fullfilepath);

    // Check if we can open the file
    if (!file.open(QFile::ReadOnly)) {
        std::string msg = file.fileName().toStdString() + ": " + file.errorString().toStdString();
        throw (CannotOpenFile(msg));
    }

    QByteArray data = file.readAll();

    return Utility::ConvertLineEndingsAndNormalize(GetDecoderForHTML(data).decode(data));
}


// Accepts an HTML stream and tries to determine its encoding;
// if no encoding is detected, the default codec for this locale is returned.
// We use this function because Qt's QTextCodec::codecForHtml() function
// leaves a *lot* to be desired.
QStringDecoder HTMLEncodingResolver::GetDecoderForHTML(const QByteArray &raw_text)
{
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;
    unsigned char c4;
    QString text;
    QStringDecoder decoder;

    if (raw_text.length() < 4) {
        return QStringDecoder("UTF-8");
    }

    // Check the BOM if present.
    c1 = raw_text.at(0);
    c2 = raw_text.at(1);
    c3 = raw_text.at(2);
    c4 = raw_text.at(3);
    if (c1 == 0xEF && c2 == 0xBB && c3 == 0xBF) {
        return QStringDecoder("UTF-8");
    } else if (c1 == 0xFF && c2 == 0xFE && c3 == 0 && c4 == 0) {
        return QStringDecoder("UTF-32LE");
    } else if (c1 == 0 && c2 == 0 && c3 == 0xFE && c4 == 0xFF) {
        return QStringDecoder("UTF-32BE");
    } else if (c1 == 0xFE && c2 == 0xFF) {
        return QStringDecoder("UTF-16BE");
    } else if (c1 == 0xFF && c2 == 0xFE) {
        return QStringDecoder("UTF-16LE");
    }

    // Alternating char followed by 0 is typical of utf 16 le without BOM.
    if (c1 != 0 && c2 == 0 && c3 != 0 && c4 == 0) {
        return QStringDecoder("UTF-16LE");
    }

    // Try to find an ecoding specified in the file itself.
    text = Utility::Substring(0, 1024, raw_text);

    // Check if the xml encoding attribute is set.
    QRegularExpression enc_re(ENCODING_ATTRIBUTE);
    QRegularExpressionMatch enc_mo = enc_re.match(text);
    if (enc_mo.hasMatch()) {
        QByteArray ba(enc_mo.captured(1).toLatin1());
        ba = FixupCodePageMapping(ba);
        QStringDecoder decoder = QStringDecoder(ba);
        if (decoder.isValid()) {
            return decoder;
        }
    }

    // Check if the charset is set in the head.
    QRegularExpression char_re(CHARSET_ATTRIBUTE);
    QRegularExpressionMatch char_mo = char_re.match(text);
    if (char_mo.hasMatch()) {
        QByteArray ba(char_mo.captured(1).toLatin1());
        ba = FixupCodePageMapping(ba);
        QStringDecoder decoder = QStringDecoder(ba);
        if (decoder.isValid()) {
            return decoder;
        }
    }

    // See if all characters within this document are utf-8.
    if (IsValidUtf8(raw_text)) {
        return QStringDecoder("UTF-8");
    }

    // Finally, let Qt guess
    return QStringDecoder::decoderForHtml(raw_text);
}


// This function goes through the entire byte array
// and tries to see whether this is a valid UTF-8 sequence.
// If it's valid, this is probably a UTF-8 string.
bool HTMLEncodingResolver::IsValidUtf8(const QByteArray &string)
{
    // This is an implementation of the Perl code written here:
    //   http://www.w3.org/International/questions/qa-forms-utf-8
    //
    // Basically, UTF-8 has a very specific byte-pattern. This function
    // checks if the sent byte-sequence conforms to this pattern.
    // If it does, chances are *very* high that this is UTF-8.
    //
    // This function is written to be fast, not pretty.
    if (string.isNull()) {
        return false;
    }

    int index = 0;

    while (index < string.size()) {
        QByteArray dword = string.mid(index, 4);

        if (dword.size() < 4) {
            dword = dword.leftJustified(4, '\0');
        }

        const unsigned char *bytes = (const unsigned char *) dword.constData();

        // ASCII
        if (bytes[0] == 0x09 ||
            bytes[0] == 0x0A ||
            bytes[0] == 0x0D ||
            (0x20 <= bytes[0] && bytes[0] <= 0x7E)
           ) {
            index += 1;
        }
        // non-overlong 2-byte
        else if ((0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
                 (0x80 <= bytes[1] && bytes[1] <= 0xBF)
                ) {
            index += 2;
        } else if ((bytes[0] == 0xE0                         &&              // excluding overlongs
                    (0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
                   (((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||               // straight 3-byte
                     bytes[0] == 0xEE                         ||
                     bytes[0] == 0xEF) &&
                    (0x80 <= bytes[1] && bytes[1] <= 0xBF)   &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF)) ||
                   (bytes[0] == 0xED                         &&              // excluding surrogates
                    (0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF))
                  ) {
            index += 3;
        } else if ((bytes[0] == 0xF0                         &&              // planes 1-3
                    (0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                    (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
                   ((0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&              // planes 4-15
                    (0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                    (0x80 <= bytes[3] && bytes[3] <= 0xBF)) ||
                   (bytes[0] == 0xF4                         &&            // plane 16
                    (0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
                    (0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
                    (0x80 <= bytes[3] && bytes[3] <= 0xBF))
                  ) {
            index += 4;
        } else {
            return false;
        }
    }

    return true;
}

QByteArray HTMLEncodingResolver::FixupCodePageMapping(const QByteArray& ba)
{
  if (ba == "cp1250" || ba == "CP1250" || ba == "cp-1250" || ba == "CP-1250") return QByteArray("windows-1250");
  if (ba == "cp1251" || ba == "CP1251" || ba == "cp-1251" || ba == "CP-1251") return QByteArray("windows-1251");
  if (ba == "cp1252" || ba == "CP1252" || ba == "cp-1252" || ba == "CP-1252") return QByteArray("windows-1252");
  if (ba == "cp1253" || ba == "CP1253" || ba == "cp-1253" || ba == "CP-1253") return QByteArray("windows-1253");
  if (ba == "cp1254" || ba == "CP1254" || ba == "cp-1254" || ba == "CP-1254") return QByteArray("windows-1254");
  if (ba == "cp1255" || ba == "CP1255" || ba == "cp-1255" || ba == "CP-1255") return QByteArray("windows-1255");
  if (ba == "cp1256" || ba == "CP1256" || ba == "cp-1256" || ba == "CP-1256") return QByteArray("windows-1256");
  if (ba == "cp1257" || ba == "CP1257" || ba == "cp-1257" || ba == "CP-1257") return QByteArray("windows-1257");
  if (ba == "cp1258" || ba == "CP1258" || ba == "cp-1258" || ba == "CP-1258") return QByteArray("windows-1258");
  return QByteArray(ba);
}
