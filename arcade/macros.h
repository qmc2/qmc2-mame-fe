#ifndef MACROS_H
#define MACROS_H

// make a string out of a non-string constant
#define STR(s)                      #s
#define XSTR(s)                     STR(s)

// application and ini related
#define QMC2_ARCADE_ORG_DOMAIN      QString("qmc2.arcadehits.net")
#define QMC2_ARCADE_ORG_NAME        QString("qmc2")
#define QMC2_ARCADE_APP_VERSION     QString(XSTR(QMC2_ARCADE_VERSION))
#define QMC2_ARCADE_APP_TITLE       QObject::tr("QMC2 Arcade Mode")
#define QMC2_ARCADE_APP_NAME        QMC2_ARCADE_ORG_NAME

// dot-path related
#if defined(Q_OS_MAC)
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/.qmc2")
#endif
#define QMC2_ARCADE_DYN_DOT_PATH    (qApp->arguments().indexOf("-config_path") >= 0 && qApp->arguments().indexOf("-config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-config_path") + 1]: QMC2_ARCADE_DOT_PATH)

#endif
