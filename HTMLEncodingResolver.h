/************************************************************************
**
**  Copyright (C) 2016-2024 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef HTMLEncodingResolver_H
#define HTMLEncodingResolver_H

#include <QStringDecoder>

class QString;

class HTMLEncodingResolver
{

public:

    // Accepts a full path to an HTML file.
    // Reads the file, detects the encoding
    // and returns the text converted to Unicode.
    static QString ReadHTMLFile(const QString &fullfilepath);

private:

    // Accepts an HTML stream and tries to determine its encoding;
    // if no encoding is detected, the default codec for this locale is returned.
    // We use this function because Qt's QTextCodec::codecForHtml() function
    // leaves a *lot* to be desired.
    static QStringDecoder GetDecoderForHTML(const QByteArray &raw_text);

    // This function goes through the entire byte array
    // and tries to see whether this is a valid UTF-8 sequence.
    // If it's valid, this is probably a UTF-8 string.
    static bool IsValidUtf8(const QByteArray &string);

    static QByteArray FixupCodePageMapping(const QByteArray& ba);
};


#endif // HTMLEncodingResolver_H
