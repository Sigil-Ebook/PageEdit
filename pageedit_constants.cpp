#include <QString>
#include <QStringList>
#include "pageedit_constants.h"

// Use compile-time define from CMake file to limit version to one location in the source.
const QString PAGEEDIT_VERSION = QString(PAGEEDIT_FULL_VERSION);

#if _WIN32
#include <QProcessEnvironment>
// Windows barks about getenv or _wgetenv. This elicits no warnings and works with unicode paths
const QString PAGEEDIT_PREFS_DIR = QProcessEnvironment::systemEnvironment().value("PAGEEDIT_PREFS_DIR", "").trimmed();
const QString PATH_LIST_DELIM = ";";
#else
const QString PAGEEDIT_PREFS_DIR = QString(getenv("PAGEEDIT_PREFS_DIR"));
#endif

#if __APPLE__
const QString PATH_LIST_DELIM = ":";
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
const QString PATH_LIST_DELIM = ":";
// Standard build-time location of PageEdit's 'share/pageedit' directory. Set in src/CMakeLists.txt with the line:
// set_source_files_properties( pageedit_constants.cpp PROPERTIES COMPILE_DEFINITIONS PAGEEDIT_SHARE_ROOT="${PAGEEDIT_SHARE_ROOT}" )
const QString pageedit_share_root = QString(PAGEEDIT_SHARE_ROOT);
//const QString mathjax_dir = QString(MATHJAX_DIR);
#endif
