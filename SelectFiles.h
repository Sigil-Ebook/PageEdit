/************************************************************************
**
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012-2013 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013 Dave Heiland
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
#ifndef SELECTFILES_H
#define SELECTFILES_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStringList>

#include "ui_SelectFiles.h"

class QString;
class QWebEngineView;

class SelectFiles : public QDialog
{
    Q_OBJECT

public:
    SelectFiles(QString title, 
                const QStringList &medialist, 
                const QStringList &mediakind, 
                const QString &mediabase, 
                QWidget *parent = 0);

    ~SelectFiles();

    /**
     * The image(s) selected in the dialog.
     *
     * @return The filename of the selected image.
     */
    QStringList SelectedImages();

public slots:
    /**
     * Set the list of image resources to display.
     */
    void SetImages();

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
    /**
     * Displays a given image in the list in the preview area.
     */
    void PreviewLoadComplete(bool);
    void IncreaseThumbnailSize();
    void DecreaseThumbnailSize();

    void ReloadPreview();

    void SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void SelectDefaultImage();

    /**
     * Filters the list of displayed images
     */
    void FilterEditTextChangedSlot(const QString &text);

    void WriteSettings();

    void SplitterMoved(int pos, int index);

private:
    bool IsPreviewLoaded();
    
    void ReadSettings();
    void connectSignalsSlots();

    void SetPreviewImage();

    QStringList m_MediaList;
    QStringList m_MediaKind;
    QString     m_MediaBase;

    QStandardItemModel *m_SelectFilesModel;

    QStandardItem *GetLastSelectedImageItem();
    QString GetLastSelectedImageName();

    bool m_PreviewReady;
    bool m_PreviewLoaded;

    QString m_DefaultSelectedImage;

    int m_ThumbnailSize;

    QListWidgetItem *m_AllItem;
    QListWidgetItem *m_ImageItem;
    QListWidgetItem *m_VideoItem;
    QListWidgetItem *m_AudioItem;

    QWebEngineView *m_WebView;

    Ui::SelectFiles ui;
};

#endif // SELECTFILES_H
