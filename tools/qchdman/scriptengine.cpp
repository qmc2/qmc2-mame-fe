#include <QApplication>

#include "scriptengine.h"
#include "mainwindow.h"
#include "ui_projectwidget.h"

#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) x

ScriptEngine::ScriptEngine(ScriptWidget *parent) :
    QObject(parent)
{
    mEngine = new QScriptEngine(this);
    mEngine->globalObject().setProperty("scriptEngine", mEngine->newQObject(this));
    mScriptWidget = parent;
    externalStop = false;
}

ScriptEngine::~ScriptEngine()
{
    foreach (QString id, mProjectMap.keys())
        projectDestroy(id);
    mProjectMap.clear();
    delete mEngine;
}

void ScriptEngine::runScript(QString script)
{
    mEngine->evaluate(script);
}

void ScriptEngine::stopScript()
{
    if ( mEngine->isEvaluating() )
        mEngine->abortEvaluation();
}

void ScriptEngine::log(QString message)
{
    mScriptWidget->log(message);
}

void ScriptEngine::projectCreate(QString id, QString type)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCreate(QString id = %1, QString type = %2)").arg(id).arg(type)));

    if ( mProjectMap.contains(id) )
        log(tr("warning") + ": ScriptEngine::projectCreate(): " + tr("project '%1' already exists").arg(id));
    else {
        int typeIndex = MainWindow::projectTypeIndex(type);
        if ( typeIndex >= 0 )
            mProjectMap[id] = new ProjectWidget(0, true, typeIndex, id, this);
        else
            log(tr("warning") + ": ScriptEngine::projectCreate(): " + tr("project type '%1' doesn't exists - valid types are: %2").arg(id).arg(MainWindow::projectTypes.join(", ")));
    }
}

void ScriptEngine::projectCreateFromFile(QString id, QString fileName)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCreateFromFile(QString id = %1, QString fileName = %2)").arg(id).arg(fileName)));

    // FIXME
}

void ScriptEngine::projectCreateFromString(QString id, QString buffer)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCreateFromString(QString id = %1, QString buffer = %2)").arg(id).arg(buffer)));

    // FIXME
}

void ScriptEngine::projectDestroy(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectDestroy(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) ) {
        delete mProjectMap[id];
        mProjectMap.remove(id);
    } else
        log(tr("warning") + ": ScriptEngine::projectDestroy(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectStatus(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectStatus(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->status;
    else
        return QCHDMAN_PRJSTAT_UNKNOWN;
}

void ScriptEngine::projectSetInfoInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetInfoInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditInfoInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetInfoInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetInfoVerbose(QString id, bool verbose)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetInfoVerbose(QString id = %1, bool verbose = %2)").arg(id).arg(verbose)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxInfoVerbose->setChecked(verbose);
    else
        log(tr("warning") + ": ScriptEngine::projectSetInfoVerbose(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::runProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::runProjects(QString idList = %1)").arg(idList)));

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
            if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING )
                log(tr("warning") + ": ScriptEngine::runProjects(): " + tr("project '%1' is already running").arg(id));
            else {
                mProjectMap[id]->on_toolButtonRun_clicked();
                while ( mProjectMap[id]->chdmanProc->waitForStarted(QCHDMAN_PROCESS_POLL_TIME) && !externalStop )
                    qApp->processEvents();
            }
        } else
            log(tr("warning") + ": ScriptEngine::runProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}

void ScriptEngine::stopProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::stopProjects(QString idList = %1)").arg(idList)));

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
            if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING ) {
                mProjectMap[id]->on_toolButtonStop_clicked();
                while ( mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME) && !externalStop )
                    qApp->processEvents();
            }
        } else
            log(tr("warning") + ": ScriptEngine::stopProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}

void ScriptEngine::syncProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::syncProjects(QString idList = %1)").arg(idList)));

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
           if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING )
               while ( mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME) && !externalStop )
                   qApp->processEvents();
        } else
            log(tr("warning") + ": ScriptEngine::syncProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}
