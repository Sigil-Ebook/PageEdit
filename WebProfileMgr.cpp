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
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
#include <QWebEngineProfileBuilder>
#endif

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

QWebEngineProfile*  WebProfileMgr::GetInspectorProfile()
{
    return m_inspector_profile;
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
    
    // create local storage path if needed
    QString localStorePath = Utility::DefinePrefsDir() + "/local-storage/";
    QDir storageDir(localStorePath);
    if (!storageDir.exists()) {
        storageDir.mkpath(localStorePath);
    }

    // create devtools storage path if needed
    QString devToolsStorePath = Utility::DefinePrefsDir() + "/local-devtools/";
    QDir devstorageDir(devToolsStorePath);
    if (!devstorageDir.exists()) {
        devstorageDir.mkpath(devToolsStorePath);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    // create a place for Preview Caches if needed
    QString PreviewCachePath = Utility::DefinePrefsDir() + "/Preview-Cache/";
    QDir preview_cacheDir(PreviewCachePath);
    if (!preview_cacheDir.exists()) {
        preview_cacheDir.mkpath(PreviewCachePath);
    }
    // create a place for Inspector Caches if needed
    QString InspectorCachePath = Utility::DefinePrefsDir() + "/Inspector-Cache/";
    QDir inspector_cacheDir(InspectorCachePath);
    if (!inspector_cacheDir.exists()) {
        inspector_cacheDir.mkpath(InspectorCachePath);
    }
#endif


   // Preview Profile
    // ---------------
    // create the profile for Preview/Edit
    // we may need to give this profile a unique storage name otherwise cache
    // is never cleared on Windows by a second or third instance of Sigil
    // m_preview_profile = new QWebEngineProfile(QString("Preview-") + Utility::CreateUUID(), nullptr);

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    m_preview_profile = new QWebEngineProfile();
    m_preview_profile->setPersistentStoragePath(localStorePath);
#else
    QWebEngineProfileBuilder pb;
    pb.setCachePath(PreviewCachePath);
    pb.setHttpCacheMaximumSize(0); // 0 - means let Qt control it
    pb.setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    pb.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    pb.setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
    pb.setPersistentStoragePath(localStorePath);
    m_preview_profile = pb.createProfile("Preview", nullptr);
#endif
    // qDebug() << "WebProfileMgr Preview - StorageName: " << m_preview_profile->storageName();
    // qDebug() << "WebProfileMgr Preview - CachePath: " << m_preview_profile->cachePath();
    
    InitializeDefaultSettings(m_preview_profile->settings());
    SettingsStore ss;
    m_preview_profile->settings()->setDefaultTextEncoding("UTF-8"); 
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, (ss.javascriptOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, (ss.remoteOn() == 1));
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    m_preview_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_preview_profile->setSpellCheckEnabled(true);
    // Use our URLInterceptor
    m_preview_profile->setUrlRequestInterceptor(m_URLint);



    // Inspector Profile
    // ---------------
    // create the profile for Dev Tools Inspector
#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    m_inspector_profile = new QWebEngineProfile();
    m_inspector_profile->setPersistentStoragePath(devToolsStorePath);
#else
    QWebEngineProfileBuilder pb2;
    pb2.setCachePath(InspectorCachePath);
    pb2.setHttpCacheMaximumSize(0); // 0 - means let Qt control it
    pb2.setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    pb2.setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    pb2.setPersistentPermissionsPolicy(QWebEngineProfile::PersistentPermissionsPolicy::StoreOnDisk);
    pb2.setPersistentStoragePath(devToolsStorePath);
    m_inspector_profile = pb2.createProfile("Inspector", nullptr);
#endif
    // qDebug() << "WebProfileMgr Inspector - StorageName: " << m_inspector_profile->storageName();
    // qDebug() << "WebProfileMgr Inspector - CachePath: " << m_inspector_profile->cachePath();

    InitializeDefaultSettings(m_inspector_profile->settings());
    m_inspector_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled,true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, true);
    m_inspector_profile->settings()->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
    // Use our URLInterceptor
    m_inspector_profile->setUrlRequestInterceptor(m_URLint);



    // OneTime Profile
    // ---------------
    // create the profile for OneTime

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
    m_onetime_profile = new QWebEngineProfile();
#else
    m_onetime_profile = QWebEngineProfileBuilder::createOffTheRecordProfile(nullptr);
#endif
    InitializeDefaultSettings(m_onetime_profile->settings());
    m_onetime_profile->settings()->setDefaultTextEncoding("UTF-8");  
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, true);
    
    // Unfortunately the PdfView used for PdfTab now requires both java and LocalStorage work
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, (ss.javascriptOn() == 1));
    m_onetime_profile->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_onetime_profile->setPersistentStoragePath(localStorePath);

    // Use URLInterceptor for protection
    m_onetime_profile->setUrlRequestInterceptor(m_URLint);



    // Default Profile
    // ---------------
    // initialize the defaultProfile to be restrictive for security
    QWebEngineSettings *web_settings = QWebEngineProfile::defaultProfile()->settings();
    InitializeDefaultSettings(web_settings);
    // Use URLInterceptor for protection
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(m_URLint);

}

