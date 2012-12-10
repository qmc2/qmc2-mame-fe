#include <QXmlStreamReader>
#include "processmanager.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

ProcessManager::ProcessManager(QObject *parent) :
    QObject(parent)
{
    mCurrentProcessId = 0;
}

ProcessManager::~ProcessManager()
{
}

int ProcessManager::startEmulator(QString id)
{
    if ( id.isEmpty() )
        return -1;
    else {
        // FIXME
        QProcess *proc = new QProcess(this);
        QStringList args;

        connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
        connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
        connect(proc, SIGNAL(started()), this, SLOT(started()));
        connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));

        mProcessMap.insert(mCurrentProcessId, proc);
        return mCurrentProcessId++;
    }
}

void ProcessManager::createTemplateList()
{
    QString templateFilePath = globalConfig->optionsTemplateFile();
    QMC2_LOG_STR(tr("Loading configuration template from '%1'").arg(templateFilePath));
    mTemplateList.clear();
    QFile templateFile(templateFilePath);
    if ( templateFile.open(QFile::ReadOnly) ) {
        QXmlStreamReader xmlReader(&templateFile);
        while ( !xmlReader.atEnd() ) {
            xmlReader.readNext();
            if ( xmlReader.hasError() ) {
                QMC2_LOG_STR(tr("FATAL: XML error reading template: '%1' in file '%2' at line %3, column %4").
                             arg(xmlReader.errorString()).arg(templateFilePath).arg(xmlReader.lineNumber()).arg(xmlReader.columnNumber()));
            } else {
                if ( xmlReader.isStartElement() ) {
                    QString elementType = xmlReader.name().toString();
                    QXmlStreamAttributes attributes = xmlReader.attributes();
                    QString name = attributes.value("name").toString();
                    if ( elementType == "option" ) {
                        bool ignore = false;
                        QString shortName;
                        if ( attributes.hasAttribute("shortname") )
                            shortName = attributes.value("shortname").toString();
                        if ( attributes.hasAttribute("ignore") )
                            ignore = attributes.value("ignore") == "true";
                        if ( attributes.hasAttribute(QString("ignore.%1").arg(QMC2_ARCADE_OS_NAME)) )
                            ignore = attributes.value(QString("ignore.%1").arg(QMC2_ARCADE_OS_NAME)) == "true";
                        if ( !ignore ) {
                            QString type = attributes.value("type").toString();
                            QString defaultValue;
                            if ( attributes.hasAttribute(QString("default.%1").arg(QMC2_ARCADE_OS_NAME)) )
                                defaultValue = attributes.value(QString("default.%1").arg(QMC2_ARCADE_OS_NAME)).toString();
                            else
                                defaultValue = attributes.value("default").toString();
                            mTemplateList.append(EmulatorOption(name, shortName, type, defaultValue, QString()));
                        }
                    }
                }
            }
        }
        templateFile.close();
    } else
        QMC2_LOG_STR(tr("FATAL: Can't open the configuration template file: reason = %1").arg(fileErrorToString(templateFile.error())));
}

QString ProcessManager::fileErrorToString(QFile::FileError errorCode)
{
    switch ( errorCode ) {
    case QFile::NoError:
        return tr("No error occurred");
    case QFile::ReadError:
        return tr("An error occurred when reading from the file");
    case QFile::WriteError:
        return tr("An error occurred when writing to the file");
    case QFile::FatalError:
        return tr("A fatal error occurred");
    case QFile::ResourceError:
        return tr("A resource error occurred");
    case QFile::OpenError:
        return tr("The file could not be opened");
    case QFile::AbortError:
        return tr("The operation was aborted");
    case QFile::TimeOutError:
        return tr("A timeout occurred");
    case QFile::UnspecifiedError:
        return tr("An unspecified error occurred");
    case QFile::RemoveError:
        return tr("The file could not be removed");
    case QFile::RenameError:
        return tr("The file could not be renamed");
    case QFile::PositionError:
        return tr("The position in the file could not be changed");
    case QFile::ResizeError:
        return tr("The file could not be resized");
    case QFile::PermissionsError:
        return tr("The file could not be accessed");
    case QFile::CopyError:
        return tr("The file could not be copied");
    default:
        return tr("An unknown error occurred");
    }
}

QString ProcessManager::processErrorToString(QProcess::ProcessError errorCode)
{
    switch ( errorCode ) {
    case QProcess::FailedToStart:
        return tr("The process failed to start");
    case QProcess::Crashed:
        return tr("The process crashed");
    case QProcess::Timedout:
        return tr("A timeout occurred");
    case QProcess::WriteError:
        return tr("An error occurred when attempting to write to the process");
    case QProcess::ReadError:
        return tr("An error occurred when attempting to read from the process");
    case QProcess::UnknownError:
    default:
        return tr("An unknown error occurred");
    }
}

QString ProcessManager::processStateToString(QProcess::ProcessState state)
{
    switch ( state ) {
    case QProcess::NotRunning:
        return tr("Not running");
    case QProcess::Starting:
        return tr("Starting");
    case QProcess::Running:
        return tr("Running");
    default:
        return tr("Unknown");
    }
}

void ProcessManager::error(QProcess::ProcessError errorCode)
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_LOG_STR(tr("Emulator #%1 error: reason = %2").arg(procID).arg(processErrorToString(errorCode)));
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_LOG_STR(tr("Emulator #%1 finished: exitCode = %2, exitStatus = %3").arg(procID).arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
}

void ProcessManager::readyReadStandardOutput()
{
//    QProcess *proc = (QProcess *)sender();
//    int procID = mProcessMap.key(proc);
}

void ProcessManager::readyReadStandardError()
{
//    QProcess *proc = (QProcess *)sender();
//    int procID = mProcessMap.key(proc);
}

void ProcessManager::started()
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_LOG_STR(tr("Emulator #%1 started").arg(procID));
    emit emulatorStarted(procID);
}

void ProcessManager::stateChanged(QProcess::ProcessState newState)
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_LOG_STR(tr("Emulator #%1 state changed: newState = %2").arg(procID).arg(processStateToString(newState)));
    emit emulatorFinished(procID);
}
