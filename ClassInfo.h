/************************************************************************
**
**  Copyright (C) 2023 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef CLASSINFO_H
#define CLASSINFO_H

#include <QObject>
#include <QStringList>


class ClassInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * Parse the supplied text to identify simple CSS class selectors.
     */
    ClassInfo(const QString &text);

    ~ClassInfo();

    struct CSSSelector {
        QString text;       /* The original text of the complete selector                  */
        bool isGroup;               /* Whether selector was declared in a comma separated group    */
        QString groupText;          /* The text of the selector for this group (same as originalText if not a group) */
        QStringList classNames;     /* The classname(s) (stripped of periods) if a class selector  */
        QStringList elementNames;   /* The element names if any (stripped of any ids/attributes)   */
        int line;                   /* For convenience of navigation, the line# selector is on     */
        int position;               /* The position in the file of the full selector name          */
        int openingBracePos;        /* Location of the opening brace which contains properties     */
        int closingBracePos;        /* Location of the closing brace which contains properties     */

        bool operator<(const CSSSelector &rhs) const {
            return line < rhs.line;
        }
    };

    /**
     * Return selectors subset for only class based CSS declarations.
     */
    QList<CSSSelector *> getClassSelectors();


private:
    void parseCSSSelectors(const QString &text, const int &offsetLines, const int &offsetPos);
    QString replaceBlockComments(const QString &text);

    QList<CSSSelector *> m_CSSSelectors;
    QString m_OriginalText;
};

template<class T>
bool dereferencedLessThan(T *o1, T *o2)
{
    return *o1 < *o2;
}

#endif // CLASSINFO_H

