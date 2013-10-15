#include <QApplication>

#include "scriptengine.h"
#include "mainwindow.h"
#include "ui_projectwidget.h"
#include "ui_scriptwidget.h"

#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) x

ScriptEngine::ScriptEngine(ScriptWidget *parent) :
    QObject(parent)
{
    mEngine = new QScriptEngine(this);
    mEngine->globalObject().setProperty("scriptEngine", mEngine->newQObject(this));
    mEngineDebugger = new QScriptEngineDebugger(this);
    mEngineDebugger->attachTo(mEngine);
    mScriptWidget = parent;
    externalStop = false;
    mErrorStates << QCHDMAN_PRJSTAT_CRASHED << QCHDMAN_PRJSTAT_ERROR;
}

ScriptEngine::~ScriptEngine()
{
    cleanUpProjects();
    mEngineDebugger->detach();
    delete mEngineDebugger;
    delete mEngine;
}

void ScriptEngine::runScript(QString script)
{
    cleanUpProjects();
    mScriptWidget->on_progressBar_valueChanged(0);
    mEngine->evaluate(script);
    mEngine->collectGarbage();
}

void ScriptEngine::stopScript()
{
    QStringList projectList = mProjectMap.keys();
    stopProjects(projectList.join(","));
    mEngine->abortEvaluation();
    mEngine->collectGarbage();
    cleanUpProjects();
}

void ScriptEngine::log(QString message)
{
    mScriptWidget->log(message);
}

void ScriptEngine::progressSetRange(int min, int max)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::progressSetRange(int min = %1, int max = %2)").arg(min).arg(max)));

    mScriptWidget->ui->progressBar->setRange(min, max);
}

void ScriptEngine::progressSetValue(int value)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::progressSetValue(int value = %1)").arg(value)));

    mScriptWidget->ui->progressBar->setValue(value);
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

    if ( mProjectMap.contains(id) )
        log(tr("warning") + ": ScriptEngine::projectCreateFromFile(): " + tr("project '%1' already exists").arg(id));
    else {
        mProjectMap[id] = new ProjectWidget(0, true, QCHDMAN_PRJ_UNKNOWN, id, this);
        mProjectMap[id]->load(fileName);
    }
}

void ScriptEngine::projectCreateFromString(QString id, QString buffer)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCreateFromString(QString id = %1, QString buffer = %2)").arg(id).arg(buffer)));

    if ( mProjectMap.contains(id) )
        log(tr("warning") + ": ScriptEngine::projectCreateFromString(): " + tr("project '%1' already exists").arg(id));
    else {
        mProjectMap[id] = new ProjectWidget(0, true, QCHDMAN_PRJ_UNKNOWN, id, this);
        mProjectMap[id]->load(QString(), &buffer);
    }
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

void ScriptEngine::projectSetVerifyInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetVerifyInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetVerifyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetVerifyParentInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetVerifyParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyParentInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetVerifyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyParentInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCopyForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyForce(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyInputStartByte(QString id, int byte)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartByte(QString id = %1, bool byte = %2)").arg(id).arg(byte)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputStartByte->setValue(byte);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyInputStartHunk(QString id, int hunk)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartHunk(QString id = %1, bool hunk = %2)").arg(id).arg(hunk)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputStartHunk->setValue(hunk);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyInputBytes(QString id, int bytes)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputBytes(QString id = %1, bool bytes = %2)").arg(id).arg(bytes)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputBytes->setValue(bytes);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyInputHunks(QString id, int hunks)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputHunks(QString id = %1, bool hunks = %2)").arg(id).arg(hunks)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputHunks->setValue(hunks);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectSetCopyCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyCompressors(QString id = %1, QString compressorList = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->copyCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyCompressors(): " + tr("project '%1' doesn't exists").arg(id));
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
                bool started = false;
                bool error = false;
                while ( !started && !error && !externalStop ) {
                    started = mProjectMap[id]->chdmanProc->waitForStarted(QCHDMAN_PROCESS_POLL_TIME);
                    error = started ? mProjectMap[id]->status != QCHDMAN_PRJSTAT_RUNNING : mErrorStates.contains(mProjectMap[id]->status);
                    qApp->processEvents();
                }
                QCHDMAN_SCRIPT_ENGINE_DEBUG(
                    if ( started )
                        log(QString("DEBUG: ScriptEngine::runProjects(): project '%1' started").arg(id));
                    else
                        log(QString("DEBUG: ScriptEngine::runProjects(): failed starting project '%1'").arg(id));
                )
            }
        } else if ( !externalStop )
            log(tr("warning") + ": ScriptEngine::runProjects(): " + tr("project '%1' doesn't exists").arg(id));

        if ( externalStop )
            break;
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
                bool finished = false;
                bool error = false;
                while ( !finished && !error ) {
                    finished = mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME);
                    error = finished ? mProjectMap[id]->status != QCHDMAN_PRJSTAT_RUNNING : mErrorStates.contains(mProjectMap[id]->status);
                    qApp->processEvents();
                }
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
           if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING ) {
               bool finished = false;
               bool error = false;
               while ( !finished && !error && !externalStop ) {
                   finished = mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME);
                   error = finished ? mProjectMap[id]->status != QCHDMAN_PRJSTAT_RUNNING : mErrorStates.contains(mProjectMap[id]->status);
                   qApp->processEvents();
               }
           }
        } else if ( !externalStop )
            log(tr("warning") + ": ScriptEngine::syncProjects(): " + tr("project '%1' doesn't exists").arg(id));

        if ( externalStop )
            break;
    }
}

void ScriptEngine::destroyProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::destroyProjects(QString idList = %1)").arg(idList)));

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) )
            projectDestroy(id);
        else
            log(tr("warning") + ": ScriptEngine::destroyProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}

void ScriptEngine::cleanUpProjects()
{
    foreach (QString id, mProjectMap.keys())
        projectDestroy(id);
    mProjectMap.clear();
}
