#include <QSettings>
#include <QTimer>
#include <QMap>
#include <QDir>
#include <QFileInfo>

#include "sampcheck.h"
#include "qmc2main.h"
#include "procmgr.h"
#include "toolexec.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern ProcessManager *qmc2ProcessManager;
extern QSettings *qmc2Config;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2ReloadActive;
extern bool qmc2ImageCheckActive;
extern bool qmc2SampleCheckActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2VerifyActive;
extern bool qmc2FilterActive;
extern bool qmc2StopParser;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

SampleChecker::SampleChecker(QWidget *parent)
#if defined(Q_WS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::SampleChecker(QWidget *parent = %1").arg((qulonglong)parent));
#endif

  setupUi(this);

  checkBoxSamplesSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SampleChecker/SelectGame", true).toBool());
  verifyProc = NULL;
}

SampleChecker::~SampleChecker()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::~SampleChecker()");
#endif

  if ( verifyProc )
    verifyProc->terminate();
}

void SampleChecker::restoreLayout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::restoreLayout()");
#endif

  if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position") )
    move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position", pos()).toPoint());
  if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size") )
    resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size", size()).toSize());
}

void SampleChecker::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::closeEvent(QCloseEvent *e = %1").arg((qulonglong)e));
#endif

  // save settings
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SampleChecker/SelectGame", checkBoxSamplesSelectGame->isChecked());

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Position", pos());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Size", size());
    if ( !qmc2CleaningUp )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Visible", false);
  }
  if ( e )
    e->accept();
}

void SampleChecker::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::hideEvent(QHideEvent *e = %1").arg((qulonglong)e));
#endif

  closeEvent(NULL);
  e->accept();
}

void SampleChecker::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::showEvent(QShowEvent *e = %1").arg((qulonglong)e));
#endif

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() )
    restoreLayout();

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Visible", true);

  if ( e )
    e->accept();
}

void SampleChecker::verify()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verify()"));
#endif

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2SampleCheckActive = true;
  qmc2StopParser = false;

  sampleSets.clear();
  stdoutLastLine = "";
  stderrLastLine = "";
  verifyTimer.start();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying samples"));
  listWidgetSamplesGood->clear();
  labelSamplesGood->setText(tr("Good: 0"));
  listWidgetSamplesBad->clear();
  labelSamplesBad->setText(tr("Bad: 0"));
  listWidgetSamplesObsolete->clear();
  labelSamplesObsolete->setText(tr("Obsolete: 0"));
  pushButtonSamplesRemoveObsolete->setEnabled(false);
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 1: sample status"));
  
  QStringList args;
  QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains("MAME/Configuration/Global/samplepath") )
    args << "-samplepath" << qmc2Config->value("MAME/Configuration/Global/samplepath").toString().replace("~", "$HOME");
  args << "-verifysamples";
#if defined(QMC2_AUDIT_WILDCARD)
  args << "*";
#endif

  verifyProc = new QProcess(this);
  connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
  connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
  connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
  connect(verifyProc, SIGNAL(readyReadStandardError()), this, SLOT(verifyReadyReadStandardError()));
  connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
  connect(verifyProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(verifyStateChanged(QProcess::ProcessState)));
  verifyProc->start(command, args);
#endif
}

void SampleChecker::verifyStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::verifyStarted()");
#endif

}

void SampleChecker::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  QProcess *proc = (QProcess *)sender();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verifyFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2): proc = %3").arg(exitCode).arg(exitStatus).arg((qulonglong)proc));
#endif

  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));

  if ( verifyProc )
    delete verifyProc;
  verifyProc = NULL;
  qApp->processEvents();
  QTimer::singleShot(0, this, SLOT(verifyObsolete()));
}

void SampleChecker::verifyObsolete()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::verifyObsolete()");
#endif

  if ( !qmc2StopParser ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete sample sets"));
    QStringList fileList;
    QString samplePath;
    if ( qmc2Config->contains("MAME/Configuration/Global/samplepath") )
      samplePath = qmc2Config->value("MAME/Configuration/Global/samplepath").toString();
    else
      samplePath = "samples";
    if ( !samplePath.endsWith("/") ) samplePath += "/";
    recursiveFileList(samplePath, fileList);
    int i;
#ifdef QMC2_DEBUG
    for (i = 0; i < sampleSets.count(); i++)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verifyObsolete(): sampleSets[%1] = %2").arg(i).arg(sampleSets[i]));
#endif
    for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
      QString relativeFilePath = fileList[i].remove(samplePath);
      QString gameName = relativeFilePath.toLower().remove(QRegExp("\\.zip$"));
#ifdef QMC2_DEBUG
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verifyObsolete(): fileList[i] = %1, relativeFilePath = %2, gameName = %3").arg(fileList[i]).arg(relativeFilePath).arg(gameName));
#endif
      if ( !sampleSets.contains(gameName) ) {
        listWidgetSamplesObsolete->addItem(relativeFilePath);
        labelSamplesObsolete->setText(tr("Obsolete: %1").arg(listWidgetSamplesObsolete->count()));
      }
    }
  }
  listWidgetSamplesGood->sortItems(Qt::AscendingOrder);
  listWidgetSamplesBad->sortItems(Qt::AscendingOrder);
  listWidgetSamplesObsolete->sortItems(Qt::AscendingOrder);
  qApp->processEvents();
  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying samples, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 good, %2 bad (or missing), %3 obsolete").arg(listWidgetSamplesGood->count()).arg(listWidgetSamplesBad->count()).arg(listWidgetSamplesObsolete->count()));

  // enable removal button if obsolete samples exist
  pushButtonSamplesRemoveObsolete->setEnabled(listWidgetSamplesObsolete->count() > 0);

  pushButtonSamplesCheck->setText(tr("&Check samples"));
  qmc2SampleCheckActive = false;
}

void SampleChecker::verifyReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verifyReadyReadStandardOutput(): proc = %1").arg((qulonglong)proc));
#endif

  // process sample verification output
  int i;
  QString s = stdoutLastLine + proc->readAllStandardOutput();
#if defined(Q_WS_WIN)
  s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
  QStringList lines = s.split("\n");

  if ( s.endsWith("\n") ) {
    stdoutLastLine = "";
  } else {
    stdoutLastLine = lines.last();
    lines.removeLast();
  }

  QStringList badList;
  for (i = 0; i < lines.count(); i++) {
    if ( lines[i].simplified().isEmpty() )
      continue;
    QStringList words = lines[i].remove("\"").simplified().split(" ");
    if ( lines[i].startsWith("sampleset ") ) {
      if ( lines[i].endsWith("good") ) {
        listWidgetSamplesGood->addItem(words[1]);
        labelSamplesGood->setText(tr("Good: %1").arg(listWidgetSamplesGood->count()));
        sampleSets << words[1];
      } else {
        QString sampleName = words[1]; 
        if ( !badList.contains(sampleName) ) {
          listWidgetSamplesBad->addItem(sampleName);
          labelSamplesBad->setText(tr("Bad: %1").arg(listWidgetSamplesBad->count()));
          badList << sampleName;
          sampleSets << sampleName;
        }
      }
    }
  }

  if ( qmc2StopParser && verifyProc )
    verifyProc->terminate();

  qApp->processEvents();
}

void SampleChecker::verifyReadyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::verifyReadyReadStandardError(): proc = %1").arg((qulonglong)proc));
#endif

  // process sample verification output
  int i;
  QString s = stderrLastLine + proc->readAllStandardError();
#if defined(Q_WS_WIN)
  s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
  QStringList lines = s.split("\n");

  if ( s.endsWith("\n") ) {
    stderrLastLine = "";
  } else {
    stderrLastLine = lines.last();
    lines.removeLast();
  }

  QStringList badList;
  for (i = 0; i < lines.count(); i++) {
    if ( lines[i].simplified().isEmpty() )
      continue;
    QStringList words = lines[i].remove("\"").simplified().split(" ");
    if ( lines[i].startsWith("sampleset ") ) {
      if ( lines[i].endsWith("good") ) {
        listWidgetSamplesGood->addItem(words[1]);
        labelSamplesGood->setText(tr("Good: %1").arg(listWidgetSamplesGood->count()));
        sampleSets << words[1];
      } else {
        QString sampleName = words[1]; 
        if ( !badList.contains(sampleName) ) {
          listWidgetSamplesBad->addItem(sampleName);
          labelSamplesBad->setText(tr("Bad: %1").arg(listWidgetSamplesBad->count()));
          badList << sampleName;
          sampleSets << sampleName;
        }
      }
    }
  }

  if ( qmc2StopParser && verifyProc )
    verifyProc->terminate();

  qApp->processEvents();
}

void SampleChecker::verifyError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::verifyError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void SampleChecker::verifyStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::verifyStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

void SampleChecker::on_pushButtonSamplesCheck_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::on_pushButtonSamplesCheck_clicked()");
#endif

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  if ( qmc2FilterActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
    return;
  }

  if ( qmc2VerifyActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ImageCheckActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
    return;
  }

  if ( qmc2SampleCheckActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("stopping sample check upon user request"));
    qmc2StopParser = true;
    return;
  }

  pushButtonSamplesCheck->setText(tr("&Stop check"));
  verify();
}

void SampleChecker::on_pushButtonSamplesRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::on_pushButtonSamplesRemoveObsolete_clicked()");
#endif

#if defined(Q_WS_WIN)
  QString command = "cmd.exe";
  QStringList args;
  args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString().replace('/', '\\')
       << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#else
  QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString();
  QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#endif

  int i, j;
  QStringList addArgs;
  for (i = 0; i < args.count(); i++) {
    if ( args[i] == "$FILELIST$" ) {
      QList<QListWidgetItem *> items = listWidgetSamplesObsolete->findItems("*", Qt::MatchWildcard); 
#if defined(Q_WS_WIN)
      QString samplePath;
      if ( qmc2Config->contains("MAME/Configuration/Global/samplepath") )
        samplePath = qmc2Config->value("MAME/Configuration/Global/samplepath").toString().replace('/', '\\');
      else
        samplePath = "samples";
      if ( !samplePath.endsWith("\\") )
        samplePath += "\\";
      for (j = 0; j < items.count(); j++)
        addArgs << samplePath + items[j]->text().replace('/', '\\');
#else
      QString samplePath;
      if ( qmc2Config->contains("MAME/Configuration/Global/samplepath") )
        samplePath = qmc2Config->value("MAME/Configuration/Global/samplepath").toString();
      else
        samplePath = "samples";
      if ( !samplePath.endsWith("/") )
        samplePath += "/";
      for (j = 0; j < items.count(); j++)
        addArgs << samplePath + items[j]->text();
#endif
      args.removeAt(i);
      args << addArgs;
    }
  }

  ToolExecutor fileRemovalTool(this, command, args);
  fileRemovalTool.exec();
}

void SampleChecker::on_listWidgetSamplesGood_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::on_listWidgetSamplesGood_itemSelectionChanged()");
#endif

  if ( checkBoxSamplesSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetSamplesGood->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void SampleChecker::on_listWidgetSamplesBad_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SampleChecker::on_listWidgetSamplesBad_itemSelectionChanged()");
#endif

  if ( checkBoxSamplesSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetSamplesBad->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void SampleChecker::selectItem(QString gameName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::selectItem(QString gameName = %1)").arg(gameName));
#endif

  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
    case QMC2_VIEWGAMELIST_INDEX: {
      QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
      if ( gameItem ) {
        qmc2MainWindow->treeWidgetGamelist->clearSelection();
        qmc2MainWindow->treeWidgetGamelist->setCurrentItem(gameItem);
        qmc2MainWindow->treeWidgetGamelist->scrollToItem(gameItem, qmc2CursorPositioningMode);
        gameItem->setSelected(true);
      }
      break;
    }
    case QMC2_VIEWHIERARCHY_INDEX: {
      QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
      if ( hierarchyItem ) {
        qmc2MainWindow->treeWidgetHierarchy->clearSelection();
        qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
        qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
        hierarchyItem->setSelected(true);
      }
      break;
    }
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    case QMC2_VIEWCATEGORY_INDEX: {
      QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
      if ( categoryItem ) {
        qmc2MainWindow->treeWidgetCategoryView->clearSelection();
        qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
        qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
        categoryItem->setSelected(true);
      }
      break;
    }
    case QMC2_VIEWVERSION_INDEX: {
      QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
      if ( versionItem ) {
        qmc2MainWindow->treeWidgetVersionView->clearSelection();
        qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
        qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
        versionItem->setSelected(true);
      }
      break;
    }
#endif
  }
}

void SampleChecker::recursiveFileList(const QString &sDir, QStringList &fileNames)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SampleChecker::recursiveFileList(const QString& sDir = %1, QStringList &fileNames)").arg(sDir));
#endif

  QDir dir(sDir);
  QFileInfoList list = dir.entryInfoList();
  int i;
  for (i = 0; i < list.count(); i++) {
    QFileInfo info = list[i];
    QString path = info.filePath();
    if ( info.isDir() ) {
      // directory recursion
      if ( info.fileName() != ".." && info.fileName() != "." ) {
        recursiveFileList(path, fileNames);
        qApp->processEvents();
      }
    } else
      fileNames << path;
  }
} 
