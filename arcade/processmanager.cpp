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

        mProcessMap.insert(mCurrentProcessId++, proc);
        return mCurrentProcessId;
    }
}

void ProcessManager::createTemplateList()
{
    QMC2_LOG_STR(tr("Creating template configuration map"));

    mTemplateList.clear();
    QString templateFilePath = globalConfig->optionsTemplateFile();
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
        QMC2_LOG_STR(tr("FATAL: Can't open options template file"));
}

void ProcessManager::error(QProcess::ProcessError)
{
}

void ProcessManager::finished(int, QProcess::ExitStatus)
{
}

void ProcessManager::readyReadStandardOutput()
{
}

void ProcessManager::readyReadStandardError()
{
}

void ProcessManager::started()
{
}

void ProcessManager::stateChanged(QProcess::ProcessState)
{
}
