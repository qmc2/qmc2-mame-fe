#include <QApplication>

#include "scriptengine.h"
#include "mainwindow.h"
#include "ui_projectwidget.h"
#include "ui_scriptwidget.h"

//#if defined(QCHDMAN_DEBUG)
#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) x
//#else
//#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) ;
//#endif

extern MainWindow *mainWindow;

ScriptEngine::ScriptEngine(ScriptWidget *parent) :
    QObject(parent)
{
    mEngine = new QScriptEngine(this);
    mEngine->globalObject().setProperty("scriptEngine", mEngine->newQObject(this));
    mEngine->globalObject().setProperty("qchdman", mEngine->newQObject(this));
    mEngineDebugger = new QScriptEngineDebugger(this);
    mEngineDebugger->attachTo(mEngine);
    mScriptWidget = parent;
    externalStop = false;
    mErrorStates << QCHDMAN_PRJSTAT_CRASHED << QCHDMAN_PRJSTAT_ERROR;
    mEntryListIterator = NULL;
}

ScriptEngine::~ScriptEngine()
{
    externalStop = true;
    mEngineDebugger->detach();
    destroyProjects();
    delete mEngineDebugger;
    delete mEngine;
    if ( mEntryListIterator )
        delete mEntryListIterator;
}

void ScriptEngine::runScript(QString script)
{
    externalStop = false;
    destroyProjects();
    mScriptWidget->on_progressBar_valueChanged(0);
    mEngine->evaluate(script);
    mEngine->collectGarbage();
}

void ScriptEngine::stopScript()
{
    externalStop = true;
    mEngine->abortEvaluation();
    mEngine->collectGarbage();
    stopProjects();
    destroyProjects();
}

void ScriptEngine::log(QString message)
{
    if ( !externalStop )
        mScriptWidget->log(message);
}

void ScriptEngine::dirStartEntryList(QString path, QString filter, bool subDirs)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dirStartEntryList(QString path = %1, QString filter = %2, bool subDirs = %3)").arg(path).arg(filter).arg(subDirs)));

    if ( mEntryListIterator )
        delete mEntryListIterator;

    QStringList nameFilters;

    if ( filter.isEmpty() )
        nameFilters << "*";
    else
        nameFilters = filter.split(QRegExp(",.*"), QString::SkipEmptyParts);

    if ( subDirs )
        mEntryListIterator = new QDirIterator(path, nameFilters, QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive);
    else
        mEntryListIterator = new QDirIterator(path, nameFilters, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive);
}

bool ScriptEngine::dirHasNextEntry()
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dirHasNextEntry(")));

    return mEntryListIterator->hasNext();
}

QString ScriptEngine::dirNextEntry()
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dirNextEntry(")));

    if ( mEntryListIterator->hasNext() )
        return mEntryListIterator->next();
    else
        return QString();
}

QStringList ScriptEngine::dirEntryList(QString path, QString filter, bool sort, bool ascending)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dirEntryList(QString path = %1, QString filter = %2, bool sort = %3, bool ascending = %4)").arg(path).arg(filter).arg(sort).arg(ascending)));

    QStringList nameFilters;

    if ( filter.isEmpty() )
        nameFilters << "*";
    else
        nameFilters = filter.split(QRegExp(",.*"), QString::SkipEmptyParts);

    mEntryListDir.setPath(path);
    mEntryListDir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive);
    mEntryListDir.setNameFilters(nameFilters);

    if ( sort ) {
        if ( ascending )
            mEntryListDir.setSorting(QDir::Name);
        else
            mEntryListDir.setSorting(QDir::Name | QDir::Reversed);
    } else
        mEntryListDir.setSorting(QDir::Name | QDir::Unsorted);

    return mEntryListDir.entryList();
}

QStringList ScriptEngine::dirSubDirList(QString path, QString filter, bool sort, bool ascending)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dirSubDirList(QString path = %1, QString filter = %2, bool sort = %3, bool ascending = %4)").arg(path).arg(filter).arg(sort).arg(ascending)));

    QStringList nameFilters;

    if ( filter.isEmpty() )
        nameFilters << "*";
    else
        nameFilters = filter.split(QRegExp(",.*"), QString::SkipEmptyParts);

    mEntryListDir.setPath(path);
    mEntryListDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive);
    mEntryListDir.setNameFilters(nameFilters);

    if ( sort ) {
        if ( ascending )
            mEntryListDir.setSorting(QDir::Name);
        else
            mEntryListDir.setSorting(QDir::Name | QDir::Reversed);
    } else
        mEntryListDir.setSorting(QDir::Name | QDir::Unsorted);

    return mEntryListDir.entryList();
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

void ScriptEngine::projectClone(QString sourceId, QString destinationId)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectClone(QString sourceId = %1, QString sourceId = %2)").arg(sourceId).arg(destinationId)));

    if ( mProjectMap.contains(sourceId) ) {
        if ( mProjectMap.contains(destinationId) )
            log(tr("warning") + ": ScriptEngine::projectClone(): " + tr("project '%1' already exists").arg(destinationId));
        else {
            QString buffer;
            mProjectMap[sourceId]->save(&buffer);
            projectCreateFromString(destinationId, buffer);
        }
    } else
        log(tr("warning") + ": ScriptEngine::projectClone(): " + tr("project '%1' doesn't exists").arg(sourceId));
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

int ScriptEngine::projectReturnCode(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectReturnCode(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) ) {
        if ( mProjectMap[id]->chdmanProc )
            return mProjectMap[id]->lastRc;
        else
            log(tr("warning") + ": ScriptEngine::projectReturnCode(): " + tr("project '%1' hasn't run yet").arg(id));
    } else
        log(tr("warning") + ": ScriptEngine::projectReturnCode(): " + tr("project '%1' doesn't exists").arg(id));

    return -1;
}

void ScriptEngine::projectSetType(QString id, QString type)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetType(QString id = %1, QString type = %2)").arg(id).arg(type)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->setProjectType(MainWindow::projectTypeIndex(type));
    else
        log(tr("warning") + ": ScriptEngine::projectSetType(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetType(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetType(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return MainWindow::projectTypes[mProjectMap[id]->ui->comboBoxProjectType->currentIndex()];
    else {
        log(tr("warning") + ": ScriptEngine::projectGetType(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectToWindow(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectToWindow(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) ) {
        projectCloneToWindow(id);
        projectDestroy(id);
    } else
        log(tr("warning") + ": ScriptEngine::projectToWindow(): " + tr("project '%1' doesn't exists").arg(id));
}

void ScriptEngine::projectCloneToWindow(QString sourceId)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCloneToWindow(QString id = %1)").arg(sourceId)));

    if ( mProjectMap.contains(sourceId) ) {
        ProjectWindow *projectWindow = mainWindow->createProjectWindow(QCHDMAN_MDI_PROJECT);
        projectWindow->projectWidget->fromString(mProjectMap[sourceId]->toString());
    } else
        log(tr("warning") + ": ScriptEngine::projectCloneToWindow(): " + tr("project '%1' doesn't exists").arg(sourceId));
}

// Info

void ScriptEngine::projectSetInfoInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetInfoInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditInfoInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetInfoInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetInfoInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetInfoInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->lineEditInfoInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetInfoInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetInfoVerbose(QString id, bool verbose)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetInfoVerbose(QString id = %1, bool verbose = %2)").arg(id).arg(verbose)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxInfoVerbose->setChecked(verbose);
    else
        log(tr("warning") + ": ScriptEngine::projectSetInfoVerbose(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetInfoVerbose(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetInfoVerbose(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxInfoVerbose->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetInfoVerbose(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

// Verify

void ScriptEngine::projectSetVerifyInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetVerifyInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetVerifyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetVerifyInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetVerifyInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetVerifyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetVerifyParentInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetVerifyParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyParentInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetVerifyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetVerifyParentInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetVerifyParentInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditVerifyParentInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectSetVerifyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

// Copy

void ScriptEngine::projectSetCopyInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCopyInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCopyParentInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCopyParentInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyParentInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCopyOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCopyOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCopyParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCopyParentOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyParentOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCopyParentOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCopyForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCopyForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetCopyForce(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyForce(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxCopyForce->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyForce(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

void ScriptEngine::projectSetCopyInputStartByte(QString id, int byte)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartByte(QString id = %1, bool byte = %2)").arg(id).arg(byte)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputStartByte->setValue(byte);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyInputStartByte(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyInputStartByte(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCopyInputStartByte->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCopyInputStartHunk(QString id, int hunk)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartHunk(QString id = %1, bool hunk = %2)").arg(id).arg(hunk)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputStartHunk->setValue(hunk);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyInputStartHunk(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyInputStartHunk(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCopyInputStartHunk->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCopyInputBytes(QString id, int bytes)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputBytes(QString id = %1, bool bytes = %2)").arg(id).arg(bytes)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputBytes->setValue(bytes);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyInputBytes(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyInputBytes(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCopyInputBytes->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCopyInputHunks(QString id, int hunks)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputHunks(QString id = %1, bool hunks = %2)").arg(id).arg(hunks)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyInputHunks->setValue(hunks);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyInputHunks(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyInputHunks(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCopyInputHunks->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCopyCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyCompressors(QString id = %1, QString compressors = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->copyCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyCompressors(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCopyCompressors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyCompressors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->copyCompressors.join(",");
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyCompressors(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCopyProcessors(QString id, int processors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyProcessors(QString id = %1, int processors = %2)").arg(id).arg(processors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCopyProcessors->setValue(processors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCopyProcessors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyProcessors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyProcessors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCopyProcessors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCopyProcessors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

// CreateRaw

void ScriptEngine::projectSetCreateRawInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateRawInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateRawOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateRawOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateRawParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateRawParentOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawParentOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateRawParentOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateRawForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCreateRawForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetCreateRawForce(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawForce(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxCreateRawForce->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawForce(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

void ScriptEngine::projectSetCreateRawInputStartByte(QString id, int byte)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputStartByte(QString id = %1, bool byte = %2)").arg(id).arg(byte)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawInputStartByte->setValue(byte);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawInputStartByte(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawInputStartByte(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawInputStartByte->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawInputStartHunk(QString id, int hunk)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputStartHunk(QString id = %1, bool hunk = %2)").arg(id).arg(hunk)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawInputStartHunk->setValue(hunk);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawInputStartHunk(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawInputStartHunk(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawInputStartHunk->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawInputBytes(QString id, int bytes)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputBytes(QString id = %1, bool bytes = %2)").arg(id).arg(bytes)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawInputBytes->setValue(bytes);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawInputBytes(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawInputBytes(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawInputBytes->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawInputHunks(QString id, int hunks)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputHunks(QString id = %1, bool hunks = %2)").arg(id).arg(hunks)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawInputHunks->setValue(hunks);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawInputHunks(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawInputHunks(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawInputHunks->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawHunkSize(QString id, int size)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawHunkSize(QString id = %1, bool size = %2)").arg(id).arg(size)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawHunkSize->setValue(size);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawHunkSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawHunkSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawHunkSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawUnitSize(QString id, int size)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawUnitSize(QString id = %1, bool size = %2)").arg(id).arg(size)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawUnitSize->setValue(size);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawUnitSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawUnitSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawUnitSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawUnitSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawUnitSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateRawCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawCompressors(QString id = %1, QString compressors = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createRawCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawCompressors(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateRawCompressors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawCompressors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createRawCompressors.join(",");
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawCompressors(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateRawProcessors(QString id, int processors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawProcessors(QString id = %1, int processors = %2)").arg(id).arg(processors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateRawProcessors->setValue(processors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateRawProcessors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateRawProcessors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateRawProcessors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateRawProcessors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateRawProcessors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

// CreateHD

void ScriptEngine::projectSetCreateHDInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateHDInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateHDOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateHDOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateHDParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateHDParentOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDParentOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateHDParentOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateHDForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCreateHDForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetCreateHDForce(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDForce(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxCreateHDForce->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDForce(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

void ScriptEngine::projectSetCreateHDInputStartByte(QString id, int byte)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputStartByte(QString id = %1, bool byte = %2)").arg(id).arg(byte)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDInputStartByte->setValue(byte);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDInputStartByte(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDInputStartByte(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDInputStartByte->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDInputStartHunk(QString id, int hunk)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputStartHunk(QString id = %1, bool hunk = %2)").arg(id).arg(hunk)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDInputStartHunk->setValue(hunk);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDInputStartHunk(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDInputStartHunk(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDInputStartHunk->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDInputBytes(QString id, int bytes)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputBytes(QString id = %1, bool bytes = %2)").arg(id).arg(bytes)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDInputBytes->setValue(bytes);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDInputBytes(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDInputBytes(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDInputBytes->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDInputHunks(QString id, int hunks)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputHunks(QString id = %1, bool hunks = %2)").arg(id).arg(hunks)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDInputHunks->setValue(hunks);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDInputHunks(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDInputHunks(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDInputHunks->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDHunkSize(QString id, int size)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDHunkSize(QString id = %1, bool size = %2)").arg(id).arg(size)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDHunkSize->setValue(size);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDHunkSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDHunkSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDHunkSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDCompressors(QString id = %1, QString compressors = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createHDCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateHDCompressors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDCompressors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createHDCompressors.join(",");
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateHDProcessors(QString id, int processors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDProcessors(QString id = %1, int processors = %2)").arg(id).arg(processors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDProcessors->setValue(processors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDProcessors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDProcessors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDProcessors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDSectorSize(QString id, int sectorSize)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDSectorSize(QString id = %1, int sectorSize = %2)").arg(id).arg(sectorSize)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDSectorSize->setValue(sectorSize);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDSectorSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDSectorSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDSectorSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDSectorSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDSectorSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDCylinders(QString id, int cylinders)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDCylinders(QString id = %1, int cylinders = %2)").arg(id).arg(cylinders)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDCylinders->setValue(cylinders);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDCylinders(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDCylinders(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDCylinders(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDCylinders->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDCylinders(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDHeads(QString id, int heads)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDHeads(QString id = %1, int heads = %2)").arg(id).arg(heads)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDHeads->setValue(heads);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDHeads(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDHeads(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDHeads(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDHeads->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDHeads(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDSectors(QString id, int sectors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDSectors(QString id = %1, int sectors = %2)").arg(id).arg(sectors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateHDSectors->setValue(sectors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDSectors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateHDSectors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateHDSectors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateHDSectors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateHDSectors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateHDChsFromTemplate(QString id, QString vendorName, QString diskName)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDChsFromTemplate(QString id = %1, QString vendorName = %2, QString diskName = %3)").arg(id).arg(vendorName).arg(diskName)));

    if ( mProjectMap.contains(id) ) {
        QList<DiskGeometry> geoList = MainWindow::hardDiskTemplates[vendorName];
        bool found = false;
        DiskGeometry geo;
        for (int i = 0; i < geoList.count() && !found; i++) {
            geo = geoList[i];
            found = ( geo.name == diskName );
        }
        if ( found ) {
            if ( found ) {
                mProjectMap[id]->ui->spinBoxCreateHDCylinders->setValue(geo.cyls);
                mProjectMap[id]->ui->spinBoxCreateHDHeads->setValue(geo.heads);
                mProjectMap[id]->ui->spinBoxCreateHDSectors->setValue(geo.sectors);
                mProjectMap[id]->ui->spinBoxCreateHDSectorSize->setValue(geo.sectorSize);
                int noneIndex = mProjectMap[id]->ui->comboBoxCreateHDCompression->findText(tr("none"));
                if ( noneIndex >= 0 )
                    mProjectMap[id]->ui->comboBoxCreateHDCompression->setCurrentIndex(noneIndex);
            }
            mProjectMap[id]->ui->comboBoxCreateHDFromTemplate->setCurrentIndex(0);
        } else
            log(tr("warning") + ": ScriptEngine::projectSetCreateHDChsFromTemplate(): " + tr("CHS template for vendorName = '%1', diskName = '%2' doesn't exist").arg(vendorName).arg(diskName));
    } else
        log(tr("warning") + ": ScriptEngine::projectSetCreateHDChsFromTemplate(): " + tr("project '%1' doesn't exists").arg(id));
}

// CreateCD

void ScriptEngine::projectSetCreateCDInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateCDInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateCDOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateCDOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateCDParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateCDParentOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDParentOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateCDParentOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateCDForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCreateCDForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetCreateCDForce(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDForce(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxCreateCDForce->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDForce(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

void ScriptEngine::projectSetCreateCDHunkSize(QString id, int size)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDHunkSize(QString id = %1, bool size = %2)").arg(id).arg(size)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateCDHunkSize->setValue(size);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateCDHunkSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDHunkSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateCDHunkSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateCDCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDCompressors(QString id = %1, QString compressors = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createCDCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateCDCompressors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDCompressors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createCDCompressors.join(",");
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateCDProcessors(QString id, int processors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDProcessors(QString id = %1, int processors = %2)").arg(id).arg(processors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateCDProcessors->setValue(processors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateCDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateCDProcessors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateCDProcessors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateCDProcessors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateCDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

// CreateLD

void ScriptEngine::projectSetCreateLDInputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDInputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateLDInputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDInputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDInputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateLDOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateLDOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateLDParentOutputFile(QString id, QString file)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDParentOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDParentOutputFile->setText(file);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateLDParentOutputFile(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDParentOutputFile(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->lineEditCreateLDParentOutputFile->text();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDParentOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateLDForce(QString id, bool force)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->checkBoxCreateLDForce->setChecked(force);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetCreateLDForce(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDForce(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->checkBoxCreateLDForce->isChecked();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDForce(): " + tr("project '%1' doesn't exists").arg(id));
        return false;
    }
}

void ScriptEngine::projectSetCreateLDInputStartFrame(QString id, int frame)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDInputStartFrame(QString id = %1, bool hunk = %2)").arg(id).arg(frame)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateLDInputStartFrame->setValue(frame);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDInputStartFrame(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateLDInputStartFrame(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDInputStartFrame(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateLDInputStartFrame->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDInputStartFrame(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateLDInputFrames(QString id, int frames)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDInputFrames(QString id = %1, bool hunks = %2)").arg(id).arg(frames)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateLDInputFrames->setValue(frames);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDInputFrames(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateLDInputFrames(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDInputFrames(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateLDInputFrames->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDInputFrames(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateLDHunkSize(QString id, int size)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDHunkSize(QString id = %1, bool size = %2)").arg(id).arg(size)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateLDHunkSize->setValue(size);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateLDHunkSize(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDHunkSize(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateLDHunkSize->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::projectSetCreateLDCompressors(QString id, QString compressors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDCompressors(QString id = %1, QString compressors = %2)").arg(id).arg(compressors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createLDCompressors = compressors.split(",", QString::SkipEmptyParts);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetCreateLDCompressors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDCompressors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->createLDCompressors.join(",");
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDCompressors(): " + tr("project '%1' doesn't exists").arg(id));
        return QString();
    }
}

void ScriptEngine::projectSetCreateLDProcessors(QString id, int processors)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDProcessors(QString id = %1, int processors = %2)").arg(id).arg(processors)));

    if ( mProjectMap.contains(id) )
        mProjectMap[id]->ui->spinBoxCreateLDProcessors->setValue(processors);
    else
        log(tr("warning") + ": ScriptEngine::projectSetCreateLDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCreateLDProcessors(QString id)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCreateLDProcessors(QString id = %1)").arg(id)));

    if ( mProjectMap.contains(id) )
        return mProjectMap[id]->ui->spinBoxCreateLDProcessors->value();
    else {
        log(tr("warning") + ": ScriptEngine::projectGetCreateLDProcessors(): " + tr("project '%1' doesn't exists").arg(id));
        return -1;
    }
}

void ScriptEngine::runProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::runProjects(QString idList = %1)").arg(idList)));

    if ( idList.isEmpty() ) {
        QStringList projectIds = mProjectMap.keys();
        idList = projectIds.join(",");
    }

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        if ( externalStop )
            break;

        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
            if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING )
                log(tr("warning") + ": ScriptEngine::runProjects(): " + tr("project '%1' is already running").arg(id));
            else {
                mProjectMap[id]->on_toolButtonRun_clicked();
                bool started = false;
                bool error = false;
                while ( !started && !error && !externalStop && mProjectMap[id]->chdmanProc ) {
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
    }
}

void ScriptEngine::stopProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::stopProjects(QString idList = %1)").arg(idList)));

    if ( idList.isEmpty() ) {
        QStringList projectIds = mProjectMap.keys();
        idList = projectIds.join(",");
    }

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
            if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING ) {
                mProjectMap[id]->on_toolButtonStop_clicked();
                bool finished = false;
                bool error = false;
                while ( !finished && !error && mProjectMap[id]->chdmanProc ) {
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

    if ( idList.isEmpty() ) {
        QStringList projectIds = mProjectMap.keys();
        idList = projectIds.join(",");
    }

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        if ( externalStop )
            break;

        id = id.trimmed();
        if ( mProjectMap.contains(id) ) {
           if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING ) {
               bool finished = false;
               bool error = false;
               while ( !finished && !error && !externalStop && mProjectMap[id]->chdmanProc ) {
                   finished = mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME);
                   error = finished ? mProjectMap[id]->status != QCHDMAN_PRJSTAT_RUNNING : mErrorStates.contains(mProjectMap[id]->status);
                   qApp->processEvents();
               }
           }
        } else if ( !externalStop )
            log(tr("warning") + ": ScriptEngine::syncProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}

void ScriptEngine::destroyProjects(QString idList)
{
    QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::destroyProjects(QString idList = %1)").arg(idList)));

    if ( idList.isEmpty() ) {
        QStringList projectIds = mProjectMap.keys();
        idList = projectIds.join(",");
    }

    foreach (QString id, idList.split(",", QString::SkipEmptyParts)) {
        id = id.trimmed();
        if ( mProjectMap.contains(id) )
            projectDestroy(id);
        else
            log(tr("warning") + ": ScriptEngine::destroyProjects(): " + tr("project '%1' doesn't exists").arg(id));
    }
}
