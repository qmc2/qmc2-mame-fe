#ifndef MACROS_H
#define MACROS_H

// make a string out of a non-string constant
#define STR(s)                      #s
#define XSTR(s)                     STR(s)

// project file format version
#define QCHDMAN_PRJ_FMT_VERSION     1

// title, author, copyright, ...
#define QCHDMAN_APP_NAME            QString("qchdman")
#define QCHDMAN_APP_TITLE           QString("Qt CHDMAN GUI")
#define QCHDMAN_APP_VERSION         QString(XSTR(QCHDMAN_VERSION))
#define QCHDMAN_APP_COPYRIGHT       tr("Copyright (C) 2012, René Reucher. All Rights Reserved.")
#define QCHDMAN_ORG_DOMAIN          QString("qmc2.arcadehits.net")
#define QCHDMAN_ORG_NAME            QString("qmc2")

// dot-path related
#if defined(Q_OS_MAC)
#define QCHDMAN_DOT_PATH            (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QCHDMAN_DOT_PATH            (QDir::homePath() + "/.qmc2")
#endif
#define QCHDMAN_DYN_DOT_PATH        (qApp->arguments().indexOf("-config_path") >= 0 && qApp->arguments().indexOf("-config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-config_path") + 1]: QCHDMAN_DOT_PATH)

// project types
#define QCHDMAN_PRJ_UNKNOWN         -1
#define QCHDMAN_PRJ_INFO            0
#define QCHDMAN_PRJ_VERIFY          1
#define QCHDMAN_PRJ_COPY            2
#define QCHDMAN_PRJ_CREATE_RAW      3
#define QCHDMAN_PRJ_CREATE_HD       4
#define QCHDMAN_PRJ_CREATE_CD       5
#define QCHDMAN_PRJ_CREATE_LD       6
#define QCHDMAN_PRJ_EXTRACT_RAW     7
#define QCHDMAN_PRJ_EXTRACT_HD      8
#define QCHDMAN_PRJ_EXTRACT_CD      9
#define QCHDMAN_PRJ_EXTRACT_LD      10

// status update timer interval
#define QCHDMAN_STATUS_INTERVAL     500

// max. recent files
#define QCHDMAN_MAX_RECENT_FILES    10

// view-modes
#define QCHDMAN_VIEWMODE_WINDOWED   QMdiArea::SubWindowView
#define QCHDMAN_VIEWMODE_TABBED     QMdiArea::TabbedView

#define QCHDMAN_ITEM_ACTIVE         QString("active")
#define QCHDMAN_ITEM_INACTIVE       QString("inactive")

#endif // MACROS_H
