/************************************************************************
**
**  Copyright (C) 2023-2024  Kevin B. Hendricks, Stratford, ON Canada
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

#include <QRegularExpression>
#include <QDebug>
#include "Utility.h"
#include "ClassInfo.h"

const int TAB_SPACES_WIDTH = 4;
static const QString DELIMITERS = "}{;";

// Note: CSSProperties and CSSSelectors are simple struct that this code
// created with new and so need to be manually cleaned up to prevent
// large memory leaks

ClassInfo::ClassInfo(const QString &text)
    : m_OriginalText(text)
{
    parseCSSSelectors(text, 0, 0);
}

// Need to manually clean up the Selector List
ClassInfo::~ClassInfo()
{
  foreach(CSSSelector * sp, m_CSSSelectors) {
      if (sp) delete sp;
  } 
}

QList<ClassInfo::CSSSelector *> ClassInfo::getClassSelectors()
{
    QList<ClassInfo::CSSSelector *> selectors;
    foreach(ClassInfo::CSSSelector * cssSelector, m_CSSSelectors) {
        if (cssSelector->classNames.count() > 0) {
            selectors.append(cssSelector);
        }
    }
    return selectors;
}

void ClassInfo::parseCSSSelectors(const QString &text, const int &offsetLines, const int &offsetPos)
{
    QRegularExpression strip_attributes_regex("\\[[^\\]]*\\]");
    QRegularExpression strip_ids_regex("#[^\\s\\.]+");
    // QRegularExpression strip_non_name_chars_regex("[^A-Za-z0-9_\\-\\.:]+");
    QRegularExpression strip_non_name_chars_regex("[^\\w_\\-\\.:]+", QRegularExpression::UseUnicodePropertiesOption);
    QString search_text = replaceBlockComments(text);
    // CSS selectors can be in a myriad of formats... the class based selectors could be:
    //    .c1 / e1.c1 / e1.c1.c2 / e1[class~=c1] / e1#id1.c1 / e1.c1#id1 / .c1, .c2 / ...
    // Then the element based selectors could be:
    //    e1 / e1 > e2 / e1 e2 / e1 + e2 / e1[attribs...] / e1#id1 / e1, e2 / ...
    // Really needs a parser to do this properly, this will only handle the 90% scenarios.

    // Note: selector groups can be sepaparated by line feeds so you can not stop
    // at the beginning of line when searching for the start of a selector

    int pos = 0;
    int open_brace_pos = -1;
    int close_brace_pos = -1;

    while (true) {
        open_brace_pos = search_text.indexOf(QChar('{'), pos);

        if (open_brace_pos < 0) {
            break;
        }

        // Now search backwards until we get a line (or more) containing text .
        bool have_text = false;
        pos = open_brace_pos - 1;

        while ((pos >= 0) && (!DELIMITERS.contains(search_text.at(pos)) || !have_text)) {
            if (search_text.at(pos).isLetter()) {
                have_text = true;
            }
            pos--;
        }
        pos++;
        if (!have_text) {
            // Really badly formed CSS document - try to skip ahead
            pos = open_brace_pos + 1;
            continue;
        }
        close_brace_pos = search_text.indexOf(QChar('}'), open_brace_pos + 1);
        if (close_brace_pos < 0) {
            // Another badly formed scenario - no point in looking further
            break;
        }
        // Skip past leading whitespace/newline.
        while (search_text.at(pos).isSpace()) {
            pos++;
        }

        int line = search_text.left(pos + 1).count(QChar('\n')) + 1;
        QString selector_text = search_text.mid(pos, open_brace_pos - pos).trimmed();
        // Handle case of a selector group containing multiple declarations
        QStringList matches = selector_text.split(QChar(','), Qt::SkipEmptyParts);
        foreach(QString match, matches) {
            CSSSelector *selector = new CSSSelector();
            selector->text = selector_text;
            selector->groupText = match.trimmed();
            selector->position = pos + offsetPos;
            selector->line = line + offsetLines;
            selector->isGroup = matches.length() > 1;
            selector->openingBracePos = open_brace_pos + offsetPos;
            selector->closingBracePos = close_brace_pos + offsetPos;
            // Need to parse our selector text to determine what sort of selector it contains.
            // First strip out any attributes and then identifiers
            match.replace(strip_attributes_regex, "");
            match.replace(strip_ids_regex, "");
            // Also replace any other characters like > or + not of interest
            match.replace(strip_non_name_chars_regex, QChar(' '));
            // Now break it down into the element components
            QStringList elements = match.trimmed().split(QChar(' '), Qt::SkipEmptyParts);
            foreach(QString element, elements) {
                if (element.contains(QChar('.'))) {
                    QStringList parts = element.split('.');
                    if (!parts.at(0).isEmpty()) {
                        selector->elementNames.append(parts.at(0));
                    }
                    for (int i = 1; i < parts.length(); i++) {
                        selector->classNames.append(parts.at(i));
                    }
                } else {
                    selector->elementNames.append(element);
                }
            }
            m_CSSSelectors.append(selector);
        }
        pos = open_brace_pos + 1;
    }
}

QString ClassInfo::replaceBlockComments(const QString &text)
{
    // We take a copy of the text and remove all block comments from it.
    // However we must be careful to replace with spaces/keep line feeds
    // so that do not corrupt the position information used by the parser.
    QString new_text(text);
    QRegularExpression comment_search("/\\*.*\\*/", QRegularExpression::InvertedGreedinessOption|QRegularExpression::DotMatchesEverythingOption);
    int start = 0;
    int comment_index;

    while (true) {
        int comment_len = 0;
        comment_index = -1;
        QRegularExpressionMatch match = comment_search.match(new_text, start);
        if (match.hasMatch()) {
            comment_index = match.capturedStart();
            comment_len = match.capturedLength();
        }
        if (comment_index < 0) {
            break;
        }
        QString match_text = new_text.mid(comment_index, comment_len);
        match_text.replace(QRegularExpression("[^\r\n]"), QChar(' '));
        new_text.remove(comment_index, match_text.length());
        new_text.insert(comment_index, match_text);
        // Prepare for the next comment.
        start = comment_index + comment_len;
        if (start >= new_text.length() - 2) {
            break;
        }
    }
    return new_text;
}
