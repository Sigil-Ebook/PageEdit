/************************************************************************
**
**  Copyright (C) 2019-2020 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_main.h"

#include <QtCore/QSignalMapper>
#include <QMap>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QMainWindow>
#include <QMoveEvent>
#include <QCloseEvent>
#include <QResizeEvent>
#include <Viewer.h>
#include <Inspector.h>

#include "ElementIndex.h"

class WebViewEdit;
class Inspector;
class QWebEngineView;
class QVBoxLayout;
class QHBoxLayout;
class QSlider;
class QLabel;
class SelectCharacter;
class SearchToolbar;

const int STATUSBAR_MSG_DISPLAY_TIME = 7000;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString filepath, QWidget *parent = 0);
    ~MainWindow();
    QList<ElementIndex> GetCaretLocation();
    bool IsVisible();
    bool HasFocus();
    bool eventFilter(QObject *object, QEvent *event);
    // void setMathJaxURL(QString mathjaxurl) { m_mathjaxurl = mathjaxurl; };
    void setUserCSSURL(QString usercssurl) { m_usercssurl = usercssurl; }
    void ExtendIconSizes();

    // Zoom Related
    float GetZoomFactor();
    int   ZoomFactorToSliderRange(float zoom_factor);
    float SliderRangeToZoomFactor(int slider_range_value);
    void  ZoomByStep(bool zoom_in);
    void  ZoomByFactor(float new_zoom_factor);

    QString GetSandBoxPath() { return m_SandBoxPath; };

public slots:

    // Zoom Related
    void ZoomIn();
    void ZoomOut();
    void ZoomReset();
    void SliderZoom(int slider_value);
    void UpdateZoomSlider(float new_zoom_factor);
    void UpdateZoomLabel(int slider_value);
    void UpdateZoomLabel(float new_zoom_factor);

    // General Slots
    void DoUpdatePage();
    void UpdatePage(const QString &filename, const QString &source="");
    void RefreshPage();
    void UpdateWindowTitle();
    void ScrollTo(QList<ElementIndex> location);
    void SetZoomFactor(float factor);
    void EmitGoToPreviewLocationRequest();
    void InspectPreviewPage();
    void SelectAllPreview();
    void CopyPreview();
    void ReloadPreview();
    void ShowMessageOnStatusBar(const QString &message = "", int millisecond_duration = STATUSBAR_MSG_DISPLAY_TIME);
    void AboutPageEdit();

    // Navigation Slots
    void EditNext();
    void EditPrev();
    void CBNavigateActivated(int index);
    void LinkClicked(const QUrl &url);
    void LinkReturn();

    // Utility routines
    QString GetCurrentFilePath();
    QStringList GetAllFilePaths(int skip = -1);

    // Mode slots
    void ToggleMode(bool on);

    static const QMap<QString, QString> GetLoadFiltersMap();
    static const QMap<QString, QString> GetSaveFiltersMap();

    // GUI slots
    void sizeMenuIcons();
    void SelectionChanged();
    void CheckHeadingLevel(const QString &element_name);
    void ApplyHeadingToSelection(const QString &heading_type);
    void SetPreserveHeadingAttributes(bool new_state);
    bool Save();
    bool SaveAs();
    void Open();
    void Exit();
    void Undo();
    void Redo();
    void Cut();
    void Copy();
    void SelectAll();
    void Paste();
    void PasteText(const QString& text);
    void PreferencesDialog();
    void SearchOnPage();
    void InsertSpecialCharacter();
    void InsertSGFSectionMarker();
    void InsertBulletedList();
    void InsertNumberedList();
    void InsertId();
    void InsertHyperlink();
    void InsertFileDialog();
    void InsertFiles(const QStringList &selected_files);
    void Bold();
    void Italic();
    void Underline();
    void Strikethrough();
    void Subscript();
    void Superscript();
    void AlignLeft();
    void AlignCenter();
    void AlignRight();
    void AlignJustify();
    void DecreaseIndent();
    void IncreaseIndent();
    void UpdateActionState();
    void ChangeCasing(int casing_mode);
    void printRendered();
    // void ToggleSpellCheck();

    void ApplicationPaletteChanged();

signals:
    void Shown();
    void ZoomFactorChanged(float factor);
    void GoToPreviewLocationRequest();
    void RequestPreviewReload();

    /**
     * Emitted whenever m_WebView wants to open an URL.
     * @param url The URL to open.
     */
    void OpenUrlRequest(const QUrl &url);


protected:
    virtual void hideEvent(QHideEvent* event);
    virtual void showEvent(QShowEvent* event);
    void resizeEvent(QResizeEvent * event);
    void moveEvent(QMoveEvent *event);
    void closeEvent(QCloseEvent * event);
    bool MaybeSaveDialogSaysProceed(bool modified);
    QString GetCleanHtml();
    QString GetSource();
    QString GetHTMLToPaste(const QString & hmtl_snippet);

private:
    void SetupView();
    void SetupFileList(const QString& filepath);
    void SetupNavigationComboBox();
    void LoadSettings();
    void SaveSettings();
    void AllowSaveIfModified();
    void ConnectSignalsToSlots();

    WebViewEdit *m_WebView;
    Inspector *m_Inspector;
    QString m_Filepath;
    bool m_GoToRequestPending;
    bool m_MouseReleaseEventHappened;

    // QString m_mathjaxurl;
    QString m_usercssurl;

    QSignalMapper *m_headingMapper;
    QSignalMapper *m_casingChangeMapper;
    bool m_preserveHeadingAttributes;
    QString m_CurrentFilePath;
    SelectCharacter* m_SelectCharacter;
    QSlider *m_slZoomSlider;
    QLabel *m_lbZoomLabel;
    bool m_updateActionStatePending;
    QByteArray m_LastWindowSize;
    QString m_LastFolderOpen;
    bool m_using_wsprewrap;
    SearchToolbar * m_search;
    QVBoxLayout * m_layout;
    QString m_source;

    QStringList m_SpineList;
    QString m_Base;
    int m_ListPtr;
    bool m_UpdatePageInProgress;
    QStringList m_MediaList;
    QStringList m_MediaKind;
    QString m_MediaBase;
    QString m_SandBoxPath;
    bool m_skipPrintWarnings;

    QList<ElementIndex> m_LastLocation;
    int m_LastPtr;

    Ui::MainWindow ui;
};

#endif // MAINWINDOW_H
