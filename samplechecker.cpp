#include <QTimer>
#include <QMap>
#include <QHash>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include "settings.h"
#include "options.h"
#include "samplechecker.h"
#include "machinelist.h"
#include "qmc2main.h"
#include "processmanager.h"
#include "toolexec.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern ProcessManager *qmc2ProcessManager;
extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern bool qmc2CleaningUp;
extern bool qmc2SampleCheckActive;
extern bool qmc2StopParser;
extern bool qmc2TemplateCheck;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;

SampleChecker::SampleChecker(QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	setupUi(this);
	progressBar->setFormat(tr("Idle"));
	progressBar->setRange(-1, -1);
	progressBar->setValue(-1);
	adjustIconSizes();
}

void SampleChecker::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	toolButtonSamplesRemoveObsolete->setIconSize(iconSize);
	pushButtonSamplesCheck->setIconSize(iconSize);
}

void SampleChecker::restoreLayout()
{
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position") )
		move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position", pos()).toPoint());
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size") )
		resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size", size()).toSize());
}

void SampleChecker::closeEvent(QCloseEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position", pos());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size", size());

	if ( e )
		e->accept();
}

void SampleChecker::hideEvent(QHideEvent *e)
{
	closeEvent(0);
	e->accept();
}

void SampleChecker::showEvent(QShowEvent *e)
{
	restoreLayout();
	if ( e )
		e->accept();
}

void SampleChecker::verify()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying samples"));
	qmc2SampleCheckActive = true;
	qmc2StopParser = false;
	sampleSets.clear();
	verifyTimer.start();
	listWidgetSamplesGood->clear();
	labelSamplesGood->setText(tr("Good: 0"));
	listWidgetSamplesBad->clear();
	labelSamplesBad->setText(tr("Bad: 0"));
	listWidgetSamplesMissing->clear();
	labelSamplesMissing->setText(tr("Missing: 0"));
	listWidgetSamplesObsolete->clear();
	labelSamplesObsolete->setText(tr("Obsolete: 0"));
	toolButtonSamplesRemoveObsolete->setEnabled(false);
	sampleMap.clear();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("preparing sample-check: parsing XML data for relevant sample information"));
	QString currentGameName, currentSampleOf;
	bool hasSamples = false;
	int sampleCount = 0;
	QMap<QString, int> sampleCountMap;
	progressBar->setFormat(tr("Parsing XML data"));
	qint64 xmlRowCount = qmc2MachineList->xmlDb()->xmlRowCount();
	progressBar->setRange(0, xmlRowCount);
	progressBar->setValue(0);
	for (qint64 rowCounter = 1; rowCounter < xmlRowCount; rowCounter++) {
		QStringList xmlLines = qmc2MachineList->xmlDb()->xml(rowCounter).split("\n", QString::SkipEmptyParts);
		int xmlLinesCount = xmlLines.count();
		progressBar->setValue(rowCounter);
		if ( rowCounter % QMC2_CHECK_UPDATE_FAST == 0 )
			qApp->processEvents();
		for (int gameListPos = 0; gameListPos < xmlLinesCount && !qmc2StopParser; gameListPos++) {
			QString line = xmlLines[gameListPos];
			int startIndex = line.indexOf("<machine name=\"");
			int endIndex = -1;
			if ( startIndex >= 0 ) {
				startIndex += 15;
				endIndex = line.indexOf("\"", startIndex);
				if ( endIndex >= 0 )
					currentGameName = line.mid(startIndex, endIndex - startIndex);
				hasSamples = false;
				sampleCount = 0;
				startIndex = line.indexOf("sampleof=\"");
				if ( startIndex >= 0 ) {
					startIndex += 10;
					endIndex = line.indexOf("\"", startIndex);
					if ( endIndex >= 0 ) {
						currentSampleOf = line.mid(startIndex, endIndex - startIndex);
						if ( currentSampleOf == currentGameName )
							currentSampleOf.clear();
					}
					hasSamples = true;
				}
			} else if ( line.indexOf("<sample name=\"") >= 0 ) {
				hasSamples |= true;
				sampleCount++;
			} else {
				startIndex = line.indexOf("</machine>");
				if ( startIndex >= 0 ) {
					if ( !currentGameName.isEmpty() && hasSamples ) {
						if ( currentSampleOf.isEmpty() ) {
							sampleMap[currentGameName] = currentGameName;
							sampleCountMap[currentGameName] = sampleCount;
						} else {
							if ( qmc2MachineListItemHash.contains(currentSampleOf) ) {
								sampleMap[currentGameName] = currentSampleOf;
								sampleCountMap[currentGameName] = sampleCount;
							} else
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the machine '%1' is referencing a non-existing sample-set (sampleof=\"%2\") -- please inform MAME developers").arg(currentGameName).arg(currentSampleOf));
						}
					}
					currentGameName.clear();
					currentSampleOf.clear();
					hasSamples = false;
					sampleCount = 0;
				}
			}
		}
	}

	if ( qmc2StopParser ) {
		progressBar->setFormat(tr("Idle"));
		progressBar->setRange(-1, -1);
		progressBar->setValue(-1);
		pushButtonSamplesCheck->setText(tr("&Check samples"));
		pushButtonSamplesCheck->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
		QTime elapsedTime(0, 0, 0, 0);
		elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying samples, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
		qmc2SampleCheckActive = false;
		return;
	}

	sampleSets = sampleMap.values();
	sampleSets.removeDuplicates();

	for (int i = 0; i < sampleSets.count(); i++) {
		QString currentSample = sampleSets[i];
		if ( sampleCountMap[currentSample] == 0 ) {
			QStringList refList = sampleMap.keys(currentSample);
			if ( refList.count() > 0 ) {
				if ( refList.count() > 1 )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the following machines are referencing a sample-set which isn't required (sampleof=\"%1\"): %2 -- please inform MAME developers").arg(currentSample).arg(refList.join(", ")));
				else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the following machine is referencing a sample-set which isn't required (sampleof=\"%1\"): %2 -- please inform MAME developers").arg(currentSample).arg(refList.join(", ")));
				sampleSets.removeAll(currentSample);
			}
		}
	}

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("found %n individual sample set(s)", "", sampleSets.count()) + " " + tr("and") + " " + tr("%n system(s) using samples", "", sampleMap.keys().count()));

	progressBar->setRange(0, sampleSets.count());
	progressBar->setValue(0);
	progressBar->setFormat(tr("Checking sample status"));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 1: checking sample status"));
  
	QString userScopePath = Options::configPath();
	QString emuWorkDir = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString();
	for (int i = 0; i < sampleSets.count() && !qmc2StopParser; i++) {
		progressBar->setValue(i + 1);
		QString sampleSet = sampleSets[i];
		QProcess commandProc;
#if defined(QMC2_SDLMAME)
		commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
		commandProc.setStandardErrorFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
		commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
		commandProc.setStandardErrorFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif

		if ( !emuWorkDir.isEmpty() )
			commandProc.setWorkingDirectory(emuWorkDir);

		QStringList args;
		if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath") )
			args << "-samplepath" << qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath").toString().replace("~", "$HOME");
		args << "-verifysamples" << sampleSet;

		bool commandProcStarted = false;
		int retries = 0;
		commandProc.start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
		bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
			started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
			qApp->processEvents();
		}

		if ( started ) {
			commandProcStarted = true;
			bool commandProcRunning = (commandProc.state() == QProcess::Running);
			while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
				qApp->processEvents();
				commandProcRunning = (commandProc.state() == QProcess::Running);
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up") + " (" + tr("error text = %1").arg(ProcessManager::errorText(commandProc.error())) + ")");
			break;
		}

#if defined(QMC2_SDLMAME)
		QFile sampleTemp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
		QFile sampleTemp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#endif

		if ( commandProcStarted && sampleTemp.open(QFile::ReadOnly) ) {
			QTextStream ts(&sampleTemp);
			QString buffer = ts.readAll();
#if defined(QMC2_OS_WIN)
			buffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
			if ( !buffer.isEmpty() ) {
				QStringList bufferLines = buffer.split("\n", QString::SkipEmptyParts);
				if ( !bufferLines.isEmpty() ) {
					QString bufferLine = bufferLines[0].simplified().replace("\"", "");
					if ( bufferLine.endsWith("is good") ) {
						listWidgetSamplesGood->addItem(sampleSet);
						labelSamplesGood->setText(tr("Good: %1").arg(listWidgetSamplesGood->count()));
					} else if ( bufferLine.endsWith("is bad") || bufferLine.endsWith(", 0 were OK.") ) {
						listWidgetSamplesBad->addItem(sampleSet);
						labelSamplesBad->setText(tr("Bad: %1").arg(listWidgetSamplesBad->count()));
					} else if ( bufferLine.endsWith("not found!") ) {
						listWidgetSamplesMissing->addItem(sampleSet);
						labelSamplesMissing->setText(tr("Missing: %1").arg(listWidgetSamplesMissing->count()));
					} else
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: received unknown output when checking the sample status of '%1': '%2'").arg(sampleSet).arg(bufferLine));
					qApp->processEvents();
				} else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: received no output when checking the sample status of '%1'").arg(sampleSet));
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: received no output when checking the sample status of '%1'").arg(sampleSet));
		}
		sampleTemp.remove();
	}

	if ( !qmc2StopParser )
		verifyObsolete();

	listWidgetSamplesGood->sortItems(Qt::AscendingOrder);
	listWidgetSamplesBad->sortItems(Qt::AscendingOrder);
	listWidgetSamplesMissing->sortItems(Qt::AscendingOrder);
	toolButtonSamplesRemoveObsolete->setEnabled(listWidgetSamplesObsolete->count() > 0);

	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying samples, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 good, %2 bad, %3 missing, %4 obsolete").arg(listWidgetSamplesGood->count()).arg(listWidgetSamplesBad->count()).arg(listWidgetSamplesMissing->count()).arg(listWidgetSamplesObsolete->count()));
	pushButtonSamplesCheck->setText(tr("&Check samples"));
	pushButtonSamplesCheck->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));

	qmc2SampleCheckActive = false;
	progressBar->setFormat(tr("Idle"));
	progressBar->setRange(-1, -1);
	progressBar->setValue(-1);
}

void SampleChecker::verifyObsolete()
{
	progressBar->setFormat(tr("Checking for obsolete files / folders"));
	progressBar->setRange(0, 0);
	progressBar->setValue(-1);
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: checking for obsolete files / folders"));

	QStringList samplePaths;
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath") )
		samplePaths = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/samplepath").toString().split(";", QString::SkipEmptyParts);
	else
		samplePaths << "samples";

	QString emuWorkDir = QDir::toNativeSeparators(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
	if ( !emuWorkDir.isEmpty() )
		if ( !emuWorkDir.endsWith(QDir::separator()) )
			emuWorkDir += QDir::separator();
	foreach (QString samplePath, samplePaths) {
		if ( QDir::isRelativePath(samplePath) ) {
			if ( !emuWorkDir.isEmpty() )
				samplePath.prepend(emuWorkDir);

		}
		if ( !samplePath.endsWith(QDir::separator()) )
			samplePath += QDir::separator();
		QDir sampleDir(samplePath);
		if ( sampleDir.exists() ) {
			QStringList fileList;
			recursiveFileList(samplePath, fileList);
			foreach (QString file, fileList) {
				QString relativeFilePath = file.remove(samplePath);
				QString gameName = relativeFilePath.toLower().remove(QRegExp("\\.zip$"));
				if ( !sampleSets.contains(gameName) ) {
					listWidgetSamplesObsolete->addItem(QDir::toNativeSeparators(sampleDir.absolutePath() + QDir::separator() + relativeFilePath));
					labelSamplesObsolete->setText(tr("Obsolete: %1").arg(listWidgetSamplesObsolete->count()));
				}
			}
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: sample path '%1' does not exist").arg(sampleDir.absolutePath()));
	}
}

void SampleChecker::on_pushButtonSamplesCheck_clicked()
{
	if ( qmc2SampleCheckActive ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("stopping sample check upon user request"));
		qmc2StopParser = true;
		return;
	}

	if ( qmc2MainWindow->isActiveState ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (the sample-checker can only run exclusively)"));
		return;
	}

	pushButtonSamplesCheck->setText(tr("&Stop check"));
	pushButtonSamplesCheck->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	verify();
}

void SampleChecker::on_toolButtonSamplesRemoveObsolete_clicked()
{
	toolButtonSamplesRemoveObsolete->setEnabled(false);
	pushButtonSamplesCheck->setEnabled(false);
	QString emuWorkDir = QDir::toNativeSeparators(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
	if ( !emuWorkDir.isEmpty() )
		if ( !emuWorkDir.endsWith(QDir::separator()) )
			emuWorkDir += QDir::separator();
	int count = 0;
	progressBar->setFormat(tr("Removing obsolete files / folders"));
	progressBar->setRange(0, listWidgetSamplesObsolete->count());
	listWidgetSamplesObsolete->setUpdatesEnabled(false);
	for (int row = 0; row < listWidgetSamplesObsolete->count(); row++) {
		progressBar->setValue(count);
		QListWidgetItem *item = listWidgetSamplesObsolete->item(row);
		QString path = item->text();
		QFileInfo fi(path);
		if ( fi.isDir() ) {
			QDir d(path);
			if ( QDir::isRelativePath(path) )
				if ( !emuWorkDir.isEmpty() )
					path.prepend(emuWorkDir);
			if ( d.rmdir(path) ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("obsolete folder '%1' removed").arg(d.absolutePath()));
				QListWidgetItem *itemToDelete = listWidgetSamplesObsolete->takeItem(row);
				if ( itemToDelete ) {
					delete itemToDelete;
					labelSamplesObsolete->setText(tr("Obsolete: %1").arg(listWidgetSamplesObsolete->count()));
					row--;
				}
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("obsolete folder '%1' cannot be removed, please check permissions").arg(d.absolutePath()));
		} else {
			if ( fi.isRelative() )
				if ( !emuWorkDir.isEmpty() )
					path.prepend(emuWorkDir);
			QFile f(path);
			if ( f.remove() ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("obsolete file '%1' removed").arg(fi.absoluteFilePath()));
				QListWidgetItem *itemToDelete = listWidgetSamplesObsolete->takeItem(row);
				if ( itemToDelete ) {
					delete itemToDelete;
					labelSamplesObsolete->setText(tr("Obsolete: %1").arg(listWidgetSamplesObsolete->count()));
					row--;
				}
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("obsolete file '%1' cannot be removed, please check permissions").arg(fi.absoluteFilePath()));
		}
		if ( count++ % QMC2_CHECK_UPDATE_MEDIUM == 0 ) {
			listWidgetSamplesObsolete->setUpdatesEnabled(true);
			listWidgetSamplesObsolete->update();
			qApp->processEvents();
			listWidgetSamplesObsolete->setUpdatesEnabled(false);
		}
	}
	listWidgetSamplesObsolete->setUpdatesEnabled(true);
	toolButtonSamplesRemoveObsolete->setEnabled(listWidgetSamplesObsolete->count() > 0);
	pushButtonSamplesCheck->setEnabled(true);
	progressBar->setFormat(tr("Idle"));
	progressBar->setRange(-1, -1);
	progressBar->setValue(-1);
}

void SampleChecker::recursiveFileList(const QString &sDir, QStringList &fileNames)
{
	QDir dir(sDir);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System)) {
		QString path = QDir::toNativeSeparators(info.filePath());
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." ) {
				recursiveFileList(path, fileNames);
				fileNames << path + QDir::separator();
				qApp->processEvents();
			}
		} else
			fileNames << path;
	}
} 
