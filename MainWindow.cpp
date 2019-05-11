/************************************************************************
**
**  Copyright (C) 2019 Kevin Hendricks, Doug Massay
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
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QtWebEngineWidgets/QWebEngineSettings>
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

#include "MainWindow.h"
#include "Inspector.h"
#include "SettingsStore.h"
#include "Utility.h"
#include "WebViewEdit.h"
#include "SelectCharacter.h"
#include "Preferences.h"

static const QString SETTINGS_GROUP = "mainwindow";

static const QString CUSTOM_WEBVIEW_STYLE_FILENAME = "custom_webview_style.css";

static const QString BREAK_TAG_INSERT    = "<hr class=\"sigil_split_marker\" />";

const float ZOOM_STEP               = 0.1f;
const float ZOOM_MIN                = 0.09f;
const float ZOOM_MAX                = 5.0f;
const float ZOOM_NORMAL             = 1.0f;
static const int ZOOM_SLIDER_MIN    = 0;
static const int ZOOM_SLIDER_MAX    = 1000;
static const int ZOOM_SLIDER_MIDDLE = 500;
static const int ZOOM_SLIDER_WIDTH  = 140;


MainWindow::MainWindow(QString filepath, QWidget *parent)
    :
    QMainWindow(parent),
    m_WebView(new WebViewEdit(this)),
    m_Inspector(new Inspector(this)),
    m_Filepath(QString()),
    m_GoToRequestPending(false),
    m_headingMapper(new QSignalMapper(this)),
    m_preserveHeadingAttributes(false),
    m_SelectCharacter(new SelectCharacter(this)),
    m_slZoomSlider(NULL),
    m_lbZoomLabel(NULL),
    m_updateActionStatePending(false)
{
    ui.setupUi(this);
    SetupView();
    LoadSettings();
    ConnectSignalsToSlots();
    QFileInfo fi(filepath);
    m_CurrentFilePath=fi.absolutePath();
    QTimer::singleShot(200, this, SLOT(DoUpdatePage()));
}

MainWindow::~MainWindow()
{
    // BookViewPreview must be deleted before QWebInspector.
    // BookViewPreview's QWebPage is linked to the QWebInspector
    // and when deleted it will send a message to the linked QWebInspector
    // to remove the association. If QWebInspector is deleted before
    // BookViewPreview, BookViewPreview will try to access the deleted
    // QWebInspector and the application will SegFault. This is an issue
    // with how QWebPages interface with QWebInspector.

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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    setAttribute(Qt::WA_DeleteOnClose);
    
    QFrame *frame = new QFrame(this);
    QLayout *layout = new QVBoxLayout(frame);
    frame->setLayout(layout);
    layout->addWidget(m_WebView);
    layout->setContentsMargins(0, 0, 0, 0);
    frame->setObjectName("PrimaryFrame");
    setCentralWidget(frame);
    // m_Inspector->SetObjectName("Inspector");
    addDockWidget(Qt::RightDockWidgetArea, m_Inspector);
    m_Inspector->hide();

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

    // Add alternate icon sizes for actions
    ExtendIconSizes();

    // Headings QToolButton
    ui.tbHeadings->setPopupMode(QToolButton::InstantPopup);

    // Preferences
    ui.actionPreferences->setMenuRole(QAction::PreferencesRole);
    ui.actionPreferences->setEnabled(true);

    ui.actionOpen->setEnabled(true);
    ui.actionSave->setEnabled(true);
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
        CheckHeadingLevel(m_WebView->GetCaretElementName());
    }
    m_updateActionStatePending = false;
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
          UpdatePage(m_CurrentFilePath);
      }
  }
}

void MainWindow::UpdatePage(const QString &filename_url)
{
    QString text = Utility::ReadUnicodeTextFile(filename_url);

    // If the user has set a default stylesheet inject it
    if (!m_usercssurl.isEmpty()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_userstyles = 
              "<link rel=\"stylesheet\" type=\"text/css\" "
	      "href=\"" + m_usercssurl + "\" />\n";
	    // qDebug() << "WebView injecting stylesheet: " << inject_userstyles;
            text.insert(endheadpos, inject_userstyles);
	    }
    }

#if 0
    // If this page uses mathml tags, inject a polyfill
    // MathJax.js so that the mathml appears in the Preview Window
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

    m_Filepath = filename_url;
    m_WebView->CustomSetDocument(filename_url, text);

    // this next bit is allowing javascript to run before
    // the page is finished loading somehow? 
    // but we explicitly prevent that

    // Wait until the preview is loaded before moving cursor.
    while (!m_WebView->IsLoadingFinished()) {
        qApp->processEvents();
    }
    
    if (!m_WebView->WasLoadOkay()) qDebug() << "PV loadFinished with okay set to false!";
 
    // qDebug() << "WebViewWindow UpdatePage load is Finished";
    // qDebug() << "WebViewWindow UpdatePage final step scroll to location";

#if 0
    m_WebView->StoreCaretLocationUpdate(location);
    m_WebView->ExecuteCaretUpdate();
#endif
    UpdateWindowTitle();
}

void MainWindow::ScrollTo(QList<ElementIndex> location)
{
    // qDebug() << "received a PreviewWindow ScrollTo event";
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
        setWindowTitle(tr("Preview") + " (" + QString::number(width) + "x" + QString::number(height) + ")");
    }
}

QList<ElementIndex> MainWindow::GetCaretLocation()
{
    // qDebug() << "PreviewWindow in GetCaretLocation";
    QList<ElementIndex> hierarchy = m_WebView->GetCaretLocation();
    // foreach(ElementIndex ei, hierarchy) qDebug() << "name: " << ei.name << " index: " << ei.index;
    return hierarchy;
}

void MainWindow::SetZoomFactor(float factor)
{
    m_WebView->SetZoomFactor(factor);
}

void MainWindow::EmitGoToPreviewLocationRequest()
{
    // qDebug() << "EmitGoToPreviewLocationRequest request: " << m_GoToRequestPending;
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
	  // qDebug() << "child add event";
	  const QChildEvent *childEvent(static_cast<QChildEvent*>(event));
	  if (childEvent->child()) {
	      childEvent->child()->installEventFilter(this);
	  }
      }
      break;
    case QEvent::MouseButtonPress:
      {
	  // qDebug() << "Preview mouse button press event " << object;
	  const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
	  if (mouseEvent) {
	      if (mouseEvent->button() == Qt::LeftButton) {
		  //  qDebug() << "Detected Left Mouse Button Press Event";
 		  // qDebug() << "emitting GoToPreviewLocationRequest";
	          m_GoToRequestPending = true;
		  // we must delay long enough to separate out LinksClicked from scroll sync clicks
	          QTimer::singleShot(100, this, SLOT(EmitGoToPreviewLocationRequest()));
	      }
	  }
      }
      break;
    case QEvent::MouseButtonRelease:
      {
	  // qDebug() << "Preview mouse button release event " << object;
	  const QMouseEvent *mouseEvent(static_cast<QMouseEvent*>(event));
	  if (mouseEvent) {
	      if (mouseEvent->button() == Qt::LeftButton) {
		  // qDebug() << "Detected Left Mouse Button Release Event";
	      }
	  }
      }
      break;
    default:
      break;
  }
  return QObject::eventFilter(object, event);
}

void MainWindow::LinkClicked(const QUrl &url)
{
    if (m_GoToRequestPending) m_GoToRequestPending = false;

    // qDebug() << "in PreviewWindow LinkClicked with url :" << url.toString();

    if (url.toString().isEmpty()) {
        return;
    }

    QFileInfo finfo(m_Filepath);
    QString url_string = url.toString();

    // Convert fragments to full filename/fragments
    if (url_string.startsWith("#")) {
        url_string.prepend(finfo.fileName());
    } else if (url.scheme() == "file") {
        if (url_string.contains("/#")) {
            url_string.insert(url_string.indexOf("/#") + 1, finfo.fileName());
        }
    }
    emit OpenUrlRequest(QUrl(url_string));
}

void MainWindow::InspectPreviewPage()
{
    qDebug() << "InspectPreviewPage()";
    // non-modal dialog
    if (!m_Inspector->isVisible()) {
        qDebug() << "inspecting";
        m_Inspector->InspectPage(m_WebView->page());
        m_Inspector->show();
        m_Inspector->raise();
        m_Inspector->activateWindow();
	return;
    }
    qDebug() << "stopping inspection()";
    m_Inspector->close();
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

void MainWindow::LoadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // m_Layout->restoreState(settings.value("layout").toByteArray());
    settings.endGroup();

    // Our default fonts for WebView
    SettingsStore::WebViewAppearance WVAppearance = settings.webViewAppearance();
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize, WVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont, WVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont, WVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont, WVAppearance.font_family_sans_serif);

    // Check for existing custom WebView stylesheet in Prefs dir
    QFileInfo CustomWebViewStylesheetInfo(QDir(Utility::DefinePrefsDir()).filePath(CUSTOM_WEBVIEW_STYLE_FILENAME));
    if (CustomWebViewStylesheetInfo.exists() &&
        CustomWebViewStylesheetInfo.isFile() &&
        CustomWebViewStylesheetInfo.isReadable()) {
        QString m_usercssurl = QUrl::fromLocalFile(CustomWebViewStylesheetInfo.absoluteFilePath()).toString();
    }
}

const QMap<QString, QString> MainWindow::GetLoadFiltersMap()
{
    QMap<QString, QString> file_filters;
    file_filters[ "htm"   ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "html"  ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "xhtml" ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "txt"   ] = tr("Text files (*.txt)");
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

// fix me Save and Open
bool MainWindow::Save()
{
    QString text = m_WebView->GetHtml();
    qDebug() << "Saving: " << text;
    QFileInfo fi(m_CurrentFilePath);
    if (fi.exists() && fi.isWritable()) {
 	Utility::WriteUnicodeTextFile(text, m_CurrentFilePath);
        ShowMessageOnStatusBar(tr("File Saved"));
	return true;
    }
    ShowMessageOnStatusBar(tr("File Save Failed!"));
    return false;
}

void MainWindow::Open()
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
                       "~",
                       filter_string,
                       &default_filter);

    if (!filename.isEmpty()) {
        QFileInfo fi(filename);
        if (fi.exists() && fi.isReadable()) {
    	    m_CurrentFilePath = filename;
    	    UpdatePage(m_CurrentFilePath);
            ShowMessageOnStatusBar(tr("File Opened"));
	    return;
	}
        ShowMessageOnStatusBar(tr("File Open Failed!"));
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
    Preferences preferences(this);
    preferences.exec();
    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.                                 
        m_SelectCharacter->show();
    }
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

void MainWindow::InsertBulletedList()
{
    m_WebView->triggerPageAction(QWebEnginePage::InsertUnorderedList);
    // m_WebView->ExecCommand("insertUnorderedList");
}

void MainWindow::InsertNumberedList()
{
    m_WebView->triggerPageAction(QWebEnginePage::InsertOrderedList);
    // m_WebView->ExecCommand("insertOrderedList");
}

void MainWindow::Subscript()
{
    m_WebView->ExecCommand("subscript");
}

void MainWindow::Superscript()
{
    m_WebView->ExecCommand("superscript");
}

void MainWindow::Bold()           { m_WebView->triggerPageAction(QWebEnginePage::ToggleBold);          }
void MainWindow::Italic()         { m_WebView->triggerPageAction(QWebEnginePage::ToggleItalic);        }
void MainWindow::Underline()      { m_WebView->triggerPageAction(QWebEnginePage::ToggleUnderline);     }
void MainWindow::Strikethrough()  { m_WebView->triggerPageAction(QWebEnginePage::ToggleStrikethrough); }
void MainWindow::AlignLeft()      { m_WebView->triggerPageAction(QWebEnginePage::AlignLeft);           }
void MainWindow::AlignCenter()    { m_WebView->triggerPageAction(QWebEnginePage::AlignCenter);         }
void MainWindow::AlignRight()     { m_WebView->triggerPageAction(QWebEnginePage::AlignRight);          }
void MainWindow::AlignJustify()   { m_WebView->triggerPageAction(QWebEnginePage::AlignJustified);      }
void MainWindow::DecreaseIndent() { m_WebView->triggerPageAction(QWebEnginePage::Outdent);             }
void MainWindow::IncreaseIndent() { m_WebView->triggerPageAction(QWebEnginePage::Indent);              }


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
  }
}

void MainWindow::ExtendIconSizes()
{
    QIcon icon;

    icon = ui.actionSave->icon();
    icon.addFile(QString::fromUtf8(":/icons/document-save_22px.png"));
    ui.actionSave->setIcon(icon);

    icon = ui.actionCut->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-cut_22px.png"));
    ui.actionCut->setIcon(icon);

    icon = ui.actionPaste->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-paste_22px.png"));
    ui.actionPaste->setIcon(icon);

    icon = ui.actionUndo->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-undo_22px.png"));
    ui.actionUndo->setIcon(icon);

    icon = ui.actionRedo->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-redo_22px.png"));
    ui.actionRedo->setIcon(icon);

    icon = ui.actionCopy->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-copy_22px.png"));
    ui.actionCopy->setIcon(icon);

    icon = ui.actionSelectAll->icon();
    icon.addFile(QString::fromUtf8(":/icons/edit-select-all_22px.png"));
    ui.actionSelectAll->setIcon(icon);

    icon = ui.actionAlignLeft->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-justify-left_22px.png"));
    ui.actionAlignLeft->setIcon(icon);

    icon = ui.actionAlignRight->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-justify-right_22px.png"));
    ui.actionAlignRight->setIcon(icon);

    icon = ui.actionAlignCenter->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-justify-center_22px.png"));
    ui.actionAlignCenter->setIcon(icon);

    icon = ui.actionAlignJustify->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-justify-fill_22px.png"));
    ui.actionAlignJustify->setIcon(icon);

    icon = ui.actionBold->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-bold_22px.png"));
    ui.actionBold->setIcon(icon);

    icon = ui.actionItalic->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-italic_22px.png"));
    ui.actionItalic->setIcon(icon);

    icon = ui.actionUnderline->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-underline_22px.png"));
    ui.actionUnderline->setIcon(icon);

    icon = ui.actionStrikethrough->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-strikethrough_22px.png"));
    ui.actionStrikethrough->setIcon(icon);

    icon = ui.actionSubscript->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-subscript_22px.png"));
    ui.actionSubscript->setIcon(icon);

    icon = ui.actionSuperscript->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-text-superscript_22px.png"));
    ui.actionSuperscript->setIcon(icon);

    icon = ui.actionInsertNumberedList->icon();
    icon.addFile(QString::fromUtf8(":/icons/insert-numbered-list_22px.png"));
    ui.actionInsertNumberedList->setIcon(icon);

    icon = ui.actionInsertBulletedList->icon();
    icon.addFile(QString::fromUtf8(":/icons/insert-bullet-list_22px.png"));
    ui.actionInsertBulletedList->setIcon(icon);

    icon = ui.actionIncreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-indent-more_22px.png"));
    ui.actionIncreaseIndent->setIcon(icon);

    icon = ui.actionDecreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/icons/format-indent-less_22px.png"));
    ui.actionDecreaseIndent->setIcon(icon);

    icon = ui.actionHeading1->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-1_22px.png"));
    ui.actionHeading1->setIcon(icon);

    icon = ui.actionHeading2->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-2_22px.png"));
    ui.actionHeading2->setIcon(icon);

    icon = ui.actionHeading3->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-3_22px.png"));
    ui.actionHeading3->setIcon(icon);

    icon = ui.actionHeading4->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-4_22px.png"));
    ui.actionHeading4->setIcon(icon);

    icon = ui.actionHeading5->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-5_22px.png"));
    ui.actionHeading5->setIcon(icon);

    icon = ui.actionHeading6->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-6_22px.png"));
    ui.actionHeading6->setIcon(icon);

    icon = ui.tbHeadings->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-all_22px.png"));
    ui.tbHeadings->setIcon(icon);

    icon = ui.actionHeadingNormal->icon();
    icon.addFile(QString::fromUtf8(":/icons/heading-normal_22px.png"));
    ui.actionHeadingNormal->setIcon(icon);

    icon = ui.actionOpen->icon();
    icon.addFile(QString::fromUtf8(":/icons/document-open_22px.png"));
    ui.actionOpen->setIcon(icon);

    icon = ui.actionExit->icon();
    icon.addFile(QString::fromUtf8(":/icons/process-stop_22px.png"));
    ui.actionExit->setIcon(icon);

    icon = ui.actionInsertSGFSectionMarker->icon();
    icon.addFile(QString::fromUtf8(":/icons/split-section_22px.png"));
    ui.actionInsertSGFSectionMarker->setIcon(icon);

    icon = ui.actionInsertSpecialCharacter->icon();
    icon.addFile(QString::fromUtf8(":/icons/insert-special-character_22px.png"));
    ui.actionInsertSpecialCharacter->setIcon(icon);

    icon = ui.actionZoomIn->icon();
    icon.addFile(QString::fromUtf8(":/icons/list-add_22px.png"));
    ui.actionZoomIn->setIcon(icon);

    icon = ui.actionZoomOut->icon();
    icon.addFile(QString::fromUtf8(":/icons/list-remove_22px.png"));
    ui.actionZoomOut->setIcon(icon);
    
    icon = ui.actionInspect->icon();
    icon.addFile(QString::fromUtf8(":/icons/inspect_22px.png"));
    ui.actionInspect->setIcon(icon);

}

void MainWindow::ConnectSignalsToSlots()
{
    connect(m_WebView,   SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_WebView,   SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));
    connect(m_WebView,   SIGNAL(selectionChanged()), this, SLOT(SelectionChanged()));

    connect(ui.actionInspect, SIGNAL(triggered()),     this, SLOT(InspectPreviewPage()));
    connect(m_SelectCharacter, SIGNAL(SelectedCharacter(const QString &)), this, SLOT(PasteText(const QString &)));

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
    connect(m_headingMapper, SIGNAL(mapped(const QString &)), this, SLOT(ApplyHeadingToSelection(const QString &)));
    connect(ui.actionHeadingPreserveAttributes, SIGNAL(triggered(bool)), this, SLOT(SetPreserveHeadingAttributes(bool)));

    // File Related
    connect(ui.actionOpen,      SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui.actionSave,      SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui.actionExit,      SIGNAL(triggered()), this, SLOT(Exit()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(PreferencesDialog()));
      
    // Edit Related
    connect(ui.actionUndo,      SIGNAL(triggered()),  this,   SLOT(Undo()));
    connect(ui.actionRedo,      SIGNAL(triggered()),  this,   SLOT(Redo()));
    connect(ui.actionCut,       SIGNAL(triggered()),  this,   SLOT(Cut()));
    connect(ui.actionCopy,      SIGNAL(triggered()),  this,   SLOT(Copy()));
    connect(ui.actionPaste,     SIGNAL(triggered()),  this,   SLOT(Paste()));
    connect(ui.actionSelectAll, SIGNAL(triggered()),  this,   SLOT(SelectAll()));

    // Insert Related
    connect(ui.actionInsertSGFSectionMarker,   SIGNAL(triggered()),  this,   SLOT(InsertSGFSectionMarker()));
    connect(ui.actionInsertSpecialCharacter,   SIGNAL(triggered()),  this,   SLOT(InsertSpecialCharacter()));
    connect(ui.actionInsertBulletedList,       SIGNAL(triggered()),  this,   SLOT(InsertBulletedList()));
    connect(ui.actionInsertNumberedList,       SIGNAL(triggered()),  this,   SLOT(InsertNumberedList()));

    // Format Related
    connect(ui.actionBold,                     SIGNAL(triggered()),  this,   SLOT(Bold()));
    connect(ui.actionItalic,                   SIGNAL(triggered()),  this,   SLOT(Italic()));
    connect(ui.actionUnderline,                SIGNAL(triggered()),  this,   SLOT(Underline()));
    connect(ui.actionStrikethrough,            SIGNAL(triggered()),  this,   SLOT(Strikethrough()));
    connect(ui.actionSubscript,                SIGNAL(triggered()),  this,   SLOT(Subscript()));
    connect(ui.actionSuperscript,              SIGNAL(triggered()),  this,   SLOT(Superscript()));
    connect(ui.actionAlignLeft,                SIGNAL(triggered()),  this,   SLOT(AlignLeft()));
    connect(ui.actionAlignCenter,              SIGNAL(triggered()),  this,   SLOT(AlignCenter()));
    connect(ui.actionAlignRight,               SIGNAL(triggered()),  this,   SLOT(AlignRight()));
    connect(ui.actionAlignJustify,             SIGNAL(triggered()),  this,   SLOT(AlignJustify()));
    connect(ui.actionDecreaseIndent,           SIGNAL(triggered()),  this,   SLOT(DecreaseIndent()));
    connect(ui.actionIncreaseIndent,           SIGNAL(triggered()),  this,   SLOT(IncreaseIndent()));

    // View/Zoom Related
    connect(ui.actionZoomIn,        SIGNAL(triggered()), this, SLOT(ZoomIn()));
    connect(ui.actionZoomOut,       SIGNAL(triggered()), this, SLOT(ZoomOut()));
    connect(ui.actionZoomReset,     SIGNAL(triggered()), this, SLOT(ZoomReset()));
    connect(m_slZoomSlider,         SIGNAL(valueChanged(int)), this, SLOT(SliderZoom(int)));
    connect(m_slZoomSlider,         SIGNAL(sliderMoved(int)),  this, SLOT(UpdateZoomLabel(int)));
}
