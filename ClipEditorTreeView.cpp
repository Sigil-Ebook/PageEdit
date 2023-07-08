/************************************************************************
**
**  Copyright (C) 2023 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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

#include <QHeaderView>

#include "ClipEditorTreeView.h"

ClipEditorTreeView::ClipEditorTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setDragEnabled(true);
    setAcceptDrops(false);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setAutoScroll(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSortingEnabled(false);
    setTabKeyNavigation(true);
    setAutoExpandDelay(200);
}

ClipEditorTreeView::~ClipEditorTreeView()
{
}

QModelIndex ClipEditorTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (cursorAction == QAbstractItemView::MoveNext) {
        QModelIndex index = currentIndex();

        // Only the first column of a group is editable
        if (!model()->data(index.sibling(index.row(), 0), Qt::UserRole + 1).toBool()) {
            // Move to next column in row if there is one
            if (index.column() < (header()->count() - 1)) {
                return model()->index(index.row(), index.column() + 1, index.parent());
            }
        }

        // Reset to first column in row so that default moveCursor moves it down one
        if (indexBelow(index).isValid()) {
            setCurrentIndex(model()->index(index.row(), 0, index.parent()));
        }
    } else if (cursorAction == QAbstractItemView::MovePrevious) {
        QModelIndex index = currentIndex();

        // Only the first column of a group is editable
        if (!model()->data(index.sibling(index.row(), 0), Qt::UserRole + 1).toBool()) {
            // Move to previous column in row if there is one
            if (index.column() > 0) {
                return model()->index(index.row(), index.column() - 1, index.parent());
            }
        }

        if (indexAbove(index).isValid()) {
            // If row above is a group always reset to first column otherwise last column
            if (model()->data(indexAbove(index).sibling(indexAbove(index).row(), 0), Qt::UserRole + 1).toBool()) {
                setCurrentIndex(model()->index(index.row(), 0, index.parent()));
            } else {
                setCurrentIndex(model()->index(index.row(), header()->count() - 1, index.parent()));
            }
        }
    }

    return QTreeView::moveCursor(cursorAction, modifiers);
}
