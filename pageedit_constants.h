/************************************************************************
**
**  Copyright (C) 2019-2020  Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2020  Doug Massay
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
#ifndef PE_CONSTANTS_H
#define PE_CONSTANTS_H

class QString;

extern const float ZOOM_STEP;
extern const float ZOOM_MIN;
extern const float ZOOM_MAX;
extern const float ZOOM_NORMAL;

extern const QString PAGEEDIT_PREFS_DIR;
extern const QString PATH_LIST_DELIM;


#if !defined(_WIN32) && !defined(__APPLE__)
extern const QString pageedit_extra_root;
extern const QString pageedit_share_root;
extern const QString force_sigil_darkmode_palette;
extern const QString force_pageedit_darkmode_palette;
extern const QString mathjax_dir;
#endif

#endif // PE_CONSTANTS_H
