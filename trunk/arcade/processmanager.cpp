#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QXmlStreamReader>
#include <QFileInfo>

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
    foreach (QProcess *proc, mProcessMap) {
        proc->terminate();
        proc->waitForFinished(500);
        if ( proc->state() == QProcess::Running ) {
            proc->kill();
            proc->waitForFinished(250);
        }
    }
}

int ProcessManager::startEmulator(QString id)
{
    if ( id.isEmpty() )
        return -1;
    else {
        QProcess *proc = new QProcess(this);
        QStringList args;

        foreach (EmulatorOption emuOpt, mTemplateList) {
            QString globalOptionKey = globalConfig->emulatorPrefix + "/Configuration/Global/" + emuOpt.name;
            QString localOptionKey = globalConfig->emulatorPrefix + QString("/Configuration/%1/").arg(id) + emuOpt.name;

            switch ( emuOpt.type ) {
                case QMC2_ARCADE_EMUOPT_INT: {
                    int dv = emuOpt.dvalue.toInt();
                    int gv = globalConfig->value(globalOptionKey, dv).toInt();
                    int v = globalConfig->value(localOptionKey, gv).toInt();
                    if ( v != dv )
                        args << QString("-%1").arg(emuOpt.name) << QString("%1").arg(v);
                    break;
                }
                case QMC2_ARCADE_EMUOPT_FLOAT: {
                    double dv = emuOpt.dvalue.toDouble();
                    double gv = globalConfig->value(globalOptionKey, dv).toDouble();
                    double v = globalConfig->value(localOptionKey, gv).toDouble();
                    if ( v != dv )
                      args << QString("-%1").arg(emuOpt.name) << QString::number(v);
                    break;
                }
                case QMC2_ARCADE_EMUOPT_BOOL: {
                    bool dv = (emuOpt.dvalue == "true");
                    bool gv = globalConfig->value(globalOptionKey, dv).toBool();
                    bool v = globalConfig->value(localOptionKey, gv).toBool();
                    if ( v != dv ) {
                        if ( v )
                            args << QString("-%1").arg(emuOpt.name);
                        else
                            args << QString("-no%1").arg(emuOpt.name);
                    }
                    break;
                }
                case QMC2_ARCADE_EMUOPT_STRING:
                default: {
                    QString dv = emuOpt.dvalue;
                    QString gv = globalConfig->value(globalOptionKey, dv).toString();
                    QString v = globalConfig->value(localOptionKey, gv).toString();
                    if ( v != dv )
                        args << QString("-%1").arg(emuOpt.name) << v.replace("~", "$HOME");
                    break;
                }
            }
        }

        args << id;

        connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
        connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
        connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
        connect(proc, SIGNAL(started()), this, SLOT(started()));
        connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));

        if ( !globalConfig->emulatorWorkingDirectory().isEmpty() )
            proc->setWorkingDirectory(globalConfig->emulatorWorkingDirectory());

        mProcessMap.insert(mCurrentProcessId, proc);
        proc->start(globalConfig->emulatorExecutablePath(), args);

        return mCurrentProcessId++;
    }
}

void ProcessManager::createTemplateList()
{
    QString templateFilePath = QFileInfo(globalConfig->optionsTemplateFile()).absoluteFilePath();
    QMC2_ARCADE_LOG_STR(tr("Loading configuration template from '%1'").arg(QDir::toNativeSeparators(templateFilePath)));
    mTemplateList.clear();
    QFile templateFile(templateFilePath);
    if ( templateFile.open(QFile::ReadOnly) ) {
        QXmlStreamReader xmlReader(&templateFile);
        while ( !xmlReader.atEnd() ) {
            xmlReader.readNext();
            if ( xmlReader.hasError() ) {
                QMC2_ARCADE_LOG_STR(tr("FATAL: XML error reading template: '%1' in file '%2' at line %3, column %4").
                             arg(xmlReader.errorString()).arg(QDir::toNativeSeparators(templateFilePath)).arg(xmlReader.lineNumber()).arg(xmlReader.columnNumber()));
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
        qSort(mTemplateList.begin(), mTemplateList.end(), EmulatorOption::lessThan);
        QMC2_ARCADE_LOG_STR(QString(tr("Done (loading configuration template from '%1')").arg(QDir::toNativeSeparators(templateFilePath)) + " - " + tr("%n option(s) loaded", "", mTemplateList.count())));
    } else
        QMC2_ARCADE_LOG_STR(tr("FATAL: Can't open the configuration template file: reason = %1").arg(fileErrorToString(templateFile.error())));
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
    QMC2_ARCADE_LOG_STR(tr("Emulator #%1 error: reason = %2").arg(procID).arg(processErrorToString(errorCode)));
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_ARCADE_LOG_STR(tr("Emulator #%1 finished: exitCode = %2, exitStatus = %3").arg(procID).arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
    mProcessMap.remove(procID);
}

void ProcessManager::readyReadStandardOutput()
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QString data = proc->readAllStandardOutput();
#if defined(QMC2_ARCADE_OS_WIN)
    QString separator = "\r\n";
#else
    QString separator = "\n";
#endif
    foreach (QString line, data.split(separator)) {
        if ( !line.isEmpty() ) {
            QMC2_ARCADE_LOG_STR(tr("Emulator #%1 stdout: %2").arg(procID).arg(line));
        }
    }
}

void ProcessManager::readyReadStandardError()
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QString data = proc->readAllStandardError();
#if defined(QMC2_ARCADE_OS_WIN)
    QString separator = "\r\n";
#else
    QString separator = "\n";
#endif
    foreach (QString line, data.split(separator)) {
        if ( !line.isEmpty() ) {
            QMC2_ARCADE_LOG_STR(tr("Emulator #%1 stderr: %2").arg(procID).arg(line));
        }
    }
}

void ProcessManager::started()
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_ARCADE_LOG_STR(tr("Emulator #%1 started").arg(procID));
    emit emulatorStarted(procID);
}

void ProcessManager::stateChanged(QProcess::ProcessState newState)
{
    QProcess *proc = (QProcess *)sender();
    int procID = mProcessMap.key(proc);
    QMC2_ARCADE_LOG_STR(tr("Emulator #%1 state changed: newState = %2").arg(procID).arg(processStateToString(newState)));
    emit emulatorFinished(procID);
}
