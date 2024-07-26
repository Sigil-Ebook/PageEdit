/************************************************************************
**
**  Copyright (C) 2019-2024  Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QEventLoop>
#include <QDeadlineTimer>
#include <QTimer>
#include <QSize>
#include <QUrl>
#include <QDir>
#include <QMenu>
#include <QContextMenuEvent>
#include <QtWebEngineWidgets>
#include <QtWebEngineCore>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineScript>
#include <QWebEngineContextMenuRequest>
#include <QDebug>

#include "Utility.h"
#include "SettingsStore.h"
#include "WebProfileMgr.h"
#include "UIDictionary.h"

#include "WebPageEdit.h"
#include "WebViewEdit.h"

#define DBG if(0)

const QString SET_CURSOR_JS2 =
    "var range = document.createRange();"
    "range.setStart(element, 0);"
    "range.setEnd(element, 0);"
    "var selection = window.getSelection();"
    "selection.removeAllRanges();"
    "selection.addRange(range);";

const QStringList ANCHOR_TAGS = QStringList() << "a";

struct JSResult {
  QVariant res;
  bool     finished;
  JSResult() : res(QVariant("")), finished(false) {}
  JSResult(JSResult * pRes) : res(pRes->res),  finished(pRes->finished) {}
  ~JSResult() { }
  bool isFinished() { return finished; }
};

struct SetJavascriptResultFunctor {
    JSResult * pres;
    SetJavascriptResultFunctor(JSResult * pres) : pres(pres) {}
    void operator()(const QVariant &result) {
        pres->res.setValue(result);
        pres->finished = true;
        DBG qDebug() << "javascript done";
    }
};

struct HTMLResult {
  QString res;
  bool finished;
  HTMLResult() : res(QString("")), finished(false) {}
  HTMLResult(HTMLResult * pRes) : res(pRes->res),  finished(pRes->finished) {}
  bool isFinished() { return finished; }
};

struct SetToHTMLResultFunctor {
    HTMLResult * pres;
    SetToHTMLResultFunctor(HTMLResult * pres) : pres(pres) {}
    void operator()(const QString &result) {
        pres->res = result;
        pres->finished = true;
    }
};

WebViewEdit::WebViewEdit(QWidget *parent)
    : QWebEngineView(parent),
      c_jQuery(Utility::ReadUnicodeTextFile(":/javascript/jquery-3.6.4.min.js")),
      c_jQueryScrollTo(Utility::ReadUnicodeTextFile(":/javascript/jquery.scrollTo-2.1.2-min.js")),
      c_GetCaretLocation(Utility::ReadUnicodeTextFile(":/javascript/book_view_current_location.js")),
      c_GetBlock(Utility::ReadUnicodeTextFile(":/javascript/get_block.js")),
      c_FormatBlock(Utility::ReadUnicodeTextFile(":/javascript/format_block.js")),
      c_GetAncestor(Utility::ReadUnicodeTextFile(":/javascript/get_ancestor.js")),
      c_GetAncestorAttribute(Utility::ReadUnicodeTextFile(":/javascript/get_ancestor_attribute.js")),
      c_SetAncestorAttribute(Utility::ReadUnicodeTextFile(":/javascript/set_ancestor_attribute.js")),
      m_CaretLocationUpdate(QString()),
      m_CustomSetDocumentInProgress(false),
      m_pendingScrollToFragment(QString()),
      m_isLoadFinished(false),
      m_LoadOkay(false),
      m_menu(new QMenu(this))
{
    QWebEngineProfile* profile = WebProfileMgr::instance()->GetPreviewProfile();
    m_ViewWebPage = new WebPageEdit(profile, this);
    setPage(m_ViewWebPage);
    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    SetCurrentZoomFactor(settings.zoomPreview());
    m_dictionaries = UIDictionary::GetUIDictionaries();
    ConnectSignalsToSlots();
}

WebViewEdit::~WebViewEdit()
{
    if (m_ViewWebPage != NULL) {
        delete m_ViewWebPage;
        m_ViewWebPage = 0;
    }
}

void WebViewEdit::contextMenuEvent(QContextMenuEvent *event)
{
    const QWebEngineContextMenuRequest* request = lastContextMenuRequest();

    QWebEngineProfile *profile = page()->profile();
    m_menu->clear();

    if (request->isContentEditable()) {
        
        // Used for edit mode
        const QStringList &dictionaries = profile->spellCheckLanguages();

        if (!request->misspelledWord().isEmpty()) {
            QFont boldFont = m_menu->font();
            boldFont.setBold(true);

            for (const QString &suggestion : request->spellCheckerSuggestions()) {
	            QAction *action = m_menu->addAction(suggestion);
	            action->setFont(boldFont);

	            connect(action, &QAction::triggered, this, [=]() {
	                page()->replaceMisspelledWord(suggestion);
	            });
            }

            if (m_menu->actions().isEmpty()) {
	        m_menu->addAction(tr("No suggestions"))->setEnabled(false);
            }
        }
        m_menu->addSeparator();

        m_menu->addAction(pageAction(QWebEnginePage::Cut));
        m_menu->addAction(pageAction(QWebEnginePage::Copy));
        m_menu->addAction(pageAction(QWebEnginePage::Paste));
        m_menu->addAction(pageAction(QWebEnginePage::SelectAll));

        m_menu->addSeparator();

        QAction *spellcheckAction = new QAction(tr("Check Spelling"), nullptr);
        spellcheckAction->setCheckable(true);
        spellcheckAction->setChecked(profile->isSpellCheckEnabled());
        connect(spellcheckAction, &QAction::toggled, this, [profile](bool toogled) {
            profile->setSpellCheckEnabled(toogled);
        });
        m_menu->addAction(spellcheckAction);

        if (profile->isSpellCheckEnabled()) {
            QMenu *subMenu = m_menu->addMenu(tr("Select Language"));
            for (const QString &dict : m_dictionaries) {
                QAction *action = subMenu->addAction(dict);
                action->setCheckable(true);
                action->setChecked(dictionaries.contains(dict));
                connect(action, &QAction::triggered, this, [profile, dict](){
	            profile->setSpellCheckLanguages(QStringList()<<dict);
	        });
            }
        }
    } else {  
        // Used for preview mode
        m_menu->addAction(pageAction(QWebEnginePage::Copy));
        m_menu->addAction(pageAction(QWebEnginePage::SelectAll));
    }
    // connect(m_menu, &QMenu::aboutToHide, m_menu, &QObject::deleteLater);
    m_menu->popup(event->globalPos());
}

void WebViewEdit::LinkHovered(const QString &url)
{
    DBG qDebug() << "linkHovered received " << url;;
    m_hoverUrl = url;
}

QString WebViewEdit::GetHoverUrl()
{
    return m_hoverUrl;
}

QString WebViewEdit::GetCaretElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = get_block( node );"
      "if (startNode != null) { startNode.nodeName; }";
    return EvaluateJavascript(c_GetBlock + javascript).toString();
}

QString WebViewEdit::GetCaretLocationUpdate()
{
    StoreCaretLocationUpdate(GetCaretLocation());
    return m_CaretLocationUpdate;
}

QSize WebViewEdit::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void WebViewEdit::CustomSetDocument(const QString &path, const QString &html)
{

    if (html.isEmpty()) {
        return;
    }

    m_CustomSetDocumentInProgress = true;

    // If this is not the very first load of this document, store the caret location
    if (!url().isEmpty()) {

        // This next line really causes problems as it happens to interfere with later loading
        // StoreCurrentCaretLocation();
 
	// keep memory footprint small clear any caches when a new page loads
	if (url().toLocalFile() != path) {
	    page()->profile()->clearHttpCache();
	} 
    }

    m_isLoadFinished = false;

    // If Tidy is turned off, then PageEdit will explode if there is no xmlns
    // on the <html> element. So we will silently add it if needed to ensure
    // no errors occur, to allow loading of documents created outside of
    // PageEdit as well as catering for section splits etc.
    QString replaced_html = html;
    replaced_html = replaced_html.replace("<html>", "<html xmlns=\"http://www.w3.org/1999/xhtml\">");
    setContent(replaced_html.toUtf8(), "application/xhtml+xml;charset=UTF-8", QUrl::fromLocalFile(path));
}

bool WebViewEdit::IsLoadingFinished()
{
    return m_isLoadFinished;
}

void WebViewEdit::SetZoomFactor(float factor)
{
    SettingsStore settings;
    settings.setZoomPreview(factor);
    SetCurrentZoomFactor(factor);
    Zoom();
    emit ZoomFactorChanged(factor);
}

void WebViewEdit::SetCurrentZoomFactor(float factor)
{
    m_CurrentZoomFactor = factor;
}

float WebViewEdit::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}

void WebViewEdit::Zoom()
{
    setZoomFactor(m_CurrentZoomFactor);
}

void WebViewEdit::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomWeb();

    if (stored_factor != m_CurrentZoomFactor) {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

void WebViewEdit::ScrollToTop()
{
    QString caret_location = "var elementList = document.getElementsByTagName(\"body\");"
                             "var element = elementList[0];";
    QString scroll = "var from_top = window.innerHeight / 2;"
                     "if (typeof element !== 'undefined') {"
                     "    $.scrollTo(element, 0, {offset: {top:-from_top, left:0} });";
    QString script = caret_location + scroll + SET_CURSOR_JS2 + "}";

    DoJavascript(script);
}

void WebViewEdit::ScrollToFragment(const QString &fragment)
{
    if (IsLoadingFinished()) {
        ScrollToFragmentInternal(fragment);
    } else {
        m_pendingScrollToFragment = fragment;
    }
}

void WebViewEdit::ScrollToFragmentInternal(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToTop();
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" + fragment + "\");";
    QString scroll = "var from_top = window.innerHeight / 2.5;"
                     "if (typeof element !== 'undefined') {"
                     "$.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";
    QString script = caret_location + scroll + SET_CURSOR_JS2 + "}";
    DoJavascript(script);
}

void WebViewEdit::SelectPreviousChar()
{
    QString script = "var selection = window.getSelection();"
                     "selection.modify('extend', 'backward', 'character');";
    DoJavascript(script);
}


void WebViewEdit::LoadingStarted()
{
    DBG qDebug() << "Loading a page started";
    m_isLoadFinished = false;
    m_LoadOkay = false;
}

void WebViewEdit::LoadingProgress(int progress)
{
    DBG qDebug() << "Loading progress " << progress;
    // this hack was needed before QtWebEngine from Qt 5.15.5
    // as internal link loads generated no finished signal
    // if ((progress >= 100) && !m_CustomSetDocumentInProgress) {
    //     m_isLoadFinished = true;
    //     m_LoadOkay = true;
    // }
}

void WebViewEdit::UpdateFinishedState(bool okay)
{
    // Qt 5.12.2 returns loadFinished with okay set to false when caused by 
    // clicking a link that acceptNavigationRequest denies
    // even when there are no apparent errors!

    DBG qDebug() << "UpdateFinishedState with okay " << okay;
    // Wait until after jQuery load to set this
    // m_isLoadFinished = true;
    m_LoadOkay = okay;
    emit DocumentLoaded();
}

QString WebViewEdit::GetHtml() const 
{
    HTMLResult * pres = new HTMLResult();
    page()->toHtml(SetToHTMLResultFunctor(pres));
    while(!pres->isFinished()) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers, 100);
    }
    QString res = pres->res;
    delete pres;
    return res;
}

QString WebViewEdit::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";
    return EvaluateJavascript(javascript).toString();
}


void WebViewEdit::ApplyCaseChangeToSelection(const Utility::Casing &casing)
{
    //  Users can actually select too much here including tags
    //  Pasting over the tags then loses content (such as anchor links)
    //  So we detect if the user selected more than just text, and if so
    //  simply return

    // javascript to get html of window selection
    QString javascript = "var xml = '';"
                         "if (window.getSelection) {"
                         "  var selection = window.getSelection();"
                         "  if (selection.rangeCount > 0) {"
                         "    var range = selection.getRangeAt(0);"
                         "    var clonedSelection = range.cloneContents();"
                         "    var div = document.createElement('div');"
                         "    div.appendChild(clonedSelection);"
                         "    xml = div.innerHTML;"
                         "  }"
                         "}"
      "xml;";

      QString selected_html  = EvaluateJavascript(javascript).toString();

      QString selected_text = GetSelectedText();

      // if you selected more than just text simply return
      if (selected_text != selected_html) {
        return;
      }

      QString new_text = Utility::ChangeCase(selected_text, casing);

      // if nothing changed, simply return
      if (new_text == selected_text) {
          return;
      }
      // QApplication::clipboard()->setText(new_text);
      // triggerPageAction(QWebEnginePage::Paste);
      InsertHtml(new_text);

      // We will have lost our selection from the paste operation.
      for (int i = 0; i < new_text.length(); i++) {
          SelectPreviousChar();
      }
      return;
}

void WebViewEdit::PasteText(const QString &text)
{
    InsertHtml(text);
}

bool WebViewEdit::InsertHtml(const QString &html)
{
    return ExecCommand("insertHTML", html);
}

bool WebViewEdit::InsertId(const QString &id)
{
    const QString &element_name = "a";
    const QString &attribute_name = "id";
    return InsertTagAttribute(element_name, attribute_name, id, ANCHOR_TAGS);
}

bool WebViewEdit::InsertHyperlink(const QString &href)
{
    const QString &element_name = "a";
    const QString &attribute_name = "href";
    return InsertTagAttribute(element_name, attribute_name, href, ANCHOR_TAGS);
}


bool WebViewEdit::InsertTagAttribute(const QString &element_name, const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list, bool ignore_selection)
{
    QString selected_text = GetSelectedText();

    if (SetAncestorTagAttributeValue(attribute_name, attribute_value, tag_list)) {
        return true;
    }

    // We need to insert a new tag element into the document - cannot insert an empty
    // element or else Qt will push it into a new block. So we will just insert the
    // attribute value as some placeholder text.
    if (selected_text.isEmpty()) {
        selected_text = attribute_value;
    }

    QString safe_text(selected_text);
    safe_text.replace("&","&amp;");
    safe_text.replace("<", "&lt;");
    safe_text.replace(">", "&gt;");

    // Just prepend and append the tag pairs to the text
    const QString html = "<" + element_name + " " + attribute_name + "=\"" + 
	                  attribute_value + "\">" + safe_text + "</" + element_name + ">";
    bool insert_ok = InsertHtml(html);

    // We will have lost our selection from the insert - viewable text hasn't changed
    for (int i = 0; i < selected_text.length(); i++) {
        SelectPreviousChar();
    }

    return insert_ok;
}


QString WebViewEdit::GetAncestorTagAttributeValue(const QString &attribute_name, const QStringList &tag_list)
{
    if (tag_list.isEmpty() || attribute_name.isEmpty()) {
        return QString();
    }

    const QString js = c_GetAncestor + c_GetAncestorAttribute +
	"var tagNames = [\"" + tag_list.join("\", \"") + "\"];"
                       "var attributeName = '" + attribute_name + "';"
                       "var startNode = document.getSelection().anchorNode;"
	               "get_ancestor_attribute(startNode, tagNames, attributeName);";
    return EvaluateJavascript(js).toString();
}

bool WebViewEdit::SetAncestorTagAttributeValue(const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list)
{
    if (tag_list.isEmpty() || attribute_name.isEmpty()) {
        return false;
    }

    const QString js = c_GetAncestor + c_SetAncestorAttribute +
	"var tagNames = [\"" + tag_list.join("\", \"") + "\"];"
                       "var attributeName = '" + attribute_name + "';"
                       "var attributeValue = '" + attribute_value + "';"
                       "var startNode = document.getSelection().anchorNode;"
	               "set_ancestor_attribute(startNode, tagNames, attributeName, attributeValue);";
    bool applied = EvaluateJavascript(js).toBool();

#if 0
    if (applied) {
        emit contentsChangedExtra();
    }
#endif

    return applied;
}



QString WebViewEdit::EscapeJSString(const QString &string)
{
    QString new_string(string);
    /* \ -> \\ */
    // " -> \"
    // ' -> \'
    return new_string.replace("\\", "\\\\").replace("\"", "\\\"").replace("'", "\\'");
}

bool WebViewEdit::ExecCommand(const QString &command)
{
    QString javascript = QString("document.execCommand( '%1', false, null)").arg(EscapeJSString(command));
    return EvaluateJavascript(javascript).toBool();
}

bool WebViewEdit::ExecCommand(const QString &command, const QString &parameter)
{
    QString javascript = QString("document.execCommand( '%1', false, '%2' )")
                           .arg(EscapeJSString(command))
                           .arg(EscapeJSString(parameter));
    return EvaluateJavascript(javascript).toBool();
}

QVariant WebViewEdit::EvaluateJavascript(const QString &javascript)
{
    DBG qDebug() << "EvaluateJavascript: " << m_isLoadFinished;

    // do not try to evaluate javascripts with the page not loaded yet
    if (!m_isLoadFinished) return QVariant();

    JSResult * pres = new JSResult();
    QDeadlineTimer deadline(10000);  // in milliseconds

    DBG qDebug() << "evaluate javascript" << javascript;

    page()->runJavaScript(javascript, QWebEngineScript::ApplicationWorld, SetJavascriptResultFunctor(pres));
    while(!pres->isFinished() && (!deadline.hasExpired())) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 100);
    }
    QVariant res;
    if (pres->isFinished()) {
        res = pres->res;
        delete pres;
    } else {
        qDebug() << "Error: WebView EvaluateJavascript timed out";
	qDebug() << "   ... intentionally leaking the JSResult structure";
    }
    return res;
}

void WebViewEdit::DoJavascript(const QString &javascript)
{
    DBG qDebug() << "run javascript with " << m_isLoadFinished;

     // do not try to evaluate javascripts with the page not loaded yet
    if (!m_isLoadFinished) return;

    page()->runJavaScript(javascript, QWebEngineScript::ApplicationWorld);
}

bool WebViewEdit::QueryCommandState(const QString &command)
{
  QString javascript = QString("document.queryCommandState( '%1', false, null)").arg(EscapeJSString(command));
  return EvaluateJavascript(javascript).toBool();
}

void WebViewEdit::FormatBlock(const QString &element_name, bool preserve_attributes)
{
  if (element_name.isEmpty()) {
    return;
  }
  QString preserve = preserve_attributes ? "true" : "false";
    QString javascript =  c_GetBlock + c_FormatBlock +
                          "var node = document.getSelection().anchorNode;"
                          "var startNode = get_block( node );"
                          "var element = format_block( startNode, \"" + element_name + "\", " + preserve + " );"
                          "startNode.parentNode.replaceChild( element, startNode );"
                          + SET_CURSOR_JS2;
    DoJavascript(javascript);
}

// If we don't steal focus first, then the QWebView
// can have focus (and its QWebFrame) and still not
// really have it (no blinking cursor).
//   We also still need to attempt to GrabFocus even
// when shown as a Preview page (even though no cursor
// is shown) or else the QStackWidget will explode on
// Windows when switching to another tab when it tries
// to determine where the previous focus was.
void WebViewEdit::GrabFocus()
{
    ScrollToTop();
    qobject_cast<QWidget *>(parent())->setFocus();
    QWebEngineView::setFocus(Qt::MouseFocusReason);
}

void WebViewEdit::SetDocumentEditable(bool newstate)
{
  if (newstate) {
      page()->runJavaScript("document.documentElement.contentEditable = true", QWebEngineScript::ApplicationWorld);
      ExecCommand("defaultParagraphSeparator","p");
  } else {
      page()->runJavaScript("document.documentElement.contentEditable = false", QWebEngineScript::ApplicationWorld);
  }
}

void WebViewEdit::WebPageJavascriptOnLoad()
{
    page()->runJavaScript(c_jQuery, QWebEngineScript::ApplicationWorld);
    page()->runJavaScript(c_jQueryScrollTo, QWebEngineScript::ApplicationWorld);
    // page()->runJavaScript("document.documentElement.contentEditable = true", QWebEngineScript::ApplicationWorld);
    // set load finished only when javascript has been loaded and ready to run
    m_isLoadFinished = true;

    if (m_CustomSetDocumentInProgress) {
        if (!m_pendingScrollToFragment.isEmpty()) {
            ScrollToFragment(m_pendingScrollToFragment);
            m_pendingScrollToFragment.clear();
        } else {
            executeCaretUpdateInternal();
        }
        m_CustomSetDocumentInProgress = false;
    }
}

QString WebViewEdit::GetElementSelectingJS_NoTextNodes(const QList<ElementIndex> &hierarchy) const
{
    // TODO: see if replacing jQuery with pure JS will speed up
    // caret location scrolling... note the children()/contents() difference:
    // children() only considers element nodes, contents() considers text nodes too.
    QString element_selector = "$('html')";
    // We will have a different hierarchy depending on whether it was generated by CodeView or
    // generated by Preview. If the last element is '#text', strip it off and make sure the
    // element preceding it has a -1 index in order for the caret to move correctly.
    bool adjust_last_index = false;
    int hierarchy_length = hierarchy.count() - 1;

    if (hierarchy_length > 1 && hierarchy.last().name == "#text") {
        hierarchy_length--;
    }

    for (int i = 0; i < hierarchy_length; ++i) {
        int index = hierarchy[ i ].index;

        if ((i == hierarchy_length) && adjust_last_index) {
            index = -1;
        }

        element_selector.append(QString(".children().eq(%1)").arg(index));
    }

    element_selector.append(".get(0)");
    return element_selector;
}

QList<ElementIndex> WebViewEdit::GetCaretLocation()
{
    // The location element hierarchy encoded in a string
    QString location_string = EvaluateJavascript(c_GetCaretLocation).toString();
    return ConvertQWebPathToHierarchy(location_string);
}

QList<ElementIndex> WebViewEdit::ConvertQWebPathToHierarchy(const QString & webpath) const
{
    // The location element hierarchy encoded in a string
    QString location_string = webpath;
    QStringList elements    = location_string.split(",", Qt::SkipEmptyParts);
    QList<ElementIndex> location;
    foreach(QString element, elements) {
        ElementIndex new_element;
        new_element.name  = element.split(" ")[ 0 ];
        new_element.index = element.split(" ")[ 1 ].toInt();
        location.append(new_element);
    }
    return location;
}

QString WebViewEdit::ConvertHierarchyToQWebPath(const QList<ElementIndex>& hierarchy)
{
    QStringList pathparts;
    for (int i=0; i < hierarchy.count(); i++) {
        QString part = hierarchy.at(i).name + " " + QString::number(hierarchy.at(i).index);
        pathparts.append(part);
    }
    return pathparts.join(",");
}

void WebViewEdit::StoreCurrentCaretLocation()
{
    // Only overwrite the current location stored if it is empty, in case we specifically
    // want a new location when switching to a new view
    if (m_CaretLocationUpdate.isEmpty()) {
        StoreCaretLocationUpdate(GetCaretLocation());
    }
}

void WebViewEdit::StoreCaretLocationUpdate(const QList<ElementIndex> &hierarchy)
{
    QString caret_location = "var element = " + GetElementSelectingJS_NoTextNodes(hierarchy) + ";";
    // We scroll to the element and center the screen on it
    QString scroll = "var from_top = window.innerHeight / 2;"
                     "if (typeof element !== 'undefined') {"
                     "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";
    m_CaretLocationUpdate = caret_location + scroll + SET_CURSOR_JS2 + "}";
}

QString WebViewEdit::GetElementSelectingJS_WithTextNode(const QList<ElementIndex> &hierarchy) const
{
    QString element_selector = "$('html')";

    for (int i = 0; i < hierarchy.count() - 1; ++i) {
        element_selector.append(QString(".children().eq(%1)").arg(hierarchy[ i ].index));
    }

    element_selector.append(QString(".contents().eq(%1)").arg(hierarchy.last().index));
    element_selector.append(".get(0)");
    return element_selector;
}


bool WebViewEdit::ExecuteCaretUpdate()
{
    // If there is no caret location update pending...
    if (m_CaretLocationUpdate.isEmpty()) {
        return false;
    }

    // run it...
    DoJavascript(m_CaretLocationUpdate);
    // ...and clear the update.
    m_CaretLocationUpdate.clear();
    return true;
}

bool WebViewEdit::ExecuteCaretUpdate(const QString &caret_update)
{
    // This overload is for use with the Back To Link type functionality, where we have
    // a specific caret location javascript we want to force when the tab is fully loaded.
    if (!caret_update.isEmpty()) {
        m_CaretLocationUpdate = caret_update;
        return ExecuteCaretUpdate();
    }

    return false;
}

bool WebViewEdit::PasteClipNumber(int clip_number)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);
    if (!clip) {
        return false;
    }
    PasteClipEntry(clip);
    return true;
}

bool WebViewEdit::PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips)
{
    bool applied = false;
    foreach(ClipEditorModel::clipEntry * clip, clips) {
        applied = applied || PasteClipEntry(clip);
    }
    return applied;
}

bool WebViewEdit::PasteClipEntry(ClipEditorModel::clipEntry *clip)
{
    if (!clip || clip->text.isEmpty()) {
        return false;
    }
    QString text = clip->text;

    if (text.contains("\\1")) {
        text.replace("\\1", GetSelectedText());
    }
    InsertHtml(text);
    return true;
}


void WebViewEdit::ConnectSignalsToSlots()
{
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(UpdateFinishedState(bool)));
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(WebPageJavascriptOnLoad()));
    connect(page(), SIGNAL(loadStarted()), this, SLOT(LoadingStarted()));
    connect(page(), SIGNAL(loadProgress(int)), this, SLOT(LoadingProgress(int)));
    connect(page(), SIGNAL(LinkClicked(const QUrl &)), this, SIGNAL(LinkClicked(const QUrl &)));
    connect(page(), SIGNAL(linkHovered(const QString &)), this, SLOT(LinkHovered(const QString &)));
}
