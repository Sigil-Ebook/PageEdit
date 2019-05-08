/************************************************************************
**
**  Copyright (C) 2019 Kevin Hendricks, Doug Massay
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
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
#include <QToolBar>
#include <QtWebEngineWidgets/QWebEngineView>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTimer>
#include <QDebug>

#include "MainWindow.h"
#include "Inspector.h"
#include "SettingsStore.h"
#include "Utility.h"
#include "WebViewEdit.h"
#include "SelectCharacter.h"

static const QString SETTINGS_GROUP = "mainwindow";

static const QString BREAK_TAG_INSERT    = "<hr class=\"sigil_split_marker\" />";

MainWindow::MainWindow(QString filepath, QWidget *parent)
    :
    QMainWindow(parent),
    m_WebView(new WebViewEdit(this)),
    m_Inspector(new Inspector(this)),
    m_Filepath(QString()),
    m_GoToRequestPending(false),
    m_headingMapper(new QSignalMapper(this)),
    m_preserveHeadingAttributes(false),
    m_CurrentFilePath(filepath),
    m_SelectCharacter(new SelectCharacter(this))
{
    ui.setupUi(this);
    SetupView();
    LoadSettings();
    ConnectSignalsToSlots();
    QTimer::singleShot(200, this, SLOT(UpdatePage(m_CurrentFilePath)));
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

float MainWindow::GetZoomFactor()
{
    return m_WebView->GetZoomFactor();
}

void MainWindow::SetupView()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    setAttribute(Qt::WA_DeleteOnClose);
    
    // QWebEngineView events are routed to their parent
    m_WebView->installEventFilter(this);
    
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

    ui.actionOpen->setEnabled(true);
    ui.actionSave->setEnabled(true);
    ui.actionExit->setEnabled(true);
    ui.actionPreferences->setEnabled(true);
    
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

    m_WebView->Zoom();

    QApplication::restoreOverrideCursor();
}

void MainWindow::UpdatePage(QString filename_url)
{

#if 0
    if (!m_WebView->isVisible()) {
        return;
    }
#endif

    // qDebug() << "PV UpdatePage " << filename_url;
    // foreach(ElementIndex ei, location) qDebug()<< "PV name: " << ei.name << " index: " << ei.index;

    QString text = Utility::ReadUnicodeTextFile(filename_url);

#if 0
    // If the user has set a default stylesheet inject it
    if (!m_usercssurl.isEmpty()) {
        int endheadpos = text.indexOf("</head>");
        if (endheadpos > 1) {
            QString inject_userstyles = 
              "<link rel=\"stylesheet\" type=\"text/css\" "
	      "href=\"" + m_usercssurl + "\" />\n";
	    // qDebug() << "Preview injecting stylesheet: " << inject_userstyles;
            text.insert(endheadpos, inject_userstyles);
	    }
    }

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
 
    // qDebug() << "PreviewWindow UpdatePage load is Finished";
    // qDebug() << "PreviewWindow UpdatePage final step scroll to location";

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

void MainWindow::InspectorClosed(int i)
{
    qDebug() << "received finished with argument: " << i;
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


void MainWindow::SelectEntryOnHeadingToolbar(const QString &element_name)
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

void MainWindow::ApplyHeadingStyleToTab(const QString &heading_type)
{
    if (heading_type == "Normal") {
        m_WebView->FormatBlock("p", m_preserveHeadingAttributes);
    } else if (heading_type[0].isDigit()) {
        QString heading = "h" + heading_type;
        m_WebView->FormatBlock(heading, m_preserveHeadingAttributes);
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
        qDebug() << text;
	return true;
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
                       &default_filter
                                                   );

    if (!filename.isEmpty()) {
    	m_CurrentFilePath = filename;
    	UpdatePage(m_CurrentFilePath);
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

void MainWindow::Preferences()
{
    // fix me
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

void MainWindow::ZoomIn()
{
    // fix me
}


void MainWindow::ZoomOut()
{
    // fix me
}


void MainWindow::ZoomReset()
{
    // fix me
}

void MainWindow::ConnectSignalsToSlots()
{
    connect(m_WebView,   SIGNAL(ZoomFactorChanged(float)), this, SIGNAL(ZoomFactorChanged(float)));
    connect(m_WebView,   SIGNAL(LinkClicked(const QUrl &)), this, SLOT(LinkClicked(const QUrl &)));

    connect(ui.actionInspect, SIGNAL(triggered()),     this, SLOT(InspectPreviewPage()));
    connect(m_Inspector,      SIGNAL(finished(int)),   this, SLOT(InspectorClosed(int)));

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
    connect(m_headingMapper, SIGNAL(mapped(const QString &)), this, SLOT(ApplyHeadingStyleToTab(const QString &)));
    connect(ui.actionHeadingPreserveAttributes, SIGNAL(triggered(bool)), this, SLOT(SetPreserveHeadingAttributes(bool)));

    // File Related
    connect(ui.actionOpen,      SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui.actionSave,      SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui.actionExit,      SIGNAL(triggered()), this, SLOT(Exit()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(Preferences()));
      
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

}

