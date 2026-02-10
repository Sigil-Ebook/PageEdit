/************************************************************************
**
**  Copyright (C) 2019-2026 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2019-2026 Doug Massay
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Grant Drake
**  Copyright (C) 2012 Dave Heiland
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
#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include <QEvent>
#include <QPalette>
#include <QApplication>

class QTimer;

class MainApplication : public QApplication
{
    Q_OBJECT

public:
    MainApplication(int &argc, char **argv);

    bool isDarkMode() { return m_isDark; }
    bool AlwaysUseNFC(){ return m_AlwaysUseNFC; };
    void setFirstInstance(bool val) { m_first_instance = val; };
    bool isFirstInstance() { return m_first_instance; };

signals:
    void applicationActivated();
    void applicationDeactivated();
    void applicationPaletteChanged();

public slots:
    void EmitPaletteChanged();
    void systemColorChanged();

protected:
    bool event(QEvent *pEvent);

private:
    void windowsDarkThemeChange();
    void windowsLightThemeChange();
    bool m_isDark;
    QTimer * m_PaletteChangeTimer;
    bool m_AlwaysUseNFC = true;
    bool m_UseAppPaletteEvent = false;
    bool m_first_instance = false;
};

#endif // MAINAPPLICATION_H


