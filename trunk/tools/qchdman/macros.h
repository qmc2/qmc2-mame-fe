#ifndef MACROS_H
#define MACROS_H

#include <Qt>

// global OS macros for supported target operating systems
#if (defined(Q_OS_UNIX) || defined(Q_OS_LINUX)) && !defined(Q_OS_MAC)
#define QCHDMAN_OS_UNIX
#if defined(Q_OS_LINUX)
#define QCHDMAN_OS_LINUX
#define QCHDMAN_OS_NAME             QString("Linux")
#else
#define QCHDMAN_OS_NAME             QString("UNIX")
#endif
#elif defined(Q_OS_MAC)
#define QCHDMAN_OS_MAC
#define QCHDMAN_OS_NAME             QString("Darwin")
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#define QCHDMAN_OS_WIN
#define QCHDMAN_OS_NAME             QString("Windows")
#else
#warning "Target OS is not supported -- Qt CHDMAN GUI currently supports Linux/UNIX, Windows and Mac OS X!"
#define QCHDMAN_OS_UNKNOWN
#define QCHDMAN_OS_NAME             QString("Unknown")
#endif

// make a string out of a non-string constant
#define STR(s)                      #s
#define XSTR(s)                     STR(s)

// project & script file format versions
#define QCHDMAN_PRJ_FMT_VERSION     1
#define QCHDMAN_SCR_FMT_VERSION     1

// app name, title, version, ...
#define QCHDMAN_APP_NAME            QString("qchdman")
#define QCHDMAN_APP_TITLE           QString("Qt CHDMAN GUI")
#define QCHDMAN_APP_VERSION         QString(XSTR(QCHDMAN_VERSION))
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
#define QCHDMAN_PRJ_DUMP_META       11
#define QCHDMAN_PRJ_ADD_META        12
#define QCHDMAN_PRJ_DEL_META        13

// widget types
#define QCHDMAN_TYPE_NONE           -1
#define QCHDMAN_TYPE_LINEEDIT       0
#define QCHDMAN_TYPE_SPINBOX        1
#define QCHDMAN_TYPE_CHECKBOX       2
#define QCHDMAN_TYPE_COMBOBOX       3

// MDI sub-window types
#define QCHDMAN_MDI_PROJECT         0
#define QCHDMAN_MDI_SCRIPT          1

// script (tree-widget) column indexes
#define QCHDMAN_SCRCOL_NAME         0
#define QCHDMAN_SCRCOL_TYPE         1
#define QCHDMAN_SCRCOL_SUBTYPE      2
#define QCHDMAN_SCRCOL_TRANSITION   3
#define QCHDMAN_SCRCOL_STATUS       4

// status update timer interval
#define QCHDMAN_STATUS_INTERVAL     500

// status message timeout
#define QCHDMAN_STATUS_MSGTIME      2000

// max. recent files
#define QCHDMAN_MAX_RECENT_FILES    10

// wait time between consecutive kill retries
#define QCHDMAN_KILL_WAIT           100

// time between process state polls when synchronizing projects
#define QCHDMAN_PROCESS_POLL_TIME   50

// script engine's "process events" interval
#define QCHDMAN_SCR_EVENT_INTERVAL  10

// view modes
#define QCHDMAN_VIEWMODE_WINDOWED   QMdiArea::SubWindowView
#define QCHDMAN_VIEWMODE_TABBED     QMdiArea::TabbedView

// item states
#define QCHDMAN_ITEM_ACTIVE         QString("active")
#define QCHDMAN_ITEM_INACTIVE       QString("inactive")

// standard sizes
#define QCHDMAN_ONE_KILOBYTE        1024
#define QCHDMAN_ONE_MEGABYTE        1048576
#define QCHDMAN_ONE_GIGABYTE        1073741824
#define QCHDMAN_ONE_TERABYTE        1099511627776
#define QCHDMAN_1K                  QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_4K                  4 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_8K                  8 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_16K                 16 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_32K                 32 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_64K                 64 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_128K                128 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_256K                256 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_512K                512 * QCHDMAN_ONE_KILOBYTE
#define QCHDMAN_1M                  QCHDMAN_ONE_MEGABYTE
#define QCHDMAN_1G                  QCHDMAN_ONE_GIGABYTE
#define QCHDMAN_1T                  QCHDMAN_ONE_TERABYTE

// project states (for scripting)
#define QCHDMAN_PRJSTAT_UNKNOWN     "unknown"
#define QCHDMAN_PRJSTAT_IDLE        "idle"
#define QCHDMAN_PRJSTAT_RUNNING     "running"
#define QCHDMAN_PRJSTAT_FINISHED    "finished"
#define QCHDMAN_PRJSTAT_TERMINATED  "terminated"
#define QCHDMAN_PRJSTAT_CRASHED     "crashed"
#define QCHDMAN_PRJSTAT_ERROR       "error"

// script-log / project-monitor tab indexes
#define QCHDMAN_SCRIPT_LOG_INDEX    0
#define QCHDMAN_SCRIPT_MON_INDEX    1

// project-monitor columns
#define QCHDMAN_PRJMON_ID           0
#define QCHDMAN_PRJMON_PROGRESS     1
#define QCHDMAN_PRJMON_COMMAND      2

// debugging macros
#define QCHDMAN_PRINT_TXT(t)        printf("%s\n", #t); fflush(stdout);
#define QCHDMAN_PRINT_STR(s)        printf("%s = %s\n", #s, (const char *)s.toUtf8()); fflush(stdout);
#define QCHDMAN_PRINT_CSTR(s)       printf("%s = %s\n", #s, (const char *)s); fflush(stdout);
#define QCHDMAN_PRINT_PTR(p)        printf("%s = %p\n", #p, p); fflush(stdout);
#define QCHDMAN_PRINT_INT(i)        printf("%s = %ld\n", #i, i); fflush(stdout);
#define QCHDMAN_PRINT_HEX(x)        printf("%s = %x\n", #x, x); fflush(stdout);
#define QCHDMAN_PRINT_BOOL(b)       printf("%s = %s\n", #b, b ? "true" : "false"); fflush(stdout);
#define QCHDMAN_PRINT_STRLST(l)     for (int i = 0; i < l.count(); i++) printf("%s[%ld] = %s\n", #l, i, (const char *)l[i].toUtf8()); fflush(stdout);

#endif // MACROS_H
