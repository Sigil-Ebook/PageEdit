/************************************************************************
**
**  Copyright (C) 2019-2024 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2019-2024 Doug Massay
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Grant Drake
**  Copyright (C) 2012      Dave Heiland
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

#include <QApplication>
#include <QTimer>
#include <QStyleFactory>
#include <QStyle>

#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
#include <QStyleHints>
#endif

#include <QPalette>
#include <QDebug>

#include "MainApplication.h"
#include "SettingsStore.h"
#include "PEDarkStyle.h"

#define DBG if(0)

MainApplication::MainApplication(int &argc, char **argv)
    : QApplication(argc, argv),
      m_isDark(false),
      m_PaletteChangeTimer(new QTimer()),
      m_AlwaysUseNFC(true)
{
    // Do this only once early on in the PageEdit startup
    if (qEnvironmentVariableIsSet("PAGEEDIT_DISABLE_NFC_NORMALIZATION")) m_AlwaysUseNFC = false;

    // Keep track on our own of dark or light
    m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;

    // Determine how to best detect dark vs light theme changes based on Qt Version, platform,
    // with an env variable override for Linux and other platforms
    m_UseAppPaletteEvent=true;
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
    m_UseAppPaletteEvent=false;
#else 
    if (qEnvironmentVariableIsSet("PAGEEDIT_USE_COLORSCHEME_CHANGED")) m_UseAppPaletteEvent=false;
#endif
#endif
    
    if (m_UseAppPaletteEvent) {
        // Set up PaletteChangeTimer to absorb multiple QEvents
        m_PaletteChangeTimer->setSingleShot(true);
        m_PaletteChangeTimer->setInterval(50);
        connect(m_PaletteChangeTimer, SIGNAL(timeout()),this, SLOT(systemColorChanged()));
        m_PaletteChangeTimer->stop();
    } else {

// gcc compiler is not smart enough to optimize the else clause away based on qt version < 6.5.0
// so we still need this ifdef
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)

        // Connect Qt system color scheme change signal to reporting mechanism
        DBG qDebug() << "initial styleHints colorScheme: " << styleHints()->colorScheme();
        if (styleHints()->colorScheme() == Qt::ColorScheme::Unknown) {
            m_isDark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
        } else {
            m_isDark = styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        }
        connect(styleHints(), &QStyleHints::colorSchemeChanged, this, [this]() {
                    MainApplication::systemColorChanged();
            });

#endif  // Qt 6.5.0 or greater

    }
}

bool MainApplication::event(QEvent *pEvent)
{
    if (pEvent->type() == QEvent::ApplicationActivate) {
        emit applicationActivated();
    } else if (pEvent->type() == QEvent::ApplicationDeactivate) {
        emit applicationDeactivated();
    }
    if (pEvent->type() == QEvent::ApplicationPaletteChange) {
        if (m_UseAppPaletteEvent) {
            DBG qDebug() << "Application Palette Changed Event";
            // Note: can be generated multiple times in a row so use timer to absorb them
            if (m_PaletteChangeTimer->isActive()) m_PaletteChangeTimer->stop();
            m_PaletteChangeTimer->start();
        }
    }
    return QApplication::event(pEvent);
}

void MainApplication::EmitPaletteChanged()
{
    DBG qDebug() << "emitting PageEdit's applicationPaletteChanged";
    emit applicationPaletteChanged();
}

void MainApplication::systemColorChanged()
{
    DBG qDebug() << "reached systemColorChanged";
    bool isdark = qApp->palette().color(QPalette::Active,QPalette::WindowText).lightness() > 128;
    if (m_UseAppPaletteEvent) {
        m_PaletteChangeTimer->stop();
        DBG qDebug() << "    via App PaletteEvent timer signal with m_isDark: " << m_isDark;
        DBG qDebug() << "        and current palette test isdark: " << isdark;
        // in reality we really should not care if light or dark, just that palette changed
        // but this is where we are at now
        m_isDark = isdark;
    } else {
      
// gcc compiler is not smart enough to optimize the else clause away based on qt version < 6.5.0
// so we still need this ifdef
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)

        DBG qDebug() << "    via Qt ColorSDchemeChanged Signal";
        switch (styleHints()->colorScheme())
        {
            case Qt::ColorScheme::Light:
                DBG qDebug() << "Color Scheme Changed to Light Theme";
                m_isDark = false;
#ifdef Q_OS_WIN32
                windowsLightThemeChange();
#endif
                break;

            case Qt::ColorScheme::Unknown:
                DBG qDebug() << "Color Scheme Changed to Unknown Theme";
                m_isDark = isdark;
                break;

            case Qt::ColorScheme::Dark:
                DBG qDebug() << "Color Scheme Changed to Dark Theme";
                m_isDark = true;
#ifdef Q_OS_WIN32
                windowsDarkThemeChange();
#endif
                break;
        }

#endif // Qt 6.5.0 or greater

    }
    QTimer::singleShot(0, this, SLOT(EmitPaletteChanged()));
}


void MainApplication::windowsDarkThemeChange()
{
    SettingsStore settings;
    if (settings.uiUseCustomDarkTheme()) {
        QStyle* astyle = QStyleFactory::create("Fusion");
        //setStyle(astyle);
        // modify windows sigil palette to our dark
        QStyle* bstyle;
        bstyle = new PEDarkStyle(astyle);
        setStyle(bstyle);
        setPalette(style()->standardPalette());
    }
}

void MainApplication::windowsLightThemeChange()
{
    SettingsStore settings;
    if (settings.uiUseCustomDarkTheme()) {
        // Windows Fusion light mode
        QStyle* astyle = QStyleFactory::create("Fusion");
        setStyle(astyle);
        setPalette(style()->standardPalette());
    }
}
