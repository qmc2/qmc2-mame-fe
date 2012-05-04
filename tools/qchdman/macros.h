#ifndef MACROS_H
#define MACROS_H

// title, author, copyright, ...
#define QCHDMAN_TITLE               QString("Qt CHDMAN GUI")
#define QCHDMAN_VERSION             QString("0.1")
#define QCHDMAN_COPYRIGHT           tr("Copyright (C) 2012, René Reucher. All Rights Reserved.")
#define QCHDMAN_ORG_DOMAIN          QString("qmc2.arcadehits.net")
#define QCHDMAN_ORG_NAME            QString("qmc2")
#define QCHDMAN_APP_NAME            QString("qchdman")

// dot-path related
#if defined(Q_OS_MAC)
#define QCHDMAN_DOT_PATH            (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QCHDMAN_DOT_PATH            (QDir::homePath() + "/.qmc2")
#endif
#define QCHDMAN_DYN_DOT_PATH        (qApp->arguments().indexOf("-qmc2_config_path") >= 0 && qApp->arguments().indexOf("-qmc2_config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-qmc2_config_path") + 1]: QCHDMAN_DOT_PATH)

// project types
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

// view-modes
#define QCHDMAN_VIEWMODE_WINDOWED   QMdiArea::SubWindowView
#define QCHDMAN_VIEWMODE_TABBED     QMdiArea::TabbedView

#endif // MACROS_H
