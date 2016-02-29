#include <QApplication>
#include <QProcess>
#include <QInputDialog>
#include <QFileDialog>
#include <QFontMetrics>

#include "scriptengine.h"
#include "mainwindow.h"
#include "qchdmansettings.h"
#include "ui_projectwidget.h"
#include "ui_scriptwidget.h"

#if defined(QCHDMAN_DEBUG)
#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) x
#else
#define QCHDMAN_SCRIPT_ENGINE_DEBUG(x) ;
#endif

extern MainWindow *mainWindow;
extern QtChdmanGuiSettings *globalConfig;

ScriptEngine::ScriptEngine(ScriptWidget *parent) :
	QObject(parent)
{
	mEngine = new QScriptEngine(this);
	mEngine->globalObject().setProperty("scriptEngine", mEngine->newQObject(this));
	mEngine->globalObject().setProperty("qchdman", mEngine->newQObject(this));
	mEngine->setProcessEventsInterval(QCHDMAN_SCR_EVENT_INTERVAL);
	mScriptWidget = parent;
	externalStop = mInputOk = false;
	mErrorStates << QCHDMAN_PRJSTAT_CRASHED << QCHDMAN_PRJSTAT_ERROR;
	mEntryListIterator = NULL;
	mEngineDebugger = NULL;
	mRunningProjects = 0;
}

ScriptEngine::~ScriptEngine()
{
	externalStop = true;
	if ( mEngineDebugger ) {
		mEngineDebugger->standardWindow()->close();
		qApp->processEvents();
		mEngineDebugger->detach();
		delete mEngineDebugger;
		mEngineDebugger = NULL;
	}
	if ( !mProjectMap.isEmpty() )
		destroyProjects();
	delete mEngine;
	if ( mEntryListIterator )
		delete mEntryListIterator;
}

void ScriptEngine::runScript(QString script)
{
	externalStop = mInputOk = false;
	if ( !mProjectMap.isEmpty() )
		destroyProjects();
	mScriptWidget->on_progressBar_valueChanged(0);
	if ( mEngineDebugger ) {
		mEngineDebugger->standardWindow()->close();
		qApp->processEvents();
		mEngineDebugger->detach();
		delete mEngineDebugger;
		mEngineDebugger = NULL;
	}
	disconnectScriptSignals();
	mEngineDebugger = new QScriptEngineDebugger(this);
	mEngineDebugger->attachTo(mEngine);
	mEngine->evaluate(script);
	mEngine->collectGarbage();
}

void ScriptEngine::stopScript()
{
	externalStop = true;
	mInputOk = false;
	mEngine->abortEvaluation();
	mEngine->collectGarbage();
	disconnectScriptSignals();
	if ( !mProjectMap.isEmpty() ) {
		stopProjects();
		destroyProjects();
	}
	if ( mEngineDebugger ) {
		mEngineDebugger->standardWindow()->close();
		qApp->processEvents();
		mEngineDebugger->detach();
		delete mEngineDebugger;
		mEngineDebugger = NULL;
	}
}

void ScriptEngine::disconnectScriptSignals()
{
	disconnect(SIGNAL(projectStarted(QString)));
	disconnect(SIGNAL(projectFinished(QString)));
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

void ScriptEngine::dumpHardDiskTemplates()
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::dumpHardDiskTemplates()")));

	foreach (QString vendorName, MainWindow::hardDiskTemplates.keys())
		foreach (DiskGeometry geo, MainWindow::hardDiskTemplates[vendorName])
			log(QString("vendorName = %1, diskName = %2, sectorSize = %3, cylinders = %4, heads = %5, sectors = %6, volumeSize = %7").
			    arg(vendorName).
			    arg(geo.name).
			    arg(geo.sectorSize).
			    arg(geo.cyls).
			    arg(geo.heads).
			    arg(geo.sectors).
			    arg(mainWindow->humanReadable((qreal)geo.cyls * (qreal)geo.heads * (qreal)geo.sectors * (qreal)geo.sectorSize)));
}

int ScriptEngine::runShellCommand(QString command, bool detached)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::runShellCommand(QString command = %1, bool detached = %2)").arg(command).arg(detached)));

	if ( detached ) {
		if ( QProcess::startDetached(command) )
			return 0;
		else
			return 1;
	} else
		return QProcess::execute(command);
}

bool ScriptEngine::createPath(QString path)
{
	QDir dir(path);
	return dir.mkpath(dir.absolutePath());
}

bool ScriptEngine::removePath(QString path)
{
	QDir dir(path);
	return dir.rmpath(dir.absolutePath());
}

QString ScriptEngine::inputGetFilePath(QString initialPath, QString filter, QString windowTitle)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetFilePath(QString initialPath = %1, QString filter = %2, QString windowTitle = %3)").arg(initialPath).arg(filter).arg(windowTitle)));

	QString chosenFile = QFileDialog::getOpenFileName(mScriptWidget, windowTitle.isEmpty() ? tr("Choose file") : windowTitle, initialPath, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	mInputOk = !chosenFile.isNull();
	return chosenFile;
}

QString ScriptEngine::inputGetFolderPath(QString initialPath, QString windowTitle)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetFolderPath(QString initialPath = %1, QString windowTitle = %2)").arg(initialPath).arg(windowTitle)));

	QString chosenFolder = QFileDialog::getExistingDirectory(mScriptWidget, windowTitle.isEmpty() ? tr("Choose folder") : windowTitle, initialPath, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	mInputOk = !chosenFolder.isNull();
	return chosenFolder;
}

QString ScriptEngine::inputGetStringValue(QString initialValue, QString windowTitle, QString labelText)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetStringValue(QString initialValue = %1, QString windowTitle = %2, QString labelText = %3)").arg(initialValue).arg(windowTitle).arg(labelText)));

	return QInputDialog::getText(mScriptWidget, windowTitle.isEmpty() ? tr("Input text") : windowTitle, labelText.isEmpty() ? tr("Input text") : labelText, QLineEdit::Normal, initialValue, &mInputOk);
}

QString ScriptEngine::inputGetListItem(QString initialValue, QStringList itemList, bool editable, QString windowTitle, QString labelText)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetListItem(QString initialValue = %1, QStringList itemList = ..., bool editable = %2, QString windowTitle = %3, QString labelText = %4)").arg(initialValue).arg(editable).arg(windowTitle).arg(labelText)));

	int currentIndex = itemList.indexOf(initialValue);
	return QInputDialog::getItem(mScriptWidget, windowTitle.isEmpty() ? tr("Choose item") : windowTitle, labelText.isEmpty() ? tr("Choose item") : labelText, itemList, currentIndex < 0 ? 0 : currentIndex, editable, &mInputOk);
}

int ScriptEngine::inputGetIntValue(int initialValue, QString windowTitle, QString labelText)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetIntValue(int initialValue = %1, QString windowTitle = %2, QString labelText = %3)").arg(initialValue).arg(windowTitle).arg(labelText)));

	return QInputDialog::getInt(mScriptWidget, windowTitle.isEmpty() ? tr("Input value") : windowTitle, labelText.isEmpty() ? tr("Input value") : labelText, initialValue, -2147483647, 2147483647, 1, &mInputOk);
}

double ScriptEngine::inputGetDoubleValue(double initialValue, int decimals, QString windowTitle, QString labelText)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::inputGetDoubleValue(double initialValue = %1, int decimals = %2, QString windowTitle = %3, QString labelText = %4)").arg(initialValue).arg(decimals).arg(windowTitle).arg(labelText)));

	return QInputDialog::getDouble(mScriptWidget, windowTitle.isEmpty() ? tr("Input value") : windowTitle, labelText.isEmpty() ? tr("Input value") : labelText, initialValue, -2147483647, 2147483647, decimals, &mInputOk);
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

int ScriptEngine::progressGetValue()
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::progressGetValue()")));

	return mScriptWidget->ui->progressBar->value();
}

void ScriptEngine::projectCreate(QString id, QString type)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectCreate(QString id = %1, QString type = %2)").arg(id).arg(type)));

	if ( mProjectMap.contains(id) )
		log(tr("warning") + ": ScriptEngine::projectCreate(): " + tr("project '%1' already exists").arg(id));
	else {
		int typeIndex = MainWindow::projectTypeIndex(type);
		if ( typeIndex >= 0 ) {
			mProjectMap[id] = new ProjectWidget(0, true, typeIndex, id, this);
			connect(mProjectMap[id], SIGNAL(processStarted(ProjectWidget*)), this, SLOT(processStarted(ProjectWidget*)));
			connect(mProjectMap[id], SIGNAL(processFinished(ProjectWidget*)), this, SLOT(processFinished(ProjectWidget*)));
			connect(mProjectMap[id], SIGNAL(progressValueChanged(ProjectWidget*,int)), this, SLOT(monitorUpdateProgress(ProjectWidget*,int)));
		} else
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
		connect(mProjectMap[id], SIGNAL(processStarted(ProjectWidget*)), this, SLOT(processStarted(ProjectWidget*)));
		connect(mProjectMap[id], SIGNAL(processFinished(ProjectWidget*)), this, SLOT(processFinished(ProjectWidget*)));
		connect(mProjectMap[id], SIGNAL(progressValueChanged(ProjectWidget*,int)), this, SLOT(monitorUpdateProgress(ProjectWidget*,int)));
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
		connect(mProjectMap[id], SIGNAL(processStarted(ProjectWidget*)), this, SLOT(processStarted(ProjectWidget*)));
		connect(mProjectMap[id], SIGNAL(processFinished(ProjectWidget*)), this, SLOT(processFinished(ProjectWidget*)));
		connect(mProjectMap[id], SIGNAL(progressValueChanged(ProjectWidget*,int)), this, SLOT(monitorUpdateProgress(ProjectWidget*,int)));
	}
}

void ScriptEngine::projectClone(QString sourceId, QString destinationId)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectClone(QString sourceId = %1, QString destinationId = %2)").arg(sourceId).arg(destinationId)));

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
		return mProjectMap[id]->ui->lineEditVerifyInputFile->text();
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
		return mProjectMap[id]->ui->lineEditVerifyParentInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCopyInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCopyParentInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCopyOutputFile->text();
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
		return mProjectMap[id]->ui->lineEditCopyParentOutputFile->text();
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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartByte(QString id = %1, int byte = %2)").arg(id).arg(byte)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputStartHunk(QString id = %1, int hunk = %2)").arg(id).arg(hunk)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputBytes(QString id = %1, int bytes = %2)").arg(id).arg(bytes)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyInputHunks(QString id = %1, int hunks = %2)").arg(id).arg(hunks)));

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

void ScriptEngine::projectSetCopyHunkSize(QString id, int size)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCopyHunkSize(QString id = %1, int size = %2)").arg(id).arg(size)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxCopyHunkSize->setValue(size);
	else
		log(tr("warning") + ": ScriptEngine::projectSetCopyHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetCopyHunkSize(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetCopyHunkSize(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxCopyHunkSize->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetCopyHunkSize(): " + tr("project '%1' doesn't exists").arg(id));
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
		return mProjectMap[id]->copyCompressors.join(",");
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
		return mProjectMap[id]->ui->lineEditCreateRawInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateRawOutputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateRawParentOutputFile->text();
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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputStartByte(QString id = %1, int byte = %2)").arg(id).arg(byte)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputStartHunk(QString id = %1, int hunk = %2)").arg(id).arg(hunk)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputBytes(QString id = %1, int bytes = %2)").arg(id).arg(bytes)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawInputHunks(QString id = %1, int hunks = %2)").arg(id).arg(hunks)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawHunkSize(QString id = %1, int size = %2)").arg(id).arg(size)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateRawUnitSize(QString id = %1, int size = %2)").arg(id).arg(size)));

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
		return mProjectMap[id]->createRawCompressors.join(",");
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
		return mProjectMap[id]->ui->lineEditCreateHDInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateHDOutputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateHDParentOutputFile->text();
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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputStartByte(QString id = %1, int byte = %2)").arg(id).arg(byte)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputStartHunk(QString id = %1, int hunk = %2)").arg(id).arg(hunk)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputBytes(QString id = %1, int bytes = %2)").arg(id).arg(bytes)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDInputHunks(QString id = %1, int hunks = %2)").arg(id).arg(hunks)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateHDHunkSize(QString id = %1, int size = %2)").arg(id).arg(size)));

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
		return mProjectMap[id]->createHDCompressors.join(",");
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
		return mProjectMap[id]->ui->lineEditCreateCDInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateCDOutputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateCDParentOutputFile->text();
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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateCDHunkSize(QString id = %1, int size = %2)").arg(id).arg(size)));

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
		return mProjectMap[id]->createCDCompressors.join(",");
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
		return mProjectMap[id]->ui->lineEditCreateLDInputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateLDOutputFile->text();
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
		return mProjectMap[id]->ui->lineEditCreateLDParentOutputFile->text();
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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDInputStartFrame(QString id = %1, int frame = %2)").arg(id).arg(frame)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDInputFrames(QString id = %1, int frames = %2)").arg(id).arg(frames)));

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
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetCreateLDHunkSize(QString id = %1, int size = %2)").arg(id).arg(size)));

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
		return mProjectMap[id]->createLDCompressors.join(",");
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

// ExtractRaw

void ScriptEngine::projectSetExtractRawInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractRawInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractRawInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractRawInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractRawParentInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractRawParentInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractRawParentInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawParentInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractRawParentInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractRawOutputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractRawOutputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractRawOutputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawOutputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractRawOutputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractRawForce(QString id, bool force)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxExtractRawForce->setChecked(force);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetExtractRawForce(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawForce(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxExtractRawForce->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawForce(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

void ScriptEngine::projectSetExtractRawInputStartByte(QString id, int byte)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawInputStartByte(QString id = %1, int byte = %2)").arg(id).arg(byte)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractRawInputStartByte->setValue(byte);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractRawInputStartByte(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawInputStartByte(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractRawInputStartByte->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractRawInputStartHunk(QString id, int hunk)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawInputStartHunk(QString id = %1, int hunk = %2)").arg(id).arg(hunk)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractRawInputStartHunk->setValue(hunk);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractRawInputStartHunk(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawInputStartHunk(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractRawInputStartHunk->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractRawInputBytes(QString id, int bytes)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawInputBytes(QString id = %1, int bytes = %2)").arg(id).arg(bytes)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractRawInputBytes->setValue(bytes);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractRawInputBytes(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawInputBytes(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractRawInputBytes->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractRawInputHunks(QString id, int hunks)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractRawInputHunks(QString id = %1, int bytes = %2)").arg(id).arg(hunks)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractRawInputHunks->setValue(hunks);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractRawInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractRawInputHunks(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractRawInputHunks(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractRawInputHunks->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractRawInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

// ExtractHD

void ScriptEngine::projectSetExtractHDInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractHDInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractHDInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractHDInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractHDParentInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractHDParentInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractHDParentInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDParentInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractHDParentInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractHDOutputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractHDOutputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractHDOutputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDOutputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractHDOutputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractHDForce(QString id, bool force)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxExtractHDForce->setChecked(force);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetExtractHDForce(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDForce(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxExtractHDForce->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDForce(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

void ScriptEngine::projectSetExtractHDInputStartByte(QString id, int byte)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDInputStartByte(QString id = %1, int byte = %2)").arg(id).arg(byte)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractHDInputStartByte->setValue(byte);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractHDInputStartByte(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDInputStartByte(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractHDInputStartByte->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDInputStartByte(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractHDInputStartHunk(QString id, int hunk)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDInputStartHunk(QString id = %1, int hunk = %2)").arg(id).arg(hunk)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractHDInputStartHunk->setValue(hunk);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractHDInputStartHunk(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDInputStartHunk(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractHDInputStartHunk->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDInputStartHunk(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractHDInputBytes(QString id, int bytes)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDInputBytes(QString id = %1, int bytes = %2)").arg(id).arg(bytes)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractHDInputBytes->setValue(bytes);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractHDInputBytes(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDInputBytes(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractHDInputBytes->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDInputBytes(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractHDInputHunks(QString id, int hunks)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractHDInputHunks(QString id = %1, int bytes = %2)").arg(id).arg(hunks)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractHDInputHunks->setValue(hunks);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractHDInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractHDInputHunks(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractHDInputHunks(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractHDInputHunks->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractHDInputHunks(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

// ExtractCD

void ScriptEngine::projectSetExtractCDInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractCDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractCDInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractCDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractCDInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractCDInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractCDInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractCDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractCDParentInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractCDParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractCDParentInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractCDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractCDParentInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractCDParentInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractCDParentInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractCDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractCDOutputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractCDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractCDOutputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractCDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractCDOutputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractCDOutputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractCDOutputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractCDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractCDOutputBinFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractCDOutputBinFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractCDOutputBinFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractCDOutputBinFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractCDOutputBinFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractCDOutputBinFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractCDOutputBinFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractCDOutputBinFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractCDForce(QString id, bool force)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractCDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxExtractCDForce->setChecked(force);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractCDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetExtractCDForce(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractCDForce(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxExtractCDForce->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractCDForce(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

// ExtractLD

void ScriptEngine::projectSetExtractLDInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractLDInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractLDInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractLDInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractLDParentInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDParentInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractLDParentInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractLDParentInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDParentInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractLDParentInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDParentInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractLDOutputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditExtractLDOutputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetExtractLDOutputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDOutputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditExtractLDOutputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetExtractLDForce(QString id, bool force)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxExtractLDForce->setChecked(force);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetExtractLDForce(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDForce(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxExtractLDForce->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDForce(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

void ScriptEngine::projectSetExtractLDInputStartFrame(QString id, int frame)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDInputStartFrame(QString id = %1, int frame = %2)").arg(id).arg(frame)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractLDInputStartFrame->setValue(frame);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDInputStartFrame(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractLDInputStartFrame(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDInputStartFrame(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractLDInputStartFrame->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDInputStartFrame(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetExtractLDInputFrames(QString id, int frames)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetExtractLDInputFrames(QString id = %1, int frame = %2)").arg(id).arg(frames)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxExtractLDInputFrames->setValue(frames);
	else
		log(tr("warning") + ": ScriptEngine::projectSetExtractLDInputFrames(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetExtractLDInputFrames(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetExtractLDInputFrames(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxExtractLDInputFrames->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetExtractLDInputFrames(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

// DumpMeta

void ScriptEngine::projectSetDumpMetaInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDumpMetaInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditDumpMetaInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDumpMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetDumpMetaInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDumpMetaInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditDumpMetaInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDumpMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetDumpMetaOutputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDumpMetaOutputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditDumpMetaOutputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDumpMetaOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetDumpMetaOutputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDumpMetaOutputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditDumpMetaOutputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDumpMetaOutputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetDumpMetaForce(QString id, bool force)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDumpMetaForce(QString id = %1, bool force = %2)").arg(id).arg(force)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxDumpMetaForce->setChecked(force);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDumpMetaForce(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetDumpMetaForce(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDumpMetaForce(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxDumpMetaForce->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDumpMetaForce(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

void ScriptEngine::projectSetDumpMetaTag(QString id, QString tag)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDumpMetaTag(QString id = %1, QString tag = %2)").arg(id).arg(tag)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditDumpMetaTag->setText(tag);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDumpMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetDumpMetaTag(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDumpMetaTag(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditDumpMetaTag->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDumpMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetDumpMetaIndex(QString id, int index)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDumpMetaIndex(QString id = %1, int index = %2)").arg(id).arg(index)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxDumpMetaIndex->setValue(index);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDumpMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetDumpMetaIndex(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDumpMetaIndex(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxDumpMetaIndex->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDumpMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

// AddMeta

void ScriptEngine::projectSetAddMetaInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditAddMetaInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetAddMetaInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditAddMetaInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetAddMetaValueFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaValueFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditAddMetaValueFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaValueFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetAddMetaValueFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaValueFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditAddMetaValueFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaValueFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetAddMetaValueText(QString id, QString text)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaValueText(QString id = %1, QString text = %2)").arg(id).arg(text)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditAddMetaValueText->setText(text);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaValueText(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetAddMetaValueText(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaValueText(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditAddMetaValueText->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaValueText(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetAddMetaTag(QString id, QString tag)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaTag(QString id = %1, QString tag = %2)").arg(id).arg(tag)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditAddMetaTag->setText(tag);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetAddMetaTag(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaTag(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditAddMetaTag->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetAddMetaIndex(QString id, int index)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaIndex(QString id = %1, int index = %2)").arg(id).arg(index)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxAddMetaIndex->setValue(index);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetAddMetaIndex(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaIndex(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxAddMetaIndex->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

void ScriptEngine::projectSetAddMetaNoCheckSum(QString id, bool noCheckSum)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetAddMetaNoCheckSum(QString id = %1, bool noCheckSum = %2)").arg(id).arg(noCheckSum)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->checkBoxAddMetaNoCheckSum->setChecked(noCheckSum);
	else
		log(tr("warning") + ": ScriptEngine::projectSetAddMetaNoCheckSum(): " + tr("project '%1' doesn't exists").arg(id));
}

bool ScriptEngine::projectGetAddMetaNoCheckSum(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetAddMetaNoCheckSum(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->checkBoxAddMetaNoCheckSum->isChecked();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetAddMetaNoCheckSum(): " + tr("project '%1' doesn't exists").arg(id));
		return false;
	}
}

// DelMeta

void ScriptEngine::projectSetDelMetaInputFile(QString id, QString file)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDelMetaInputFile(QString id = %1, QString file = %2)").arg(id).arg(file)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditDelMetaInputFile->setText(file);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDelMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetDelMetaInputFile(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDelMetaInputFile(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditDelMetaInputFile->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDelMetaInputFile(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetDelMetaTag(QString id, QString tag)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDelMetaTag(QString id = %1, QString tag = %2)").arg(id).arg(tag)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->lineEditDelMetaTag->setText(tag);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDelMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
}

QString ScriptEngine::projectGetDelMetaTag(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDelMetaTag(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->lineEditDelMetaTag->text();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDelMetaTag(): " + tr("project '%1' doesn't exists").arg(id));
		return QString();
	}
}

void ScriptEngine::projectSetDelMetaIndex(QString id, int index)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectSetDelMetaIndex(QString id = %1, int index = %2)").arg(id).arg(index)));

	if ( mProjectMap.contains(id) )
		mProjectMap[id]->ui->spinBoxDelMetaIndex->setValue(index);
	else
		log(tr("warning") + ": ScriptEngine::projectSetDelMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
}

int ScriptEngine::projectGetDelMetaIndex(QString id)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::projectGetDelMetaIndex(QString id = %1)").arg(id)));

	if ( mProjectMap.contains(id) )
		return mProjectMap[id]->ui->spinBoxDelMetaIndex->value();
	else {
		log(tr("warning") + ": ScriptEngine::projectGetDelMetaIndex(): " + tr("project '%1' doesn't exists").arg(id));
		return -1;
	}
}

// project control & synchronization

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

void ScriptEngine::waitForRunningProjects(int numProjects)
{
	QCHDMAN_SCRIPT_ENGINE_DEBUG(log(QString("DEBUG: ScriptEngine::waitForRunningProjects(int numProjects = %1)").arg(numProjects)));

	int previouslyRunningProjects = runningProjects();
	while ( (previouslyRunningProjects - runningProjects() < numProjects) && !externalStop ) {
		foreach (QString id, mProjectMap.keys()) {
			if ( externalStop || (previouslyRunningProjects - runningProjects() >= numProjects) )
				break;

			if ( mProjectMap[id]->status == QCHDMAN_PRJSTAT_RUNNING ) {
				mProjectMap[id]->chdmanProc->waitForFinished(QCHDMAN_PROCESS_POLL_TIME);
				qApp->processEvents();
			}
		}
	}
}

// Slots for internal use

void ScriptEngine::processStarted(ProjectWidget *projectWidget)
{
	QString id = mProjectMap.key(projectWidget, "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND");
	if ( id != "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND" ) {
		mRunningProjects++;
		emit projectStarted(id);
		QTreeWidgetItem *projectItem = new QTreeWidgetItem(mScriptWidget->ui->treeWidgetProjectMonitor);
		projectItem->setText(QCHDMAN_PRJMON_ID, id);
		projectItem->setForeground(QCHDMAN_PRJMON_PROGRESS, mScriptWidget->ui->treeWidgetProjectMonitor->viewport()->palette().base());
		QProgressBar *progressBar = new QProgressBar(mScriptWidget->ui->treeWidgetProjectMonitor);
		mScriptWidget->ui->treeWidgetProjectMonitor->setItemWidget(projectItem, QCHDMAN_PRJMON_PROGRESS, progressBar);
		progressBar->setRange(0, 100);
		progressBar->setValue(0);
		progressBar->setAutoFillBackground(true);
		QFontMetrics fm(qApp->font());
		progressBar->setFixedHeight(fm.height() + 2);
		projectItem->setText(QCHDMAN_PRJMON_PROGRESS, "000");
		QString command = globalConfig->preferencesChdmanBinary();
		foreach (QString arg, projectWidget->arguments) {
			if ( arg.contains(QRegExp("\\s")) )
				command += " \"" + arg + "\"";
			else
				command += " " + arg;
		}
		projectItem->setText(QCHDMAN_PRJMON_COMMAND, command);
	}
}

void ScriptEngine::processFinished(ProjectWidget *projectWidget)
{
	QString id = mProjectMap.key(projectWidget, "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND");
	if ( id != "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND" ) {
		mRunningProjects--;
		emit projectFinished(id);
		QList<QTreeWidgetItem *> itemList = mScriptWidget->ui->treeWidgetProjectMonitor->findItems(id, Qt::MatchExactly, QCHDMAN_PRJMON_ID);
		if ( itemList.count() > 0 ) {
			QTreeWidgetItem *item = itemList[0];
			if ( item->isSelected() ) {
				mScriptWidget->ui->treeWidgetProjectMonitor->clearSelection();
				mScriptWidget->ui->treeWidgetProjectMonitor->setCurrentIndex(QModelIndex());
			}
			mScriptWidget->ui->treeWidgetProjectMonitor->setUpdatesEnabled(false);
			QProgressBar *progressBar = (QProgressBar *)mScriptWidget->ui->treeWidgetProjectMonitor->itemWidget(item, QCHDMAN_PRJMON_PROGRESS);
			if ( progressBar ) {
				mScriptWidget->ui->treeWidgetProjectMonitor->removeItemWidget(item, QCHDMAN_PRJMON_PROGRESS);
				delete progressBar;
			}
			delete mScriptWidget->ui->treeWidgetProjectMonitor->takeTopLevelItem(mScriptWidget->ui->treeWidgetProjectMonitor->indexOfTopLevelItem(item));
			mScriptWidget->ui->treeWidgetProjectMonitor->setUpdatesEnabled(true);
		}
	}
}

void ScriptEngine::monitorUpdateProgress(ProjectWidget *projectWidget, int progressValue)
{
	QString id = mProjectMap.key(projectWidget, "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND");
	if ( id != "QCHDMAN_SCRIPT_ENGINE_NO_PROJECT_ID_FOUND" ) {
		QList<QTreeWidgetItem *> itemList = mScriptWidget->ui->treeWidgetProjectMonitor->findItems(id, Qt::MatchExactly, QCHDMAN_PRJMON_ID);
		if ( itemList.count() > 0 ) {
			QTreeWidgetItem *item = itemList[0];
			QProgressBar *progressBar = (QProgressBar *)mScriptWidget->ui->treeWidgetProjectMonitor->itemWidget(item, QCHDMAN_PRJMON_PROGRESS);
			if ( progressBar )
				progressBar->setValue(progressValue);
			item->setText(QCHDMAN_PRJMON_PROGRESS, QString::number(progressValue).rightJustified(3, QLatin1Char('0')));
		}
	}
}
