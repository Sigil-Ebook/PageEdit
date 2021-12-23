/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef UIDICTIONARY_H
#define UIDICTIONARY_H

#include <QStringList>
class QString;

class UIDictionary
{

public:

    static QString GetDictionaryPath();

    static QStringList GetUIDictionaries();
};

#endif // UIDICTIONARY_H
