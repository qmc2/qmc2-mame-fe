#ifndef MACROS_H
#define MACROS_H

#include <Qt>

// global OS macros for supported target operating systems
#if (defined(Q_OS_UNIX) || defined(Q_OS_LINUX)) && !defined(Q_OS_MAC)
#define QMC2_ARCADE_OS_UNIX
#elif defined(Q_OS_MAC)
#define QMC2_ARCADE_OS_MAC
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#define QMC2_ARCADE_OS_WIN
#else
#error "Target OS is not supported -- QMC2 Arcade currently supports Linux/UNIX, Windows and Mac OS X!"
#endif

#include <QStringList>
#include <QString>
#include <QRegExp>
#include <QTime>
#include <QDir>
#include <stdio.h>

// make a string out of a non-string constant
#define STR(s)                      #s
#define XSTR(s)                     STR(s)

// application and ini related
#define QMC2_ARCADE_ORG_DOMAIN      QString("qmc2.arcadehits.net")
#define QMC2_ARCADE_ORG_NAME        QString("qmc2")
#define QMC2_ARCADE_APP_VERSION     QString(XSTR(QMC2_ARCADE_VERSION))
#define QMC2_ARCADE_APP_TITLE       QObject::tr("QMC2 Arcade")
#define QMC2_ARCADE_APP_NAME        QMC2_ARCADE_ORG_NAME

// dot-path related
#if defined(Q_OS_MAC)
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/.qmc2")
#endif
#define QMC2_ARCADE_DYN_DOT_PATH    (qApp->arguments().indexOf("-config_path") >= 0 && qApp->arguments().indexOf("-config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-config_path") + 1]: QMC2_ARCADE_DOT_PATH)

// ROM states
#define QMC2_ARCADE_ROMSTATE_C      0
#define QMC2_ARCADE_ROMSTATE_M      1
#define QMC2_ARCADE_ROMSTATE_I      2
#define QMC2_ARCADE_ROMSTATE_N      3
#define QMC2_ARCADE_ROMSTATE_U      4

// emulator modes
#define QMC2_ARCADE_MODE_MAME       0
#define QMC2_ARCADE_MODE_MESS       1
#define QMC2_ARCADE_MODE_UME        2
#define QMC2_ARCADE_MODE_UNK        3

// additional command line arguments
// -emu <emu> ([mame], mess, ume)
#define QMC2_ARCADE_CLI_EMU_MAME    (qApp->arguments().indexOf("-emu") >= 0 && qApp->arguments().count() > qApp->arguments().indexOf("-emu") + 1 ? qApp->arguments()[qApp->arguments().indexOf("-emu") + 1].toLower() == "mame" : false)
#define QMC2_ARCADE_CLI_EMU_MESS    (qApp->arguments().indexOf("-emu") >= 0 && qApp->arguments().count() > qApp->arguments().indexOf("-emu") + 1 ? qApp->arguments()[qApp->arguments().indexOf("-emu") + 1].toLower() == "mess" : false)
#define QMC2_ARCADE_CLI_EMU_UME     (qApp->arguments().indexOf("-emu") >= 0 && qApp->arguments().count() > qApp->arguments().indexOf("-emu") + 1 ? qApp->arguments()[qApp->arguments().indexOf("-emu") + 1].toLower() == "ume" : false)
#define QMC2_ARCADE_CLI_EMU_UNK     (!QMC2_ARCADE_CLI_EMU_MAME && !QMC2_ARCADE_CLI_EMU_MESS && !QMC2_ARCADE_CLI_EMU_UME)
#define QMC2_ARCADE_CLI_EMU_INV     (qApp->arguments().indexOf("-emu") >= 0 && qApp->arguments().count() == qApp->arguments().indexOf("-emu") + 1)
// -theme <theme> ([ToxicWaste])
#define QMC2_ARCADE_CLI_THEME       (qApp->arguments().indexOf("-theme") >= 0 && qApp->arguments().count() > qApp->arguments().indexOf("-theme") + 1 ? qApp->arguments()[qApp->arguments().indexOf("-theme") + 1] : "ToxicWaste")
#define QMC2_ARCADE_CLI_THEME_INV   (qApp->arguments().indexOf("-theme") >= 0 && qApp->arguments().count() == qApp->arguments().indexOf("-theme") + 1)
// -h|-?|-help
#define QMC2_ARCADE_CLI_HELP        (qApp->arguments().indexOf(QRegExp("(-h|-\\?|-help)")) >= 0)
#define QMC2_ARCADE_CLI_INVALID     (QMC2_ARCADE_CLI_EMU_INV || QMC2_ARCADE_CLI_THEME_INV)

// debugging / logging macros
#define QMC2_PRINT_TXT(t)           printf("%s: %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #t)
#define QMC2_PRINT_STR(s)           printf("%s: %s = %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #s, (const char *)s.toLocal8Bit())
#define QMC2_PRINT_CSTR(s)          printf("%s: %s = %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #s, (const char *)s)
#define QMC2_PRINT_PTR(p)           printf("%s: %s = %p\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #p, p)
#define QMC2_PRINT_INT(i)           printf("%s: %s = %ld\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #i, i)
#define QMC2_PRINT_HEX(x)           printf("%s: %s = %x\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #x, x)
#define QMC2_PRINT_BOOL(b)          printf("%s: %s = %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #b, b ? "true" : "false")
#define QMC2_PRINT_STRLST(l)        for (int i = 0; i < l.count(); i++) printf("%s: %s[%ld] = %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), #l, i, (const char *)l[i].toLocal8Bit())
#define QMC2_LOG_STR(s)             printf("%s: %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), (const char *)s.toLocal8Bit()); fflush(stdout);
#define QMC2_LOG_STR_NO_TIME(s)     printf("%s\n", (const char *)s.toLocal8Bit()); fflush(stdout);
#define QMC2_LOG_CSTR(s)            printf("%s: %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), (const char *)s); fflush(stdout);
#define QMC2_LOG_CSTR_NO_TIME(s)    printf("%s\n", (const char *)s); fflush(stdout);

#endif
