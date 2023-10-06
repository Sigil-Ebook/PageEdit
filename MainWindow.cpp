/************************************************************************
**
**  Copyright (C) 2019-2023 Kevin Hendricks, Doug Massay
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

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDesktopServices>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QToolBar>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStatusBar>
#include <QThread>
#include <QToolButton>
#include <QSlider>
#include <QTimer>
#include <QFileInfo>
#include <QDebug>

#include "Inspector.h"
#include "SettingsStore.h"
#include "Utility.h"
#include "ClipEditor.h"
#include "ClipsWindow.h"
#include "WebViewEdit.h"
#include "SelectCharacter.h"
#include "SelectFiles.h"
#include "SelectHyperlink.h"
#include "SelectId.h"
#include "Preferences.h"
#include "GumboInterface.h"
#include "HTMLEncodingResolver.h"
#include "SearchToolbar.h"
#include "OPFReader.h"
#include "MainApplication.h"
#include "MainWindow.h"

#define DBG if(0)

static const QString SETTINGS_GROUP = "mainwindow";

static const QString CUSTOM_WEBVIEW_STYLE_FILENAME = "custom_webview_style.css";

static const QString EDIT_WITH_PRE_WRAP  = ":not(html):not(body) { white-space: pre-wrap; }";

static const QString BREAK_TAG_INSERT    = "<hr class=\"sigil_split_marker\" />";

static const QString DEFAULT_FILENAME = "untitled.xhtml";

static const QStringList HEADERTAGS = QStringList() << "h1" << "h2" << "h3" << "h4" << "h5" << "h6";

static const QStringList DARKCSSLINKS = QStringList() << "qrc:///dark/mac_dark_scrollbar.css" 
                                                      << "qrc:///dark/win_dark_scrollbar.css" 
                                                      << "qrc:///dark/lin_dark_scrollbar.css";

const float ZOOM_STEP               = 0.1f;
const float ZOOM_MIN                = 0.09f;
const float ZOOM_MAX                = 5.0f;
const float ZOOM_NORMAL             = 1.0f;
static const int ZOOM_SLIDER_MIN    = 0;
static const int ZOOM_SLIDER_MAX    = 1000;
static const int ZOOM_SLIDER_MIDDLE = 500;
static const int ZOOM_SLIDER_WIDTH  = 140;


MainWindow::MainWindow(QString filepath, QString spineno, QWidget *parent)
    :
    QMainWindow(parent),
    m_WebView(new WebViewEdit(this)),
    m_Inspector(new Inspector(this)),
    m_Filepath(QString()),
    m_GoToRequestPending(false),
    m_headingMapper(new QSignalMapper(this)),
    m_casingChangeMapper(new QSignalMapper(this)),
    m_preserveHeadingAttributes(false),
    m_SelectCharacter(new SelectCharacter(this)),
    m_slZoomSlider(NULL),
    m_lbZoomLabel(NULL),
    m_updateActionStatePending(false),
    m_LastWindowSize(QByteArray()),
    m_LastFolderOpen(QString()),
    m_using_wsprewrap(false),
    m_search(NULL),
    m_layout(NULL),
    m_SpineList(QStringList()),
    m_Base(QString()),
    m_ListPtr(-1),
    m_UpdatePageInProgress(false),
    m_MediaList(QStringList()),
    m_MediaKind(QStringList()),
    m_MediaBase(QString()),
    m_skipPrintWarnings(false),
    m_skipPrintPreview(false),
    m_WebViewPrinter(new WebViewPrinter(this)),
    m_Clips(nullptr),
    m_ClipEditor(new ClipEditor(this)),
    m_LastPtr(-1)
{
    ui.setupUi(this);
    // initialize the toolbar UI clip actions
    foreach(QAction* clipaction, ui.toolBarClips->actions()) {
        if (!clipaction->isSeparator()) {
            QString strIndex = clipaction->objectName();
            strIndex.replace(QString("actionClip"), QString(""));
            clipaction->setData(strIndex.toInt());
            m_clactions.append(clipaction);
        }
    }
    UpdateClipsUI();
    SetupView();
    // start up in edit mode unless the user changes it
    ui.actionMode->setChecked(true);
    LoadSettings();
    ConnectSignalsToSlots();
    SetupFileList(filepath, spineno);
    SetupNavigationComboBox();
    m_ClipEditor->SetCSSList(m_CSSList);
    QTimer::singleShot(200, this, SLOT(DoUpdatePage()));
}

MainWindow::~MainWindow()
{
    if (m_WebView) {
        delete m_WebView;
        m_WebView = nullptr;
    }

    if (m_Inspector) {
        if (m_Inspector->isVisible()) {
            m_Inspector->StopInspection();
            m_Inspector->close();
        }
        delete m_Inspector;
        m_Inspector = nullptr;
    }
}

void MainWindow::ApplicationPaletteChanged()
{
    // qDebug() << "ApplicationPaletteChanged";
    RefreshPage();
}

// Navigation related routines

// initializes m_Base, m_SpineList, m_ListPtr, and m_CurrentFilePath
// Also sets m_SandBoxPath to limit file: urls
void MainWindow::SetupFileList(const QString &filepath, const QString &spineno)
{
    m_CurrentFilePath = "";
    ui.actionNext->setEnabled(false);
    ui.actionPrev->setEnabled(false);
    if (filepath.isEmpty()) return;
    QFileInfo fi(filepath);
    if (!fi.exists() || !fi.isReadable()) return;
    m_ListPtr = -1;
    if (fi.suffix() == "opf") {
        m_ListPtr = 0;
        OPFReader opfrdr;
        opfrdr.parseOPF(filepath);
        QStringList spine_files = opfrdr.GetSpineFilePathList();
        if (!spineno.isEmpty()) {
            bool okay;
            int val = spineno.toInt(&okay);
            if ((val >= 0) && (val < spine_files.size())) m_ListPtr = val;
        }
        m_Base = Utility::longestCommonPath(spine_files, "/");
        foreach(QString sf, spine_files) {
            m_SpineList << sf.right(sf.length()-m_Base.length());
        }
        // now collect a list of media file and kind
        QStringList media_list;
        m_MediaKind.clear();
        QStringList audiolist = opfrdr.GetAudioFilePathList();
        foreach(QString filepath, audiolist) {
            media_list << filepath;
            m_MediaKind << "audio";
        }
        QStringList videolist = opfrdr.GetVideoFilePathList();
        foreach(QString filepath, videolist) {
            media_list << filepath;
            m_MediaKind << "video";
        }
        QStringList imagelist = opfrdr.GetImageFilePathList();
        foreach(QString filepath, imagelist) {
            media_list << filepath;
            m_MediaKind << "image";
        }
        QStringList svglist = opfrdr.GetSVGFilePathList();
        foreach(QString filepath, svglist) {
            media_list << filepath;
            m_MediaKind << "svgimage";
        }
        m_CSSList = opfrdr.GetCSSFilePathList();
        m_MediaBase = Utility::longestCommonPath(media_list, "/");
        m_MediaList.clear();
        foreach(QString mf, media_list) {
            m_MediaList << mf.right(mf.length()-m_MediaBase.length());
        }
        // finally determine the sandbox to play in
        QStringList manifestlist = opfrdr.GetManifestFilePathList();
        m_SandBoxPath = Utility::longestCommonPath(manifestlist, "/");
        if (m_SandBoxPath == "/") m_SandBoxPath = m_Base;

    } else {
        // note longestCommonPath always ends with "/" but fi.absolutePath() does not
        m_Base = fi.absolutePath()+ "/";
        m_SpineList << fi.fileName();
        m_ListPtr = 0;
        // FIXME: how should we determine an appropriate sandbox for this case?
        // For Now: limit to the directory holding this file and its parent if not root
        m_SandBoxPath = m_Base;
        QDir sb(m_Base);
        if (sb.cdUp()) {
            if (!sb.isRoot()) {
                m_SandBoxPath = sb.absolutePath();
            }
        }
    }
    // enable or disable InsertFile based on if Media are present
    ui.actionInsertFile->setEnabled(!m_MediaList.isEmpty());
    m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
    if (m_SpineList.length() > 1) {
        ui.actionNext->setEnabled(true);
        ui.actionPrev->setEnabled(true);
    }
    DBG qDebug() << "in SetupFileList" << m_CurrentFilePath;
}

void MainWindow::SetupNavigationComboBox()
{
    ui.cbNavigate->clear();
    ui.cbNavigate->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    ui.cbNavigate->addItems(m_SpineList);
    ui.cbNavigate->setCurrentIndex(m_ListPtr);
}

void MainWindow::CBNavigateActivated(int index)
{
    if (m_UpdatePageInProgress) {
        ui.cbNavigate->setCurrentIndex(m_ListPtr);
        return;
    }
    
    if ((index > -1) && (index != m_ListPtr)) { 
        if (AllowSaveIfModified()) {
            m_ListPtr = index;
            m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
            UpdatePage(m_CurrentFilePath);
        }
    }
}

void MainWindow::EditNext()
{
    if (m_UpdatePageInProgress) return;
    int n = m_SpineList.length();
    if (n > 1) {
        if (AllowSaveIfModified()) {
            m_ListPtr++;
            if (m_ListPtr >= n) m_ListPtr = 0;
            m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
            ui.cbNavigate->setCurrentIndex(m_ListPtr);
            UpdatePage(m_CurrentFilePath);
        }
    }
}

void MainWindow::EditPrev()
{
    if (m_UpdatePageInProgress) return;
    int n = m_SpineList.length();
    if (n > 1) {
        if (AllowSaveIfModified()) {
            m_ListPtr--;
            if (m_ListPtr < 0) m_ListPtr = n - 1;
            m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
            ui.cbNavigate->setCurrentIndex(m_ListPtr);
            UpdatePage(m_CurrentFilePath);
        }
    }
}

QString MainWindow::GetCurrentFilePath()
{
    QString res;
    if (m_ListPtr != -1) {
        res = m_SpineList.at(m_ListPtr);
    }
    return res;
}

QStringList MainWindow::GetAllFilePaths(int skip)
{
    QStringList res;
    int i = 0;
    foreach(QString apath, m_SpineList) {
        if (skip != i) {
            res << apath;
        }
        i++;
    }
    return res;
}

// Mode setting
void MainWindow::ToggleMode(bool edit_mode)
{
    m_WebView->SetDocumentEditable(edit_mode);
    UpdateWindowTitle();
}

// Zoom Support Related Routines

void MainWindow::ZoomIn()
{
    ZoomByStep(true);
}

void MainWindow::ZoomOut()
{
    ZoomByStep(false);
}

void MainWindow::ZoomReset()
{
    ZoomByFactor(ZOOM_NORMAL);
}

float MainWindow::GetZoomFactor()
{
    return m_WebView->GetZoomFactor();
}

void MainWindow::ZoomByStep(bool zoom_in)
{
    // We use a negative zoom stepping if we are zooming *out*                                                     
    float zoom_stepping       = zoom_in ? ZOOM_STEP : - ZOOM_STEP;
    // If we are zooming in, we round UP;                                                                          
    // on zoom out, we round DOWN.                                                                                 
    float rounding_helper     = zoom_in ? 0.05f : - 0.05f;
    float current_zoom_factor = GetZoomFactor();
    float rounded_zoom_factor = Utility::RoundToOneDecimal(current_zoom_factor + rounding_helper);

    // If the rounded value is nearly the same as the original value,                                              
    // then the original was rounded to begin with and so we                                                       
    // add the zoom increment                                                                                      
    if (qAbs(current_zoom_factor - rounded_zoom_factor) < 0.01f) {
       ZoomByFactor(Utility::RoundToOneDecimal(current_zoom_factor + zoom_stepping));
    }
    // ...otherwise we first zoom to the rounded value                                                             
    else {
        ZoomByFactor(rounded_zoom_factor);
    }
}

void MainWindow::ZoomByFactor(float new_zoom_factor)
{
    if (new_zoom_factor > ZOOM_MAX || new_zoom_factor < ZOOM_MIN) {
        return;
    }

    m_WebView->SetZoomFactor(new_zoom_factor);
}

void MainWindow::SliderZoom(int slider_value)
{
    float new_zoom_factor     = SliderRangeToZoomFactor(slider_value);
    float current_zoom_factor = GetZoomFactor();

    // We try to prevent infinite loops...                                                                          
    if (!qFuzzyCompare(new_zoom_factor, current_zoom_factor)) {
        ZoomByFactor(new_zoom_factor);
    }
}

void MainWindow::UpdateZoomSlider(float new_zoom_factor)
{
    m_slZoomSlider->setValue(ZoomFactorToSliderRange(new_zoom_factor));
}

void MainWindow::UpdateZoomLabel(int slider_value)
{
    float zoom_factor = SliderRangeToZoomFactor(slider_value);
    UpdateZoomLabel(zoom_factor);
}

void MainWindow::UpdateZoomLabel(float new_zoom_factor)
{
    m_lbZoomLabel->setText(QString("%1% ").arg(qRound(new_zoom_factor * 100)));
}

int MainWindow::ZoomFactorToSliderRange(float zoom_factor)
{
    // We want a precise value for the 100% zoom,                                                                   
    // so we pick up all float values near it.                                                                      
    if (qFuzzyCompare(zoom_factor, ZOOM_NORMAL)) {
        return ZOOM_SLIDER_MIDDLE;
    }

    // We actually use two ranges: one for the below 100% zoom,                                                     
    // and one for the above 100%. This is so that the 100% mark                                                    
    // rests in the middle of the slider.                                                                           
    if (zoom_factor < ZOOM_NORMAL) {
        double range            = ZOOM_NORMAL - ZOOM_MIN;
        double normalized_value = zoom_factor - ZOOM_MIN;
        double range_proportion = normalized_value / range;
        return ZOOM_SLIDER_MIN + qRound(range_proportion * (ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN));
    } 
    double range            = ZOOM_MAX - ZOOM_NORMAL;
    double normalized_value = zoom_factor - ZOOM_NORMAL;
    double range_proportion = normalized_value / range;
    return ZOOM_SLIDER_MIDDLE + qRound(range_proportion * ZOOM_SLIDER_MIDDLE);
}

float MainWindow::SliderRangeToZoomFactor(int slider_range_value)
{
    // We want a precise value for the 100% zoom                                                                   
    if (slider_range_value == ZOOM_SLIDER_MIDDLE) {
        return ZOOM_NORMAL;
    }

    // We actually use two ranges: one for the below 100% zoom,                                                    
    // and one for the above 100%. This is so that the 100% mark                                                   
    // rests in the middle of the slider.                                                                          
    if (slider_range_value < ZOOM_SLIDER_MIDDLE) {
        double range            = ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN;
        double normalized_value = slider_range_value - ZOOM_SLIDER_MIN;
        double range_proportion = normalized_value / range;
        return ZOOM_MIN + range_proportion * (ZOOM_NORMAL - ZOOM_MIN);
    }
    double range            = ZOOM_SLIDER_MAX - ZOOM_SLIDER_MIDDLE;
    double normalized_value = slider_range_value - ZOOM_SLIDER_MIDDLE;
    double range_proportion = normalized_value / range;
    return ZOOM_NORMAL + range_proportion * (ZOOM_MAX - ZOOM_NORMAL);
}

// End of Zoom related routines

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // Workaround for Qt 4.8 bug - see WriteSettings() for details.                                             
    if (!isMaximized()) {
        m_LastWindowSize = saveGeometry();
    }
    // Update self normally
    QMainWindow::resizeEvent(event);
    UpdateWindowTitle();
}

void MainWindow::hideEvent(QHideEvent * event)
{
    if (m_Inspector) {
        m_Inspector->StopInspection();
        m_Inspector->close();
    }

    if ((m_WebView) && m_WebView->isVisible()) {
        m_WebView->hide();
    }
}

void MainWindow::showEvent(QShowEvent * event)
{
    // perform the show for all children of this widget
    if ((m_WebView) && !m_WebView->isVisible()) {
        m_WebView->show();
    }

    QMainWindow::showEvent(event);
    raise();
    emit Shown();
}

bool MainWindow::IsVisible()
{
    return m_WebView->isVisible();
}

bool MainWindow::HasFocus()
{
    if (!m_WebView->isVisible()) {
        return false;
    }
    return m_WebView->hasFocus();
}

void MainWindow::SetupView()
{
    // QWebEngineView events are routed to their parent
    m_WebView->installEventFilter(this);

#if 1 // !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // this may be needed by all platforms in the future
    QWidget * fp = m_WebView->focusProxy();
    if (fp) fp->installEventFilter(this);
#endif

    QApplication::setOverrideCursor(Qt::WaitCursor);
    setAttribute(Qt::WA_DeleteOnClose);
    
    QFrame *frame = new QFrame(this);
    m_layout = new QVBoxLayout(frame);
    frame->setLayout(m_layout);
    m_layout->addWidget(m_WebView);
    m_layout->setContentsMargins(0, 0, 0, 0);
    frame->setObjectName("PrimaryFrame");
    setCentralWidget(frame);
    m_Inspector->setObjectName("Inspector");
    addDockWidget(Qt::RightDockWidgetArea, m_Inspector);
    m_Inspector->hide();
    m_Clips = new ClipsWindow(this);
    m_Clips->setObjectName("ClipsWindow");
    addDockWidget(Qt::LeftDockWidgetArea, m_Clips);
    m_Clips->hide();

    // Creating the zoom controls in the status bar
    m_slZoomSlider = new QSlider(Qt::Horizontal, statusBar());
    m_slZoomSlider->setTracking(false);
    m_slZoomSlider->setTickInterval(ZOOM_SLIDER_MIDDLE);
    m_slZoomSlider->setTickPosition(QSlider::TicksBelow);
    m_slZoomSlider->setFixedWidth(ZOOM_SLIDER_WIDTH);
    m_slZoomSlider->setMinimum(ZOOM_SLIDER_MIN);
    m_slZoomSlider->setMaximum(ZOOM_SLIDER_MAX);
    m_slZoomSlider->setValue(ZOOM_SLIDER_MIDDLE);
    QToolButton *zoom_out = new QToolButton(statusBar());
    zoom_out->setDefaultAction(ui.actionZoomOut);
    QToolButton *zoom_in = new QToolButton(statusBar());
    zoom_in->setDefaultAction(ui.actionZoomIn);
    m_lbZoomLabel = new QLabel(QString("100% "), statusBar());
    statusBar()->addPermanentWidget(m_lbZoomLabel);
    statusBar()->addPermanentWidget(zoom_out);
    statusBar()->addPermanentWidget(m_slZoomSlider);
    statusBar()->addPermanentWidget(zoom_in);

    // Handle special case of two different icons for one action
    QIcon icon = ui.actionMode->icon();
    icon.addFile(QString::fromUtf8(":/icons/mode-preview.svg"), QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(QString::fromUtf8(":/icons/mode-edit.svg"), QSize(), QIcon::Normal, QIcon::On);
    ui.actionMode->setIcon(icon);
    
    // Headings QToolButton
    ui.tbHeadings->setPopupMode(QToolButton::InstantPopup);

    // Preferences and About
    ui.actionPreferences->setMenuRole(QAction::PreferencesRole);
    ui.actionPreferences->setEnabled(true);

    ui.actionAbout->setMenuRole(QAction::AboutRole);
    ui.actionAbout->setEnabled(true);

    ui.actionOpen->setEnabled(true);
    ui.actionSave->setEnabled(true);
    ui.actionPrint->setEnabled(true);
    ui.actionExit->setEnabled(true);
    
    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
    ui.actionCut->setEnabled(true);
    ui.actionCopy->setEnabled(true);
    ui.actionPaste->setEnabled(true);
    ui.actionSelectAll->setEnabled(true);

    ui.actionInsertSpecialCharacter->setEnabled(true);
    ui.actionInsertSGFSectionMarker->setEnabled(true);
    ui.actionInsertBulletedList ->setEnabled(true);
    ui.actionInsertNumberedList ->setEnabled(true);
    ui.actionInsertId->setEnabled(true);
    ui.actionInsertHyperlink->setEnabled(true);
    ui.actionInsertFile->setEnabled(!m_MediaList.isEmpty());

    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);

    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);
    ui.actionSubscript    ->setEnabled(true);
    ui.actionSuperscript  ->setEnabled(true);

    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingTitlecase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);

    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
    ui.actionDecreaseIndent->setEnabled(true);
    ui.actionIncreaseIndent->setEnabled(true);

    ui.actionZoomIn->setEnabled(true);
    ui.actionZoomOut->setEnabled(true);
    ui.actionZoomReset->setEnabled(true);

    ui.actionInspect->setEnabled(true);

    sizeMenuIcons();

    m_WebView->Zoom();
    UpdateZoomSlider(GetZoomFactor());
    UpdateZoomLabel(GetZoomFactor());

    QApplication::restoreOverrideCursor();
}

void MainWindow::SelectionChanged()
{
    if (!m_updateActionStatePending) {
        m_updateActionStatePending = true;
        QTimer::singleShot(200, this, SLOT(UpdateActionState()));
    }
}

void MainWindow::UpdateActionState() {
    if (!m_WebView->hasSelection()) {
        ui.actionBold->setChecked(false);
        ui.actionItalic->setChecked(false);
        ui.actionStrikethrough->setChecked(false);
        ui.actionUnderline->setChecked(false);
        ui.actionSubscript->setChecked(false);
        ui.actionSuperscript->setChecked(false);

        ui.actionCasingLowercase->setEnabled(false);
        ui.actionCasingUppercase->setEnabled(false);
        ui.actionCasingTitlecase->setEnabled(false);
        ui.actionCasingCapitalize->setEnabled(false);

        ui.actionHeading1->setChecked(false);
        ui.actionHeading2->setChecked(false);
        ui.actionHeading3->setChecked(false);
        ui.actionHeading4->setChecked(false);
        ui.actionHeading5->setChecked(false);
        ui.actionHeading6->setChecked(false);
        ui.actionHeadingNormal->setChecked(false);
    } else {
        ui.actionBold->setChecked(m_WebView->QueryCommandState("bold"));
        ui.actionItalic->setChecked(m_WebView->QueryCommandState("italic"));
        ui.actionStrikethrough->setChecked(m_WebView->QueryCommandState("strikeThrough"));
        ui.actionUnderline->setChecked(m_WebView->QueryCommandState("underline"));
        ui.actionSubscript->setChecked(m_WebView->QueryCommandState("subscript"));
        ui.actionSuperscript->setChecked(m_WebView->QueryCommandState("superscript"));

        ui.actionCasingLowercase->setEnabled(true);
        ui.actionCasingUppercase->setEnabled(true);
        ui.actionCasingTitlecase->setEnabled(true);
        ui.actionCasingCapitalize->setEnabled(true);

        CheckHeadingLevel(m_WebView->GetCaretElementName());
    }
    m_updateActionStatePending = false;
}


void MainWindow::ChangeCasing(int casing_mode)
{
    Utility::Casing casing;
    switch (casing_mode) {
        case Utility::Casing_Lowercase: {
            casing = Utility::Casing_Lowercase;
            break;
            }
        case Utility::Casing_Uppercase: {
            casing = Utility::Casing_Uppercase;
            break;
            }
        case Utility::Casing_Titlecase: {
            casing = Utility::Casing_Titlecase;
            break;
            }
        case Utility::Casing_Capitalize: {
            casing = Utility::Casing_Capitalize;
            break;
            }
        default:
            return;
    }
    m_WebView->ApplyCaseChangeToSelection(casing);
}


void MainWindow::CheckHeadingLevel(const QString &element_name)
{
    ui.actionHeading1->setChecked(false);
    ui.actionHeading2->setChecked(false);
    ui.actionHeading3->setChecked(false);
    ui.actionHeading4->setChecked(false);
    ui.actionHeading5->setChecked(false);
    ui.actionHeading6->setChecked(false);
    ui.actionHeadingNormal->setChecked(false);

    if (!element_name.isEmpty()) {
        if ((element_name[ 0 ].toLower() == QChar('h')) && (element_name[ 1 ].isDigit())) {
            QString heading_name = QString(element_name[ 1 ]);

            if (heading_name == "1") {
                ui.actionHeading1->setChecked(true);
            } else if (heading_name == "2") {
                ui.actionHeading2->setChecked(true);
            } else if (heading_name == "3") {
                ui.actionHeading3->setChecked(true);
            } else if (heading_name == "4") {
                ui.actionHeading4->setChecked(true);
            } else if (heading_name == "5") {
                ui.actionHeading5->setChecked(true);
            } else if (heading_name == "6") {
                ui.actionHeading6->setChecked(true);
            }
        } else {
            ui.actionHeadingNormal->setChecked(true);
        }
    }
}

void MainWindow::DoUpdatePage()
{
  if (!m_CurrentFilePath.isEmpty()) {
      QFileInfo fi(m_CurrentFilePath);
      if (fi.exists() && fi.isReadable()) {
          ui.actionMode->setChecked(true);
          UpdatePage(m_CurrentFilePath);
      }
  }
}

void MainWindow::UpdatePage(const QString &filename_url, const QString &source)
{
    m_UpdatePageInProgress = true;
    QString text;
    QString file_path = filename_url;
    if (!source.isEmpty()) {
        text = source;
    } else { 
        try {
            // This will read in the data and properly convert to unicode
            // from whatever encoding it is in now
            text = HTMLEncodingResolver::ReadHTMLFile(filename_url);

            // This will convert all html to xhtml and remove any 
            // improper xml header and add the proper xml header
            GumboInterface gi(text, "any_version");
            text = gi.getxhtml();

        } catch (std::exception &e) {
            Utility::DisplayStdErrorDialog(tr("File load failed"), e.what());
            text = "<html><head><title></title></head><body><h1>" + tr("File Load Failed") + "</h1></body></html>";
            file_path = "";
            m_CurrentFilePath = "";
        }
    }

    //if isDarkMode is set, inject a local style in head
    SettingsStore settings;
    if (Utility::IsDarkMode() && settings.previewDark()) {
        text = Utility::AddDarkCSS(text);
    }

    SettingsStore ss;
    // to prevent the WebEngine from inserting extraneous non-breaking space characters
    // during editing, the official editing api says we should set white-space:pre-wrap
    // on the elements we want to edit.  In our case this is just about everything
    m_using_wsprewrap = ss.useWSPreWrap();
    if (ss.useWSPreWrap()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_editstyle = "<style type=\"text/css\">" + EDIT_WITH_PRE_WRAP + "</style>"; 
            text.insert(endheadpos, inject_editstyle);
        }
    }

    // If the user has set a default stylesheet inject it
    if (!m_usercssurl.isEmpty()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_userstyles = 
              "<link rel=\"stylesheet\" type=\"text/css\" "
              "href=\"" + m_usercssurl + "\" />";
            DBG qDebug() << "WebView injecting stylesheet: " << inject_userstyles;
            text.insert(endheadpos, inject_userstyles);
        }
    }

#if 0
    // If this page uses mathml tags, inject a polyfill
    // MathJax.js so that the mathml appears in the WebView Window
    QRegularExpression mathused("<\\s*math [^>]*>");
    QRegularExpressionMatch mo = mathused.match(text);
    if (mo.hasMatch()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_mathjax = 
              "<script type=\"text/javascript\" async=\"async\" "
              "src=\"" + m_mathjaxurl + "\"></script>\n";
            text.insert(endheadpos, inject_mathjax);
        }
    }
#endif

    m_Filepath = file_path;
    m_WebView->CustomSetDocument(file_path, text);

    // this next bit is allowing javascript to run before
    // the page is finished loading somehow? 
    // but we explicitly prevent that

    // Wait until the preview is loaded before moving cursor.
    while (!m_WebView->IsLoadingFinished()) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }
    
    if (!m_WebView->WasLoadOkay()) qDebug() << "WV loadFinished with okay set to false!";
 
    DBG qDebug() << "WebViewWindow UpdatePage load is Finished";

    UpdateWindowTitle();
    m_WebView->show();
    m_WebView->GrabFocus();

    // now set the mode: edit or preview
    ToggleMode(ui.actionMode->isChecked());
    QTimer::singleShot(100, this, SLOT(SetInitialSource()));
}

void MainWindow::SetInitialSource()
{
    m_source = GetSource();
    m_UpdatePageInProgress = false;
}

void MainWindow::ScrollTo(QList<ElementIndex> location)
{
    DBG qDebug() << "received a WebViewWindow ScrollTo event";
    if (!m_WebView->isVisible()) {
        return;
    }
    m_WebView->StoreCaretLocationUpdate(location);
    m_WebView->ExecuteCaretUpdate();
}

void MainWindow::UpdateWindowTitle()
{
    if ((m_WebView) && m_WebView->isVisible()) {
        int height = m_WebView->height();
        int width = m_WebView->width();
        QString mode = "-- " + tr("mode: Preview") + " --";
        if (ui.actionMode->isChecked()) {
            mode = "-- " + tr("mode: Edit") + " --";
        }
        setWindowTitle("PageEdit " + mode + " (" + QString::number(width) + "x" + QString::number(height) + ")");
    }
}

QList<ElementIndex> MainWindow::GetCaretLocation()
{
    DBG qDebug() << "WebView in GetCaretLocation";
    QList<ElementIndex> hierarchy = m_WebView->GetCaretLocation();
    // foreach(ElementIndex ei, hierarchy) {
    //     qDebug() << "name: " << ei.name << " index: " << ei.index;
    // }    
    return hierarchy;
}

void MainWindow::SetZoomFactor(float factor)
{
    m_WebView->SetZoomFactor(factor);
}

void MainWindow::EmitGoToPreviewLocationRequest()
{
    DBG qDebug() << "EmitGoToPreviewLocationRequest request: " << m_GoToRequestPending;
    if (m_GoToRequestPending) {
        m_GoToRequestPending = false;
        emit GoToPreviewLocationRequest();
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
      case QEvent::ChildAdded:
          if (object == m_WebView) {
              DBG qDebug() << "child add event";
              const QChildEvent *childEvent(static_cast<QChildEvent*>(event));
              if (childEvent->child()) {
                  childEvent->child()->installEventFilter(this);
              }
          }
          break;
      case QEvent::MouseButtonPress:
        {
          DBG qDebug() << "Preview mouse button press event " << object;
          const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
          if (mouseEvent) {
              if (mouseEvent->button() == Qt::LeftButton) {
                  DBG qDebug() << "Detected Left Mouse Button Press Event";
                  QString hoverurl = m_WebView->GetHoverUrl();
                  DBG qDebug() << "hover url is: " << hoverurl;
                  if (!hoverurl.isEmpty() && !ui.actionMode->isChecked()) {
                      // we are taking a link so save the current location
                      m_LastPtr = m_ListPtr;
                      m_LastLocation = m_WebView->GetCaretLocation();
                  }
              }
              if (mouseEvent->button() == Qt::RightButton) {
                  DBG qDebug() << "Detected Right Mouse Button Press Event";
              }
          }
        }
        break;
      case QEvent::MouseButtonRelease:
        {
          DBG qDebug() << "Preview mouse button release event " << object;
          const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
          if (mouseEvent) {
              if (mouseEvent->button() == Qt::LeftButton) {
                  DBG qDebug() << "Detected Left Mouse Button Release Event";
              }
              if (mouseEvent->button() == Qt::RightButton) {
                  DBG qDebug() << "Detected Right Mouse Button Release Event";
              }
          }
        }
        break;
      case QEvent::Resize:
        {
          if (object == m_WebView) {
              const QResizeEvent *resizeEvent(static_cast<QResizeEvent*>(event));
              if (resizeEvent) {
                  DBG qDebug() << "Detected ResizeEvent: " << resizeEvent->oldSize() << resizeEvent->size();
                  QTimer::singleShot(100, this, SLOT(UpdateWindowTitle()));
              }
          }
        }
        break;
      case QEvent::KeyPress:
        {
            // Assume any key presses are directed at WebEngineView via the delegate
        }
        break;
      default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void MainWindow::LinkReturn()
{
    // requires the Preview Mode to function
    if (ui.actionMode->isChecked()) return;

    // return to last location before link
    if ((m_LastPtr != -1) && !m_LastLocation.isEmpty()) {
        if (m_LastPtr != m_ListPtr) {
            if (AllowSaveIfModified()) {
                m_ListPtr = m_LastPtr;
                ui.cbNavigate->setCurrentIndex(m_ListPtr);
                m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
                UpdatePage(m_CurrentFilePath);
            }
        }
        ScrollTo(m_LastLocation);
    }
    m_LastPtr = -1;
    m_LastLocation.clear();
}

void MainWindow::LinkClicked(const QUrl &url)
{
    DBG qDebug() << " In Link Clicked with url: " << url.toString();
    QUrl toUrl(url);

    if (toUrl.toString().isEmpty()) {
        return;
    }

    // First fix up the url to replace 
    // any empty fragments with target file
    QString url_string = toUrl.toString();
    QFileInfo finfo(m_Filepath);
    if (url_string.startsWith("#")) {
        url_string.prepend(finfo.fileName());
    } else if (toUrl.scheme() == "file") {
        if (url_string.contains("/#")) {
            url_string.insert(url_string.indexOf("/#") + 1, finfo.fileName());
        }
    }
    toUrl = QUrl(url_string);

    if (toUrl.scheme() == "file") {
        QString filepath = toUrl.toLocalFile();
        QString fragment = toUrl.fragment();
        DBG qDebug() << "in Link Clicked: " << filepath << fragment;

        if (filepath.startsWith(m_Base)) {
            filepath = filepath.right(filepath.length() - m_Base.length());
            if (m_SpineList.contains(filepath)) {
                int index = m_SpineList.indexOf(filepath);
                if ((index > -1) && (index != m_ListPtr)) {
                    if (AllowSaveIfModified()) {
                        m_ListPtr = index;
                        ui.cbNavigate->setCurrentIndex(m_ListPtr);
                        m_CurrentFilePath = m_Base + m_SpineList.at(m_ListPtr);
                        UpdatePage(m_CurrentFilePath);
                    }
                }
                if (!fragment.isEmpty()) {
                    m_WebView->ScrollToFragment(fragment);
                }
                return;
            }
        }
    }
    QMessageBox::StandardButton button_pressed;
    button_pressed = Utility::warning(this, tr("PageEdit"), tr("Are you sure you want to open this link in your browser?\n\n%1").arg(toUrl.toString()), QMessageBox::Ok | QMessageBox::Cancel);

    if (button_pressed == QMessageBox::Ok) {
        QDesktopServices::openUrl(toUrl);
    }
}

void MainWindow::InspectPreviewPage()
{
    DBG qDebug() << "InspectPreviewPage()";
    // non-modal dialog
    if (!m_Inspector->isVisible()) {
        DBG qDebug() << "inspecting";
        m_Inspector->InspectPageofView(m_WebView);
        m_Inspector->show();
        m_Inspector->raise();
        m_Inspector->activateWindow();
        // if needed resulting m_WebView resize event will UpdateWindowTitle();
    } else {
        DBG qDebug() << "stopping inspection()";
        m_Inspector->StopInspection();
        m_Inspector->close();
        // if needed resulting m_WebView resize event will UpdateWindowTitle();
    }
}

void MainWindow::EditClips()
{
    if (!m_ClipEditor->isVisible()) {
        m_ClipEditor->show();
        m_ClipEditor->raise();
        m_ClipEditor->activateWindow();
    } else {
        m_ClipEditor->close();
    }
}

void MainWindow::ToggleClipToolbar()
{
    // ui.toolBarClips->toggleViewAction();
#if 1
    if (!ui.toolBarClips->isVisible()) {
        ui.toolBarClips->setVisible(true);
    } else {
        ui.toolBarClips->setVisible(false);
    }
#endif
}

void MainWindow::OpenClipsWindow()
{
    if (!m_Clips->isVisible()) {
        m_Clips->show();
    } else {
        m_Clips->hide();
    }
}

void MainWindow::SelectAllPreview()
{
    m_WebView->triggerPageAction(QWebEnginePage::SelectAll);
}

void MainWindow::CopyPreview()
{
    m_WebView->triggerPageAction(QWebEnginePage::Copy);
}

void MainWindow::ReloadPreview()
{
    // m_WebView->triggerPageAction(QWebEnginePage::ReloadAndBypassCache);
    // m_WebView->triggerPageAction(QWebEnginePage::Reload);
    emit RequestPreviewReload();
}

// returns false if the user wants to cancel the exit
bool MainWindow::AllowSaveIfModified() 
{
    // if the page source has been modified since it was loaded or saved
    // allow opportunity to save it
    QString source = GetSource();
    // qDebug() << "new  source: " << source;
    // qDebug() << "orig source: " << m_source;
    bool modified = false;
    if (!m_source.isEmpty()) {
        if (m_source.length() != source.length()) {
            modified = true;
        } else if (m_source != source) {
            modified = true;
        }
    }
    if (modified) {
        QMessageBox::StandardButton button_pressed;
        button_pressed = Utility::warning(this,
                          tr("PageEdit"),
                          tr("Do you want to save your changes before leaving?"),
                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                         );
        if (button_pressed == QMessageBox::Cancel) return false;
        if (button_pressed == QMessageBox::Save) {
            bool save_result = Save();
            int cnt = 0;
            while(!save_result && (cnt < 3)) {
                cnt++;
                save_result = SaveAs();
            }
        }
    }
    return true;
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    ShowMessageOnStatusBar(tr("PageEdit is closing..."));
    if (AllowSaveIfModified()) {
        SaveSettings();
        event->accept();
        return;
    }
    event->ignore();
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    // Workaround for Qt 4.8 bug - see WriteSettings() for details.                                             
    if (!isMaximized()) {
        m_LastWindowSize = saveGeometry();
    }
    QMainWindow::moveEvent(event);
}

void MainWindow::LoadSettings()
{
    SettingsStore settings;
    m_skipPrintWarnings = settings.skipPrintWarnings();
    m_skipPrintPreview = settings.skipPrintPreview();
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and its full screen status
    // Due to the 4.8 bug, we restore its "normal" window size and then maximize
    // it afterwards (if last state was maximized) to ensure on correct screen.
    bool isMaximized = settings.value("maximized", false).toBool();
    m_LastWindowSize = settings.value("geometry").toByteArray();

    if (!m_LastWindowSize.isNull()) {
        restoreGeometry(m_LastWindowSize);

        if (isMaximized) {
            setWindowState(windowState() | Qt::WindowMaximized);
        }
    }

    // The positions of all the toolbars and dock widgets
    QByteArray toolbars = settings.value("toolbars").toByteArray();

    if (!toolbars.isNull()) {
        restoreState(toolbars);
    }
    m_Inspector->hide();

    m_preserveHeadingAttributes = settings.value("preserveheadingattributes", true).toBool();
    SetPreserveHeadingAttributes(m_preserveHeadingAttributes);
    m_LastFolderOpen  = settings.value("lastfolderopen", QDir::homePath()).toString();
    QFileInfo fi(m_LastFolderOpen);
    if (!fi.exists() || !fi.isDir()) {
        m_LastFolderOpen  = QDir::homePath();
    }
    settings.endGroup();

    // Check for existing custom WebView stylesheet in Prefs dir
    QFileInfo CustomWebViewStylesheetInfo(QDir(Utility::DefinePrefsDir()).filePath(CUSTOM_WEBVIEW_STYLE_FILENAME));
    if (CustomWebViewStylesheetInfo.exists() &&
        CustomWebViewStylesheetInfo.isFile() &&
        CustomWebViewStylesheetInfo.isReadable()) {
        QString usercssurl = QUrl::fromLocalFile(CustomWebViewStylesheetInfo.absoluteFilePath()).toString();
        setUserCSSURL(usercssurl);
    }

    // Finally set up for spellchecking in the WebEngineView
    QStringList dict_names;
    dict_names.append(settings.uiDictionary());
    QWebEngineProfile *profile = m_WebView->page()->profile();
    profile->setSpellCheckEnabled(true);
    profile->setSpellCheckLanguages(dict_names);
}

void MainWindow::SaveSettings()
{
    SettingsStore settings;
    settings.setSkipPrintWarnings(m_skipPrintWarnings);
    settings.setSkipPrintPreview(m_skipPrintPreview);
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status                                                       
    // This is a workaround for this bug:                                                                       
    // Maximizing PageEdit and then closing will forget the previous window size                                   
    // and open it maximized on the wrong screen.                                                               
    // https://bugreports.qt-project.org/browse/QTBUG-21371                                                     
    settings.setValue("maximized", isMaximized());
    settings.setValue("geometry", m_LastWindowSize);
    // The positions of all the toolbars and dock widgets                                                       
    settings.setValue("toolbars", saveState());
    settings.setValue("preserveheadingattributes", m_preserveHeadingAttributes);
    settings.setValue("lastfolderopen",  m_LastFolderOpen);
    settings.endGroup();
}

const QMap<QString, QString> MainWindow::GetLoadFiltersMap()
{
    QMap<QString, QString> file_filters;
    file_filters[ "htm"   ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "html"  ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "xhtml" ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "opf"   ] = tr("OPF files (*.opf)");    
    file_filters[ "*"     ] = tr("All files (*.*)");
    return file_filters;
}

const QMap<QString, QString> MainWindow::GetSaveFiltersMap()
{
    QMap<QString, QString> file_filters;
    file_filters[ "htm"   ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "html"  ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "xhtml" ] = tr("HTML files (*.htm *.html *.xhtml)");
    return file_filters;
}

void MainWindow::ApplyHeadingToSelection(const QString &heading_type)
{
    ui.actionHeading1->setChecked(false);
    ui.actionHeading2->setChecked(false);
    ui.actionHeading3->setChecked(false);
    ui.actionHeading4->setChecked(false);
    ui.actionHeading5->setChecked(false);
    ui.actionHeading6->setChecked(false);
    ui.actionHeadingNormal->setChecked(false);
    if (heading_type == "Normal") {
        m_WebView->FormatBlock("p", m_preserveHeadingAttributes);
        ui.actionHeadingNormal->setChecked(true);
    } else if (heading_type[0].isDigit()) {
        QString heading = "h" + heading_type;
        m_WebView->FormatBlock(heading, m_preserveHeadingAttributes);
        if (heading_type == "1") {
            ui.actionHeading1->setChecked(true);
        } else if (heading_type == "2") {
            ui.actionHeading2->setChecked(true);
        } else if (heading_type == "3") {
            ui.actionHeading3->setChecked(true);
        } else if (heading_type == "4") {
            ui.actionHeading4->setChecked(true);
        } else if (heading_type == "5") {
            ui.actionHeading5->setChecked(true);
        } else if (heading_type == "6") {
            ui.actionHeading6->setChecked(true);
        }
    }
}

void MainWindow::SetPreserveHeadingAttributes(bool new_state)
{
    m_preserveHeadingAttributes = new_state;
    ui.actionHeadingPreserveAttributes->setChecked(m_preserveHeadingAttributes);
}

QString MainWindow::GetSource() 
{
    QString text = m_WebView->GetHtml();
    GumboInterface gi = GumboInterface(text, "any_version");
    QList<GumboNode*> nodes;
    // remove any added contenteditable attributes 
    nodes = gi.get_all_nodes_with_attribute(QString("contenteditable"));
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "contenteditable");
        if (attr) {
            GumboElement* element = &node->v.element;
            gumbo_element_remove_attribute(element, attr);
        }
    }
    text = gi.getxhtml();
    return text;
}

 
QString MainWindow::GetCleanHtml() 
{
    SettingsStore ss;

    QString text = m_WebView->GetHtml();
    // now remove any leftovers and make sure it is well formed
    GumboInterface gi = GumboInterface(text, "any_version");

    QList<GumboNode*> nodes;
    QList<GumboTag> tags;

    // remove any added contenteditable attributes 
    nodes = gi.get_all_nodes_with_attribute(QString("contenteditable"));
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "contenteditable");
        if (attr) {
            GumboElement* element = &node->v.element;
            gumbo_element_remove_attribute(element, attr);
        }
    }

    // remove any added is="http://www.w3.org/1999/xhtml"
    // attributes on heading tags
    tags = QList<GumboTag>() << GUMBO_TAG_H1 << GUMBO_TAG_H2 << GUMBO_TAG_H3 <<
                                GUMBO_TAG_H4 << GUMBO_TAG_H5 << GUMBO_TAG_H6;
    nodes = gi.get_all_nodes_with_tags(tags);
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "is");
        if (attr && QString::fromUtf8(attr->value) == "http://www.w3.org/1999/xhtml") {
            GumboElement* element = &node->v.element;
            gumbo_element_remove_attribute(element, attr);
        }
    }

    // clean up leftovers from pasting from Word
    // This is a bit risky as it may remove valid but rare style tags inside the body tag
    // So enable this only via an Environment Variable

    if (qEnvironmentVariableIsSet("PAGEEDIT_ENABLE_WORD_PASTE_CLEANUP")) {  

        // remove any xhtml comments as these are often added by 
        // pasting of Word data
        GumboNode * root = gi.get_root_node();
        QList<GumboNode*> comment_nodes = gi.get_nodes_with_comments(root);
        foreach(GumboNode* node, comment_nodes) {
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
        }

        // remove non html5 tags prefixed with "o:" as these are Word leftovers
        QList<GumboNode*> prefix_nodes = gi.get_element_nodes_with_prefix(root, "o:");
        foreach(GumboNode* node, prefix_nodes) {
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
        }

        // remove any style tags in the body as these are often added by
        // pasting of Word data
        GumboNode * body_node = gi.get_body_node();
        if (body_node) {
            QList<GumboTag> style_tags = QList<GumboTag>() << GUMBO_TAG_STYLE;
            QList<GumboNode*> style_nodes = gi.get_nodes_with_tags(body_node, style_tags);
            foreach(GumboNode* node, style_nodes) {
                gumbo_remove_from_parent(node);
                gumbo_destroy_node(node);
            }
        }

    }

    // remove any already injected pre-wrap editing style in head
    if (m_using_wsprewrap) {
        tags = QList<GumboTag>() << GUMBO_TAG_STYLE;
        nodes = gi.get_all_nodes_with_tags(tags);
        foreach(GumboNode * node, nodes) {
            QString styleinfo = gi.get_local_text_of_node(node);
            if (styleinfo == EDIT_WITH_PRE_WRAP) {
                gumbo_remove_from_parent(node);
                gumbo_destroy_node(node);
                break;
            }
        }
    }

    // remove any added AddDarkCSS (style node has id="PageEdit_Injected")
    tags = QList<GumboTag>() << GUMBO_TAG_STYLE;
    nodes = gi.get_all_nodes_with_tags(tags);
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "id");
        if (attr && QString::fromUtf8(attr->value) == "PageEdit_Injected") {
            // qDebug() << "removing PageEdit_Injected dark style";
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
            break;
        }
    }
    // then the associated scrollbar stylesheet link
    tags = QList<GumboTag>() << GUMBO_TAG_LINK;
    nodes = gi.get_all_nodes_with_tags(tags);
    foreach(GumboNode * node, nodes) {
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (attr) {
            QString attrval = QString::fromUtf8(attr->value);
            if (DARKCSSLINKS.contains(attrval) ) {
                // qDebug() << "removing dark css links";
                gumbo_remove_from_parent(node);
                gumbo_destroy_node(node);
                break;
            }
        }
    }

    // remove any injected user css stylesheet link in head
    // make sure this comes last so that it can override any earlier styles
    if (!m_usercssurl.isEmpty()) {
        tags = QList<GumboTag>() << GUMBO_TAG_LINK;
        nodes = gi.get_all_nodes_with_tags(tags);
        foreach(GumboNode * node, nodes) {
            GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "href");
            if (attr) {
                QString attrval = QString::fromUtf8(attr->value);
                if (attrval.contains(m_usercssurl)) {
                    gumbo_remove_from_parent(node);
                    gumbo_destroy_node(node);
                    break;
                }
            }
        }
    }
    if (ss.usePrettify()) {
        QString indent = "   ";
        text = gi.prettyprint(indent);
    } else {
        text = gi.getxhtml();
    }
    return text;
}

bool MainWindow::SaveAs()
{
    const QMap<QString,QString> c_SaveFiltersMap(GetSaveFiltersMap());
    QStringList filters(c_SaveFiltersMap.values());
    filters.removeDuplicates();
    QString filter_string = "";
    foreach(QString filter, filters) {
        filter_string += filter + ";;";
    }
    QString save_path       = "";
    QString default_filter  = "";

    if (m_LastFolderOpen.isEmpty()) {
        m_LastFolderOpen = QDir::homePath();
    }

    if (m_CurrentFilePath.isEmpty()) {
        m_CurrentFilePath = m_LastFolderOpen + "/" + DEFAULT_FILENAME;
    }

    // if we can save this file type, then we use the current filename
    if (c_SaveFiltersMap.contains(QFileInfo(m_CurrentFilePath).suffix().toLower())) {
        save_path       = m_CurrentFilePath;
        default_filter  = c_SaveFiltersMap.value(QFileInfo(m_CurrentFilePath).suffix().toLower());
    }
    // if not, we change the extension to xhtml
    else {
        save_path       = m_LastFolderOpen + "/" + QFileInfo(m_CurrentFilePath).completeBaseName() + ".xhtml";
        default_filter  = c_SaveFiltersMap.value("xhtml");
    }

    QString filename = QFileDialog::getSaveFileName(this,
                            tr("Save File"),
                            save_path,
                            filter_string,
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
                            & default_filter,
                            QFileDialog::DontUseNativeDialog
#else
                            & default_filter
#endif
                            );

    // QFileDialog cancelled
    if (filename.isEmpty()) {
        // The only time m_CurrentFilePath should be cleared after the SaveAs dialog is
        // cancelled, is when it's populated with the "untitled.xhtml" default filename.
        if (m_CurrentFilePath.endsWith(DEFAULT_FILENAME)) {
            m_CurrentFilePath.clear();
        }
        return false;
    }

    QString extension = QFileInfo(filename).suffix().toLower();
    if (extension.isEmpty()) {
        filename += ".xhtml";
    }

    // Store the full path to the destination file and its folder
    m_CurrentFilePath = QFileInfo(filename).absoluteFilePath();
    m_LastFolderOpen = QFileInfo(filename).absolutePath();

    QString text = GetCleanHtml();

    DBG qDebug() << "Save As: " << text;
    QFileInfo fi(m_CurrentFilePath);
    if (fi.exists() && !fi.isWritable()) {
        Utility::DisplayStdErrorDialog(tr("File Save-As Failed!"), m_CurrentFilePath + " " + tr("is not writeable"));
        ShowMessageOnStatusBar(tr("File Save-As Failed!"));
        m_CurrentFilePath.clear();
        return false;
    }

    bool save_result = false;
    try {
        Utility::WriteUnicodeTextFile(text, m_CurrentFilePath);
        ShowMessageOnStatusBar(tr("File Saved"));
        m_LastFolderOpen = fi.absolutePath();
        save_result = true;
    } catch (std::exception &e) {
        Utility::DisplayStdErrorDialog(tr("File Save-As Failed!"), e.what());
        ShowMessageOnStatusBar(tr("File Save-As Failed!"));
        m_CurrentFilePath.clear();
        save_result = false;
    }
    if (save_result) m_source = GetSource();
    return save_result;
}

void MainWindow::RefreshPage()
{
    m_WebView->page()->setBackgroundColor(Utility::WebViewBackgroundColor(true));
    if (!m_CurrentFilePath.isEmpty()) {
        QString text = GetCleanHtml();
        UpdatePage(m_CurrentFilePath, text);
    }
}

bool MainWindow::Save()
{
    QString text = GetCleanHtml();

    DBG qDebug() << "Saving: " << text;
    QFileInfo fi(m_CurrentFilePath);

    if (!fi.exists() || !fi.isWritable()) {
        Utility::DisplayStdErrorDialog(tr("File Save Failed!"), 
                       m_CurrentFilePath + " " + tr("does not exist or is not writeable"));
        ShowMessageOnStatusBar(tr("File Save Failed!"));
        return false;
    }

    bool save_result = false;
    try {
        Utility::WriteUnicodeTextFile(text, m_CurrentFilePath);
        ShowMessageOnStatusBar(tr("File Saved"));
        m_LastFolderOpen = fi.absolutePath();
        save_result = true;
    } catch (std::exception &e) {
        Utility::DisplayStdErrorDialog(tr("File Save Failed!"),e.what());
        ShowMessageOnStatusBar(tr("File Save Failed!"));
        save_result = false;;
    }
    if (save_result) m_source = GetSource();
    return save_result;
}

void MainWindow::printRendered()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    // Refresh skipflags from Prefs
    SettingsStore settings;
    m_skipPrintWarnings = settings.skipPrintWarnings();
    m_skipPrintPreview = settings.skipPrintPreview();

    if (!m_skipPrintWarnings) {
        QCheckBox *cb = new QCheckBox(tr("Do not show this warning again"), this);
        QString text = tr("This file may not print the way you expect it to.");
        QString detailed_text = tr("Dark backgrounds and colored text applied with an EPUB's CSS will print.");
        detailed_text = detailed_text + " " + tr("Use caution as this can result in a lot of ink being used!");
        detailed_text = detailed_text + " " + tr("Use the following Print Preview to see how this file will print.");
        detailed_text = detailed_text + " " + tr("Check the box if you don't wish to see this warning in the future.");
        QMessageBox msgbox;
        msgbox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        msgbox.setModal(true);
        msgbox.setWindowTitle("PageEdit");
        msgbox.setText("<h3>" + text + "</h3><br/>");
        msgbox.setIcon(QMessageBox::Icon::Warning);
        msgbox.setTextFormat(Qt::RichText);
        msgbox.setDetailedText(detailed_text);
        msgbox.setStandardButtons(QMessageBox::Close);
        msgbox.setCheckBox(cb);
        connect(cb, &QCheckBox::stateChanged, [this](int state) {
            if (static_cast<Qt::CheckState>(state) == Qt::CheckState::Checked) {
                m_skipPrintWarnings = true;    
            }
        });
        msgbox.exec();
    }
    settings.setSkipPrintWarnings(m_skipPrintWarnings);

    m_WebViewPrinter->setPage(m_WebView->url(), m_skipPrintPreview);
#else
    QMessageBox msgbox;
    QString text = tr("Feature not available before Qt5.12.x");
    msgbox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    msgbox.setModal(true);
    msgbox.setWindowTitle("PageEdit");
    msgbox.setText("<h3>" + text + "</h3><br/>");
    msgbox.setIcon(QMessageBox::Icon::Warning);
    msgbox.setStandardButtons(QMessageBox::Close);
    msgbox.exec();
#endif

}

void MainWindow::Open()
{

    if (m_LastFolderOpen.isEmpty()) {
        m_LastFolderOpen = QDir::homePath();
    }

#ifndef Q_OS_MAC
    QString source = GetSource();
    bool modified = false;
    if (!m_source.isEmpty()) {
        if (m_source.length() != source.length()) {
            modified = true;
        } else if (m_source != source) {
            modified = true;
        }
    }
    if (MaybeSaveDialogSaysProceed(modified))
#endif 
    {
        const QMap<QString, QString> load_filters = MainWindow::GetLoadFiltersMap();
        QStringList filters(load_filters.values());
        filters.removeDuplicates();
        QString filter_string = "";
        foreach(QString filter, filters) {
            filter_string += filter + ";;";
        }
        // "All Files (*.*)" is the default
        QString default_filter = load_filters.value("xhtml");
        QString filename = QFileDialog::getOpenFileName(0,
                           "Open File",
                           m_LastFolderOpen,
                           filter_string,
                           &default_filter);

        if (!filename.isEmpty()) {
            QFileInfo fi(filename);
            if (fi.exists() && fi.isReadable()) {
#ifdef Q_OS_MAC
                MainWindow * new_window = new MainWindow(filename);
                new_window->show();
                return;
#else
                m_ListPtr = -1;
                m_SpineList.clear();
                m_Base = QString();
                SetupFileList(filename);
                SetupNavigationComboBox();
                UpdatePage(m_CurrentFilePath);
                ShowMessageOnStatusBar(tr("File Opened"));
                return;
#endif
            }
            ShowMessageOnStatusBar(tr("File Open Failed!"));
        }
    }
}

void MainWindow::Exit()
{
    qApp->closeAllWindows();
    qApp->quit();
}

void MainWindow::Undo()      { m_WebView->triggerPageAction(QWebEnginePage::Undo);      }
void MainWindow::Redo()      { m_WebView->triggerPageAction(QWebEnginePage::Redo);      }
void MainWindow::Cut()       { m_WebView->triggerPageAction(QWebEnginePage::Cut);       }
void MainWindow::Copy()      { m_WebView->triggerPageAction(QWebEnginePage::Copy);      }
void MainWindow::SelectAll() { m_WebView->triggerPageAction(QWebEnginePage::SelectAll); }

void MainWindow::Paste()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (clipboard->mimeData()->hasHtml()) {
        QMessageBox msgBox(QMessageBox::Question,
               tr("Clipboard contains HTML formatting"),
               tr("Do you want to paste clipboard data as plain text?"),
               QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);

        // populate the detailed text window - by HTML not by the text
        msgBox.setDetailedText(clipboard->mimeData()->html());

        // show message box
        switch (msgBox.exec()) {
            case QMessageBox::Yes:
                m_WebView->triggerPageAction(QWebEnginePage::PasteAndMatchStyle);
                break;
            case QMessageBox::No:
                m_WebView->triggerPageAction(QWebEnginePage::Paste);
                break;
            default:
                // Cancel was clicked - do nothing
                break;
        }
    } else {
        m_WebView->triggerPageAction(QWebEnginePage::Paste);
    }
}

void MainWindow::PasteText(const QString& text)
{
    m_WebView->PasteText(text);
}

void MainWindow::PreferencesDialog()
{
    Preferences prefers(this);
    prefers.exec();

    if (prefers.isReloadPreviewRequired()) {
        RefreshPage();
    }

    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.                                 
        m_SelectCharacter->show();
    }
}

void MainWindow::SearchOnPage()
{
    if (!m_search) {
        m_search = new SearchToolbar(m_WebView, this);
        // m_search.data()->showMinimalInPopupWindow();
        m_layout->insertWidget(m_layout->count() - 1, m_search);
    }
    m_search->show();
    m_search->focusSearchLine();
}

void MainWindow::InsertSpecialCharacter()
{
    // non-modal dialog
    m_SelectCharacter->show();
    m_SelectCharacter->raise();
    m_SelectCharacter->activateWindow();
}

void MainWindow::InsertSGFSectionMarker()
{
    m_WebView->InsertHtml(BREAK_TAG_INSERT);
}

void MainWindow::InsertId()
{
    if (m_ListPtr == -1) return;

    QString id = m_WebView->GetAncestorTagAttributeValue("id", QStringList() << "a");

    // Prevent adding a hidden anchor id in PageEdit.
    if (id.isEmpty() && m_WebView->GetSelectedText().isEmpty()) {
        Utility::warning(this, tr("PageEdit"), tr("You must select text before inserting a new id."));
        return;
    }

    QString source = GetSource();
    SelectId select_id(id, source, this);

    if (select_id.exec() == QDialog::Accepted) {
        QString selected_id = select_id.GetId();
        QRegularExpression invalid_id("(^[^A-Za-z]|[^A-Za-z0-9_:\\.-])");
        QRegularExpressionMatch mo = invalid_id.match(selected_id);

        if (mo.hasMatch()) {
            Utility::warning(this, tr("PageEdit"), tr("ID is invalid - must start with a letter, followed by letter\
 number _ : - or ."));
            return;
        };

        if (!m_WebView->InsertId(select_id.GetId())) {
            Utility::warning(this, tr("PageEdit"), tr("You cannot insert an id at this position."));
        }
    }
}

void MainWindow::InsertHyperlink()
{

    if (m_ListPtr == -1) return;

    QString href = m_WebView->GetAncestorTagAttributeValue("href", QStringList() << "a");

    // Prevent adding a hidden anchor link in PageEdit.
    if (href.isEmpty() && m_WebView->GetSelectedText().isEmpty()) {
        Utility::warning(this, tr("PageEdit"), tr("You must select text before inserting a new link."));
        return;
    }

    QString source = GetSource();
    QString currentpath = GetCurrentFilePath();

    QStringList other_files = GetAllFilePaths(m_ListPtr);
    SelectHyperlink select_hyperlink(href, source, currentpath, m_Base, other_files, this);

    if (select_hyperlink.exec() == QDialog::Accepted) {
        QString target = select_hyperlink.GetTarget();
        if (target.contains("<") || target.contains(">")) {
            Utility::warning(this, tr("PageEdit"), tr("Link is invalid - cannot contain '<' or '>'"));
            return;
        };
        // handle external links
        if (target.indexOf(':') != -1) {
            if (!m_WebView->InsertHyperlink(target)) {
                Utility::warning(this, tr("PageEdit"), tr("Error inserting external link target."));
            }
        }
        // convert target to relative link from the current file
        std::pair<QString, QString> parts = Utility::parseHREF(target);
        QString relative_link = Utility::buildRelativePath(m_Base + currentpath, m_Base + parts.first);
        relative_link = relative_link + parts.second;
        if (!m_WebView->InsertHyperlink(relative_link)) {
            Utility::warning(this, tr("PageEdit"), tr("Error inserting a link at this position."));
        }
    }
}


void MainWindow::InsertFileDialog()
{
    if (m_MediaList.isEmpty()) return;

    QStringList selected_files;
    QString title = tr("Insert File");
    SelectFiles select_files(title, m_MediaList, m_MediaKind, m_MediaBase, this);

    if (select_files.exec() == QDialog::Accepted) {
        selected_files = select_files.SelectedImages();
        InsertFiles(selected_files);
    }
}

void MainWindow::InsertFiles(const QStringList &selected_files)
{
    if (selected_files.isEmpty()) return;
    
    QString currentpath = m_Base + GetCurrentFilePath();

    foreach(QString selected_file, selected_files) {
        QString mediapath = m_MediaBase + selected_file;

        int pos = m_MediaList.indexOf(selected_file);
        if (pos < 0) continue;
        QString mkind = m_MediaKind.at(pos);
    
        QFileInfo fi(mediapath);
        if (!fi.exists()) continue;
            
        QString relative_link = Utility::buildRelativePath(currentpath, mediapath);
        relative_link = Utility::URLEncodePath(relative_link);

        // extract just the filename without extension to create a text label
        QString filename = fi.fileName();
        if (filename.contains(".")) {
            filename = filename.left(filename.lastIndexOf("."));
        }
            
        QString html;
        if (mkind == "image" || mkind == "svgimage") {
            html = QString("<img alt=\"%1\" src=\"%2\"/>").arg(filename).arg(relative_link);
        } else if (mkind == "video") {
            // When inserted in BV the filename will disappear
            html = QString("<video controls=\"controls\" src=\"%1\">%2</video>").arg(relative_link).arg(filename);
        } else if (mkind == "audio") {
            html = QString("<audio controls=\"controls\" src=\"%1\">%2</audio>").arg(relative_link).arg(filename);
        }
        if (!m_WebView->InsertHtml(html)) {
            Utility::warning(this, tr("PageEdit"), tr("You cannot insert a media file at this position."));
        }
    }
}



void MainWindow::Subscript()
{
    m_WebView->ExecCommand("subscript");
}

void MainWindow::Superscript()
{
    m_WebView->ExecCommand("superscript");
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
void MainWindow::Bold()               { m_WebView->triggerPageAction(QWebEnginePage::ToggleBold);          }
void MainWindow::Italic()             { m_WebView->triggerPageAction(QWebEnginePage::ToggleItalic);        }
void MainWindow::Underline()          { m_WebView->triggerPageAction(QWebEnginePage::ToggleUnderline);     }
void MainWindow::Strikethrough()      { m_WebView->triggerPageAction(QWebEnginePage::ToggleStrikethrough); }
void MainWindow::AlignLeft()          { m_WebView->triggerPageAction(QWebEnginePage::AlignLeft);           }
void MainWindow::AlignCenter()        { m_WebView->triggerPageAction(QWebEnginePage::AlignCenter);         }
void MainWindow::AlignRight()         { m_WebView->triggerPageAction(QWebEnginePage::AlignRight);          }
void MainWindow::AlignJustify()       { m_WebView->triggerPageAction(QWebEnginePage::AlignJustified);      }
void MainWindow::DecreaseIndent()     { m_WebView->triggerPageAction(QWebEnginePage::Outdent);             }
void MainWindow::IncreaseIndent()     { m_WebView->triggerPageAction(QWebEnginePage::Indent);              }
void MainWindow::InsertBulletedList() { m_WebView->triggerPageAction(QWebEnginePage::InsertUnorderedList); }
void MainWindow::InsertNumberedList() { m_WebView->triggerPageAction(QWebEnginePage::InsertOrderedList);   }
#else
void MainWindow::Bold()               { m_WebView->ExecCommand("bold");                }
void MainWindow::Italic()             { m_WebView->ExecCommand("italic");              }
void MainWindow::Underline()          { m_WebView->ExecCommand("underline");           }
void MainWindow::Strikethrough()      { m_WebView->ExecCommand("strikeThrough");       }
void MainWindow::AlignLeft()          { m_WebView->ExecCommand("justifyLeft");         }
void MainWindow::AlignCenter()        { m_WebView->ExecCommand("justifyCenter");       }
void MainWindow::AlignRight()         { m_WebView->ExecCommand("justifyRight");        }
void MainWindow::AlignJustify()       { m_WebView->ExecCommand("justifyFull");         }
void MainWindow::DecreaseIndent()     { m_WebView->ExecCommand("outdent");             }
void MainWindow::IncreaseIndent()     { m_WebView->ExecCommand("indent");              }
void MainWindow::InsertBulletedList() { m_WebView->ExecCommand("insertUnorderedList"); }
void MainWindow::InsertNumberedList() { m_WebView->ExecCommand("insertOrderedList");   }
#endif

void MainWindow::ShowMessageOnStatusBar(const QString &message,
                                        int millisecond_duration)
{
    // It is only safe to add messages to the status bar on the GUI thread.                                         
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    // The MainWindow has to have a status bar initialised                                                          
    Q_ASSERT(statusBar());

    if (message.isEmpty()) {
        statusBar()->clearMessage();
    } else {
        statusBar()->showMessage(message, millisecond_duration);
    }
}

void MainWindow::sizeMenuIcons() {
    // Size icons based on Qfont line-spacing and a
    // user-preference tweakable scale-factor.
    SettingsStore settings;
    double iconscalefactor = settings.mainMenuIconSize();
    int iconsize = QFontMetrics(QFont()).lineSpacing() * iconscalefactor;
    if (iconsize < 12) iconsize = 12;
    if (iconsize > 48) iconsize = 48;

    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    foreach(QToolBar * toolbar, all_toolbars) {
        toolbar->setIconSize(QSize(iconsize,iconsize));
        // Kvantum themes on Linux will only show QToolBar "handles" when the movable
        // property is explicitly set to true (even though true is the Qt Default)
        toolbar->setMovable(true);
    }
}

bool MainWindow::MaybeSaveDialogSaysProceed(bool modified)
{
    // allow processing of any outstanding events
    qApp->processEvents();

    // since if nothing is modified you can not discard anything (as it is already saved)
    // and since you do not need to save anything (nothing can be lost)
    if (!modified) return true;

    QMessageBox::StandardButton button_pressed;
    button_pressed = Utility::warning(this,
                    tr("PageEdit"),
                    tr("Do you want to save any changes before overwriting this file?"),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                    );
    if (button_pressed == QMessageBox::Save) {
        return Save();
    } else if (button_pressed == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

#if 0
void MainWindow::ToggleSpellCheck()
{
    SettingsStore settings;
    QWebEngineProfile *profile = m_WebView->page()->profile();
    bool is_enabled = settings.spellCheck();
    profile->setSpellCheckEnabled(!is_enabled);
    settings.setSpellCheck(!is_enabled);
}
#endif

void MainWindow::AboutPageEdit()
{
    Utility::AboutBox();
}


// How to deal with this as each clipEntry struct created with new and passed via
// emit signal to here?  Where and how should their memory be freed.
// Perhaps we need to make clipEntry a QObject instead of just a struct or use
// smart pointers
void MainWindow::PasteClipEntriesIntoTarget(QList<ClipEditorModel::clipEntry *> clips)
{
    // target will always be the WebViewEdit    
    if (m_WebView && m_WebView->PasteClipEntries(clips)) {
        m_WebView->setFocus();
        ShowMessageOnStatusBar();
    }
    foreach(ClipEditorModel::clipEntry * entry, clips) {
        if (entry) delete entry;
    }
}

void MainWindow::PasteClipNumberIntoTarget(int clip_number)
{
    // target will always be the WebViewEdit
    bool applied = false;
    if (m_WebView) {
        applied = m_WebView->PasteClipNumber(clip_number);
    }
    if (applied) {
        ShowMessageOnStatusBar(tr("Pasted clip entry %1.").arg(clip_number));
    }
}

void MainWindow::UpdateClipButton(QAction *ui_action)
{
    // clipEntry is a simple struct created by GetEntry with new,
    // no reference counting or smart pointers so they must be cleaned up appropriately
    int clip_number = ui_action->data().toInt();
    ClipEditorModel::clipEntry *clip_entry = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);
    if (clip_entry) {
        ui_action->setText(clip_entry->name);
        QString clip_text = clip_entry->text;
        clip_text.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
        ui_action->setToolTip(clip_text);
        ui_action->setVisible(true);
        // prevent memory leak
        delete clip_entry;
    } else {
        ui_action->setText("");
        ui_action->setToolTip("");
        ui_action->setVisible(false);
    }
}

void MainWindow::UpdateClipsUI()
{
    foreach(QAction * clipaction, m_clactions) {
        UpdateClipButton(clipaction);
    }
}


void MainWindow::ConnectSignalsToSlots()
{
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ApplicationPaletteChanged()));

    connect(m_WebView,   SIGNAL(ZoomFactorChanged(float)),  this, SLOT(UpdateZoomLabel(float)));
    connect(m_WebView,   SIGNAL(ZoomFactorChanged(float)),  this, SLOT(UpdateZoomSlider(float)));
    connect(m_WebView,   SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));
    connect(m_WebView,   SIGNAL(selectionChanged()),        this, SLOT(SelectionChanged()));

    connect(ui.actionInspect,   SIGNAL(triggered()),                       this, SLOT(InspectPreviewPage()));
    connect(ui.actionClips,     SIGNAL(triggered()),                       this, SLOT(OpenClipsWindow()));
    connect(ui.actionClipEdit,  SIGNAL(triggered()),                       this, SLOT(EditClips()));
    connect(ui.actionClipBar,   SIGNAL(triggered()),                       this, SLOT(ToggleClipToolbar()));
    connect(m_SelectCharacter,  SIGNAL(SelectedCharacter(const QString&)), this, SLOT(PasteText(const QString &)));
    connect(ui.actionAbout,     SIGNAL(triggered()),                       this, SLOT(AboutPageEdit()));

    // Headings Related
    connect(ui.actionHeading1, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading1, "1");

    connect(ui.actionHeading2, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading2, "2");

    connect(ui.actionHeading3, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading3, "3");

    connect(ui.actionHeading4, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading4, "4");

    connect(ui.actionHeading5, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading5, "5");

    connect(ui.actionHeading6, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading6, "6");

    connect(ui.actionHeadingNormal, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeadingNormal, "Normal");

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_headingMapper, SIGNAL(mapped(const QString &)), this, SLOT(ApplyHeadingToSelection(const QString &)));
#else
    connect(m_headingMapper, SIGNAL(mappedString(const QString &)), this, SLOT(ApplyHeadingToSelection(const QString &)));
#endif

    connect(ui.actionHeadingPreserveAttributes,SIGNAL(triggered(bool)),this,SLOT(SetPreserveHeadingAttributes(bool)));

    // Navigation Related
    connect(ui.actionPrev,       SIGNAL(triggered()),    this, SLOT(EditPrev()));
    connect(ui.actionNext,       SIGNAL(triggered()),    this, SLOT(EditNext()));
    connect(ui.cbNavigate,       SIGNAL(activated(int)), this, SLOT(CBNavigateActivated(int)));
    connect(ui.actionLinkReturn, SIGNAL(triggered()),    this, SLOT(LinkReturn()));

    // File Related
    connect(ui.actionOpen,         SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui.actionSave,         SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui.actionPrint,        SIGNAL(triggered()), this, SLOT(printRendered()));
    connect(ui.actionSaveAs,       SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(ui.actionExit,         SIGNAL(triggered()), this, SLOT(Exit()));
    connect(ui.actionPreferences,  SIGNAL(triggered()), this, SLOT(PreferencesDialog()));
    // connect(ui.actionSpellCheck, SIGNAL(triggered()), this, SLOT(ToggleSpellCheck()));
      
    // Edit Related
    connect(ui.actionUndo,      SIGNAL(triggered()),  this,   SLOT(Undo()));
    connect(ui.actionRedo,      SIGNAL(triggered()),  this,   SLOT(Redo()));
    connect(ui.actionCut,       SIGNAL(triggered()),  this,   SLOT(Cut()));
    connect(ui.actionCopy,      SIGNAL(triggered()),  this,   SLOT(Copy()));
    connect(ui.actionPaste,     SIGNAL(triggered()),  this,   SLOT(Paste()));
    connect(ui.actionSelectAll, SIGNAL(triggered()),  this,   SLOT(SelectAll()));
    connect(ui.actionMode,      SIGNAL(toggled(bool)),this,   SLOT(ToggleMode(bool))); 

    //Find Related
    connect(ui.actionFind,      SIGNAL(triggered()),  this,   SLOT(SearchOnPage()));

    // Insert Related
    connect(ui.actionInsertSGFSectionMarker, SIGNAL(triggered()), this, SLOT(InsertSGFSectionMarker()));
    connect(ui.actionInsertSpecialCharacter, SIGNAL(triggered()), this, SLOT(InsertSpecialCharacter()));
    connect(ui.actionInsertBulletedList,     SIGNAL(triggered()), this, SLOT(InsertBulletedList()));
    connect(ui.actionInsertNumberedList,     SIGNAL(triggered()), this, SLOT(InsertNumberedList()));
    connect(ui.actionInsertId,               SIGNAL(triggered()), this, SLOT(InsertId()));
    connect(ui.actionInsertHyperlink,        SIGNAL(triggered()), this, SLOT(InsertHyperlink()));
    connect(ui.actionInsertFile,             SIGNAL(triggered()), this, SLOT(InsertFileDialog()));

    // Format Related
    connect(ui.actionBold,            SIGNAL(triggered()),  this,   SLOT(Bold()));
    connect(ui.actionItalic,          SIGNAL(triggered()),  this,   SLOT(Italic()));
    connect(ui.actionUnderline,       SIGNAL(triggered()),  this,   SLOT(Underline()));
    connect(ui.actionStrikethrough,   SIGNAL(triggered()),  this,   SLOT(Strikethrough()));
    connect(ui.actionSubscript,       SIGNAL(triggered()),  this,   SLOT(Subscript()));
    connect(ui.actionSuperscript,     SIGNAL(triggered()),  this,   SLOT(Superscript()));
    connect(ui.actionAlignLeft,       SIGNAL(triggered()),  this,   SLOT(AlignLeft()));
    connect(ui.actionAlignCenter,     SIGNAL(triggered()),  this,   SLOT(AlignCenter()));
    connect(ui.actionAlignRight,      SIGNAL(triggered()),  this,   SLOT(AlignRight()));
    connect(ui.actionAlignJustify,    SIGNAL(triggered()),  this,   SLOT(AlignJustify()));
    connect(ui.actionDecreaseIndent,  SIGNAL(triggered()),  this,   SLOT(DecreaseIndent()));
    connect(ui.actionIncreaseIndent,  SIGNAL(triggered()),  this,   SLOT(IncreaseIndent()));

    // Change case
    connect(ui.actionCasingLowercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingUppercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingTitlecase, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingCapitalize, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    m_casingChangeMapper->setMapping(ui.actionCasingLowercase,  Utility::Casing_Lowercase);
    m_casingChangeMapper->setMapping(ui.actionCasingUppercase,  Utility::Casing_Uppercase);
    m_casingChangeMapper->setMapping(ui.actionCasingTitlecase, Utility::Casing_Titlecase);
    m_casingChangeMapper->setMapping(ui.actionCasingCapitalize, Utility::Casing_Capitalize);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    connect(m_casingChangeMapper, SIGNAL(mapped(int)), this, SLOT(ChangeCasing(int)));
#else
    connect(m_casingChangeMapper, SIGNAL(mappedInt(int)), this, SLOT(ChangeCasing(int)));
#endif

    // View/Zoom Related
    connect(ui.actionZoomIn,     SIGNAL(triggered()),       this, SLOT(ZoomIn()));
    connect(ui.actionZoomOut,    SIGNAL(triggered()),       this, SLOT(ZoomOut()));
    connect(ui.actionZoomReset,  SIGNAL(triggered()),       this, SLOT(ZoomReset()));
    connect(m_slZoomSlider,      SIGNAL(valueChanged(int)), this, SLOT(SliderZoom(int)));
    connect(m_slZoomSlider,      SIGNAL(sliderMoved(int)),  this, SLOT(UpdateZoomLabel(int)));

    // Clips related
    connect(m_Clips,        SIGNAL(PasteClips(QList<ClipEditorModel::clipEntry *>)),
            this,            SLOT(PasteClipEntriesIntoTarget(QList<ClipEditorModel::clipEntry *>)));
    
    connect(m_ClipEditor, SIGNAL(PasteSelectedClipRequest(QList<ClipEditorModel::clipEntry *>)),
            this,           SLOT(PasteClipEntriesIntoTarget(QList<ClipEditorModel::clipEntry *>)));
    
    connect(m_ClipEditor,   SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,            SLOT(ShowMessageOnStatusBar(const QString &)));

    connect(m_ClipEditor,   SIGNAL(ClipsUpdated()),
            this,            SLOT(UpdateClipsUI()));
    
    foreach(QAction* clipaction, m_clactions) {
        // Use the new signal/slot syntax and use a lambda to
        // eliminate the need for the obsoleted QSignalMapper.
        // [captured variables]() {...anonymous processing to do...;}
        int i = clipaction->data().toInt();
        connect(clipaction, &QAction::triggered, this, [this,i]() {
                MainWindow::PasteClipNumberIntoTarget(i);
        });
    }
}
