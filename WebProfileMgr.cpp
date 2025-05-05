/************************************************************************
**
**  Copyright (C) 2023-2025 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QDir>
#include <QString>
#include <QStringList>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

#include "Utility.h"
#include "SettingsStore.h"
#include "URLInterceptor.h"
#include "WebProfileMgr.h"

WebProfileMgr *WebProfileMgr::m_instance = 0;

WebProfileMgr *WebProfileMgr::instance()
{
    if (m_instance == 0) {
        m_instance = new WebProfileMgr();
    }

    return m_instance;
}


QWebEngineProfile*  WebProfileMgr::GetPreviewProfile()
{
    return m_preview_profile;
}

QWebEngineProfile* WebProfileMgr::GetOneTimeProfile()
{
    return m_onetime_profile;
}


void WebProfileMgr::InitializeDefaultSettings(QWebEngineSettings* web_settings)
{
    SettingsStore ss;
    SettingsStore::WebViewAppearance WVAppearance = ss.webViewAppearance();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize, WVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont, WVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont, WVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont, WVAppearance.font_family_sans_serif);
    
    web_settings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);
    web_settings->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    web_settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    web_settings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
    web_settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
    web_settings->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
    web_settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
    web_settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::PdfViewerEnabled, false);
}


WebProfileMgr::WebProfileMgr()
{
    m_URLint = new URLInterceptor();
    
    // initialize the defaultProfile to be restrictive for security
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
    InitializeDefaultSettings(web_settings);
    // Use URLInterceptor for protection
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(m_URLint);

    // create the profile for Preview / Edit
    SettingsStore ss;
    // m_preview_profile = new QWebEngineProfile("Preview", nullptr);
    // Use an off-the-rcord to workaround profile creation bugs
    m_preview_profile = new QWebEngineProfile();
    InitializeDefaultSettings(m_preview_profile->settings());
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    m_preview_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, (ss.remoteOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    // Enable local-storage for epub3
    QString localStorePath = Utility::DefinePrefsDir() + "/local-storage/";
    QDir storageDir(localStorePath);
    if (!storageDir.exists()) {
        storageDir.mkpath(localStorePath);
    }
    m_preview_profile->setPersistentStoragePath(localStorePath);
    m_preview_profile->setSpellCheckEnabled(true);
    // Use our URLInterceptor
    m_preview_profile->setUrlRequestInterceptor(m_URLint);

    // create the profile for OneTime
    m_onetime_profile = new QWebEngineProfile();
    InitializeDefaultSettings(m_onetime_profile->settings());
    m_onetime_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    // Use URLInterceptor for protection
    m_onetime_profile->setUrlRequestInterceptor(m_URLint);
}

