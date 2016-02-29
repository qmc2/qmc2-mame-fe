#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QStatusBar>
#include <QApplication>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "projectwindow.h"
#include "projectwidget.h"
#include "ui_projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "qchdmansettings.h"

extern QtChdmanGuiSettings *globalConfig;
extern MainWindow *mainWindow;
extern quint64 runningProjects;

ProjectWidget::ProjectWidget(QWidget *parent, bool scriptElement, int type, QString sId, ScriptEngine *sEngine) :
	QWidget(parent),
	ui(new Ui::ProjectWidget)
{
	ui->setupUi(this);

	isScriptElement = scriptElement;
	scriptId = sId;
	scriptEngine = sEngine;
	status = QCHDMAN_PRJSTAT_IDLE;
	chdmanProc = NULL;
	terminatedOnDemand = askFileName = false;
	needsTabbedUiAdjustment = needsWindowedUiAdjustment = true;
	lastRc = -1;

	// prepare HD geometry template selector
	ui->comboBoxCreateHDFromTemplate->addItem(tr("Select"));
	ui->comboBoxCreateHDFromTemplate->insertSeparator(1);
	QMapIterator<QString, QList<DiskGeometry> > it(MainWindow::hardDiskTemplates);
	while ( it.hasNext() ) {
		it.next();
		foreach (DiskGeometry geo, it.value())
			ui->comboBoxCreateHDFromTemplate->addItem(it.key() + ": " + geo.name + QString(" (%1)").arg(mainWindow->humanReadable((qreal)geo.cyls * (qreal)geo.heads * (qreal)geo.sectors * (qreal)geo.sectorSize)));
	}

	// morph & clone groups
	copyGroups[QCHDMAN_PRJ_INFO]
			<< ui->lineEditInfoInputFile << ui->checkBoxInfoVerbose;
	copyGroups[QCHDMAN_PRJ_VERIFY]
			<< ui->lineEditVerifyInputFile << ui->lineEditVerifyParentInputFile;
	copyGroups[QCHDMAN_PRJ_COPY]
			<< ui->lineEditCopyInputFile << ui->lineEditCopyParentInputFile << ui->lineEditCopyOutputFile << ui->lineEditCopyParentOutputFile
			<< ui->checkBoxCopyForce << ui->spinBoxCopyProcessors << ui->spinBoxCopyInputStartByte << ui->spinBoxCopyInputStartHunk
			<< ui->spinBoxCopyInputBytes << ui->spinBoxCopyInputHunks << ui->spinBoxCopyHunkSize << ui->comboBoxCopyCompression;
	copyGroups[QCHDMAN_PRJ_CREATE_RAW]
			<< ui->lineEditCreateRawInputFile << 0 << ui->lineEditCreateRawOutputFile << ui->lineEditCreateRawParentOutputFile
			<< ui->checkBoxCreateRawForce << ui->spinBoxCreateRawProcessors << ui->spinBoxCreateRawInputStartByte << ui->spinBoxCreateRawInputStartHunk
			<< ui->spinBoxCreateRawInputBytes << ui->spinBoxCreateRawInputHunks << ui->spinBoxCreateRawHunkSize << ui->comboBoxCreateRawCompression
			<< ui->spinBoxCreateRawUnitSize;
	copyGroups[QCHDMAN_PRJ_CREATE_HD]
			<< ui->lineEditCreateHDInputFile << 0 << ui->lineEditCreateHDOutputFile << ui->lineEditCreateHDParentOutputFile
			<< ui->checkBoxCreateHDForce << ui->spinBoxCreateHDProcessors << ui->spinBoxCreateHDInputStartByte << ui->spinBoxCreateHDInputStartHunk
			<< ui->spinBoxCreateHDInputBytes << ui->spinBoxCreateHDInputHunks << ui->spinBoxCreateHDHunkSize << ui->comboBoxCreateHDCompression
			<< 0 << ui->spinBoxCreateHDCylinders << ui->spinBoxCreateHDHeads << ui->spinBoxCreateHDSectors
			<< ui->lineEditCreateHDIdentFile << ui->spinBoxCreateHDSectorSize;
	copyGroups[QCHDMAN_PRJ_CREATE_CD]
			<< ui->lineEditCreateCDInputFile << 0 << ui->lineEditCreateCDOutputFile << ui->lineEditCreateCDParentOutputFile
			<< ui->checkBoxCreateCDForce << ui->spinBoxCreateCDProcessors << 0 << 0
			<< 0 << 0 << ui->spinBoxCreateCDHunkSize << ui->comboBoxCreateCDCompression;
	copyGroups[QCHDMAN_PRJ_CREATE_LD]
			<< ui->lineEditCreateLDInputFile << 0 << ui->lineEditCreateLDOutputFile << ui->lineEditCreateLDParentOutputFile
			<< ui->checkBoxCreateLDForce << ui->spinBoxCreateLDProcessors << 0 << 0
			<< 0 << 0 << ui->spinBoxCreateLDHunkSize << ui->comboBoxCreateLDCompression
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << ui->spinBoxCreateLDInputStartFrame << ui->spinBoxCreateLDInputFrames;
	copyGroups[QCHDMAN_PRJ_EXTRACT_RAW]
			<< ui->lineEditExtractRawInputFile << ui->lineEditExtractRawParentInputFile << ui->lineEditExtractRawOutputFile << 0
			<< ui->checkBoxExtractRawForce << 0 << ui->spinBoxExtractRawInputStartByte << ui->spinBoxExtractRawInputStartHunk
			<< ui->spinBoxExtractRawInputBytes << ui->spinBoxExtractRawInputHunks;
	copyGroups[QCHDMAN_PRJ_EXTRACT_HD]
			<< ui->lineEditExtractHDInputFile << ui->lineEditExtractHDParentInputFile << ui->lineEditExtractHDOutputFile << 0
			<< ui->checkBoxExtractHDForce << 0 << ui->spinBoxExtractHDInputStartByte << ui->spinBoxExtractHDInputStartHunk
			<< ui->spinBoxExtractHDInputBytes << ui->spinBoxExtractHDInputHunks;
	copyGroups[QCHDMAN_PRJ_EXTRACT_CD]
			<< ui->lineEditExtractCDInputFile << ui->lineEditExtractCDParentInputFile << ui->lineEditExtractCDOutputFile << ui->lineEditExtractCDOutputBinFile
			<< ui->checkBoxExtractCDForce;
	copyGroups[QCHDMAN_PRJ_EXTRACT_LD]
			<< ui->lineEditExtractLDInputFile << ui->lineEditExtractLDOutputFile << ui->lineEditExtractLDParentInputFile << 0
			<< ui->checkBoxExtractLDForce << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << ui->spinBoxExtractLDInputStartFrame << ui->spinBoxExtractLDInputFrames;
	copyGroups[QCHDMAN_PRJ_DUMP_META]
			<< ui->lineEditDumpMetaInputFile << ui->lineEditDumpMetaOutputFile << 0 << 0
			<< ui->checkBoxDumpMetaForce << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< ui->lineEditDumpMetaTag << ui->spinBoxDumpMetaIndex;
	copyGroups[QCHDMAN_PRJ_ADD_META]
			<< ui->lineEditAddMetaInputFile << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< ui->lineEditAddMetaTag << ui->spinBoxAddMetaIndex << ui->lineEditAddMetaValueFile << ui->lineEditAddMetaValueText
			<< ui->checkBoxAddMetaNoCheckSum;
	copyGroups[QCHDMAN_PRJ_DEL_META]
			<< ui->lineEditDelMetaInputFile << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< 0 << 0 << 0 << 0
			<< ui->lineEditDelMetaTag << ui->spinBoxDelMetaIndex;

	// prepare compression selectors
	copyCompressors.clear();
	createRawCompressors.clear();
	createHDCompressors.clear();
	createCDCompressors.clear();
	createLDCompressors.clear();
	QList<QComboBox *> compressionBoxes;
	compressionBoxes << ui->comboBoxCopyCompression << ui->comboBoxCreateRawCompression << ui->comboBoxCreateHDCompression << ui->comboBoxCreateCDCompression << ui->comboBoxCreateLDCompression;
	foreach (QComboBox *cb, compressionBoxes) {
		cb->blockSignals(true);
		cb->insertItem(0, tr("default"));
		cb->setItemIcon(0, QIcon(":/images/compression.png"));
		cb->insertSeparator(1);
		cb->insertItem(2, tr("default"));
		cb->setItemIcon(2, QIcon(":/images/default.png"));
		cb->insertItem(3, tr("none"));
		cb->setItemIcon(3, QIcon(":/images/none.png"));
		cb->insertSeparator(4);
		int i = 5;
		foreach (QString cmp, mainWindow->compressionTypes) {
			cb->insertItem(i, cmp);
			cb->setItemIcon(i, QIcon(":/images/inactive.png"));
			cb->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
			i++;
		}
		cb->blockSignals(false);
	}

	// setup tools menu
	menuActions = new QMenu(this);
	actionLoad = menuActions->addAction(tr("Load"), this, SLOT(load()));
	actionSave = menuActions->addAction(tr("Save"), this, SLOT(save()));
	actionSaveAs = menuActions->addAction(tr("Save as..."), this, SLOT(triggerSaveAs()));

	menuCloneActions = new QMenu(tr("Clone to"), this);
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_INFO], tr("Info"), this, SLOT(clone()))] = QCHDMAN_PRJ_INFO;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_VERIFY], tr("Verify"), this, SLOT(clone()))] = QCHDMAN_PRJ_VERIFY;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_COPY], tr("Copy"), this, SLOT(clone()))] = QCHDMAN_PRJ_COPY;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_RAW], tr("CreateRaw"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_RAW;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_HD], tr("CreateHD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_HD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_CD], tr("CreateCD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_CD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_LD], tr("CreateLD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_LD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_RAW], tr("ExtractRaw"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_RAW;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_HD], tr("ExtractHD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_HD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_CD], tr("ExtractCD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_CD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_LD], tr("ExtractLD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_LD;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_DUMP_META], tr("DumpMeta"), this, SLOT(clone()))] = QCHDMAN_PRJ_DUMP_META;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_ADD_META], tr("AddMeta"), this, SLOT(clone()))] = QCHDMAN_PRJ_ADD_META;
	cloneActionMap[menuCloneActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_DEL_META], tr("DelMeta"), this, SLOT(clone()))] = QCHDMAN_PRJ_DEL_META;
	actionCloneMenu = menuActions->addMenu(menuCloneActions);

	menuMorphActions = new QMenu(tr("Morph to"), this);
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_INFO], tr("Info"), this, SLOT(morph()))] = QCHDMAN_PRJ_INFO;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_VERIFY], tr("Verify"), this, SLOT(morph()))] = QCHDMAN_PRJ_VERIFY;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_COPY], tr("Copy"), this, SLOT(morph()))] = QCHDMAN_PRJ_COPY;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_RAW], tr("CreateRaw"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_RAW;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_HD], tr("CreateHD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_HD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_CD], tr("CreateCD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_CD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_CREATE_LD], tr("CreateLD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_LD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_RAW], tr("ExtractRaw"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_RAW;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_HD], tr("ExtractHD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_HD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_CD], tr("ExtractCD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_CD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_EXTRACT_LD], tr("ExtractLD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_LD;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_DUMP_META], tr("DumpMeta"), this, SLOT(morph()))] = QCHDMAN_PRJ_DUMP_META;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_ADD_META], tr("AddMeta"), this, SLOT(morph()))] = QCHDMAN_PRJ_ADD_META;
	morphActionMap[menuMorphActions->addAction(mainWindow->iconMap[QCHDMAN_PRJ_DEL_META], tr("DelMeta"), this, SLOT(morph()))] = QCHDMAN_PRJ_DEL_META;
	actionMorphMenu = menuActions->addMenu(menuMorphActions);

	actionCopyStdoutToClipboard = menuActions->addAction(tr("Copy stdout to clipboard"), this, SLOT(copyStdoutToClipboard()));
	actionCopyStderrToClipboard = menuActions->addAction(tr("Copy stderr to clipboard"), this, SLOT(copyStderrToClipboard()));
	actionCopyCommandToClipboard = menuActions->addAction(tr("Copy command to clipboard"), this, SLOT(copyCommandToClipboard()));

	menuActions->insertSeparator(actionSave);
	menuActions->insertSeparator(actionCloneMenu);
	menuActions->insertSeparator(actionCopyStdoutToClipboard);
	menuActions->insertSeparator(actionCopyCommandToClipboard);
	ui->toolButtonActions->setMenu(menuActions);

	// linear gradient from red over yellow to green (this assumes that sub-window icons are in 64x64)
	linearGradient = QLinearGradient(QPointF(0, 0), QPointF(63, 0));
	linearGradient.setColorAt(0.0, QColor(255, 0, 0, 192));
	linearGradient.setColorAt(0.5, QColor(220, 220, 0, 192));
	linearGradient.setColorAt(1.0, QColor(0, 255, 0, 192));

	QFont f;
	f.fromString(globalConfig->preferencesLogFont());
	f.setPointSize(globalConfig->preferencesLogFontSize());
	ui->plainTextEditProjectLog->setFont(f);

	connect(ui->spinBoxCreateHDCylinders, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
	connect(ui->spinBoxCreateHDHeads, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
	connect(ui->spinBoxCreateHDSectors, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
	connect(ui->spinBoxCreateHDSectorSize, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
	connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(signalProgressUpdate(int)));

	if ( type != QCHDMAN_PRJ_UNKNOWN )
		setProjectType(type);

	QTimer::singleShot(0, this, SLOT(init()));
}

ProjectWidget::~ProjectWidget()
{
	delete ui;
}

void ProjectWidget::on_comboBoxProjectType_currentIndexChanged(int index)
{
	int widgetHeight = 0;

	if ( index == -1 )
		index = ui->comboBoxProjectType->currentIndex();

#if defined(Q_OS_MAC)
	bool isAquaStyle = globalConfig->preferencesGuiStyle().startsWith("Macintosh");
#endif

	switch ( index ) {
	case QCHDMAN_PRJ_INFO:
		widgetHeight = ui->frameInfo->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelInfoHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelInfoHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_VERIFY:
		widgetHeight = ui->frameVerify->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelVerifyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelVerifyHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_COPY:
		widgetHeight = ui->frameCopy->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelCopyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelCopyHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_CREATE_RAW:
		widgetHeight = ui->frameCreateRaw->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelCreateRawHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelCreateRawHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_CREATE_HD:
		widgetHeight = ui->frameCreateHD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelCreateHDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelCreateHDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_CREATE_CD:
		widgetHeight = ui->frameCreateCD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelCreateCDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelCreateCDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_CREATE_LD:
		widgetHeight = ui->frameCreateLD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelCreateLDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelCreateLDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_EXTRACT_RAW:
		widgetHeight = ui->frameExtractRaw->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelExtractRawHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelExtractRawHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_EXTRACT_HD:
		widgetHeight = ui->frameExtractHD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelExtractHDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelExtractHDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_EXTRACT_CD:
		widgetHeight = ui->frameExtractCD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelExtractCDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelExtractCDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_EXTRACT_LD:
		widgetHeight = ui->frameExtractLD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelExtractLDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelExtractLDHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_DUMP_META:
		widgetHeight = ui->frameDumpMeta->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelDumpMetaHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelDumpMetaHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_ADD_META:
		widgetHeight = ui->frameAddMeta->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelAddMetaHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelAddMetaHelp->margin();
#endif
		break;
	case QCHDMAN_PRJ_DEL_META:
		widgetHeight = ui->frameDelMeta->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
		if ( globalConfig->preferencesShowHelpTexts() )
			widgetHeight += ui->labelDelMetaHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
		if ( isAquaStyle )
			widgetHeight -= ui->labelDelMetaHelp->margin();
#endif
		break;
	}

	// adjust window icon
	if ( !isScriptElement )
		parentWidget()->setWindowIcon(currentIcon.isNull() ? mainWindow->iconMap[index] : currentIcon);

	// adjust splitter
	int splitterHeight = ui->splitter->height();
	double percent = (double)widgetHeight / (double)splitterHeight;
	int newHeight = int((double)splitterHeight * percent);
	QList<int> newSizes;
	newSizes << newHeight << splitterHeight - newHeight;
	ui->splitter->setSizes(newSizes);
}

void ProjectWidget::on_toolButtonRun_clicked(bool refreshArgsOnly)
{    
	arguments.clear();

	switch ( ui->comboBoxProjectType->currentIndex() ) {
	case QCHDMAN_PRJ_INFO:
		projectTypeName = tr("Info");
		arguments << "info";
		if ( !ui->lineEditInfoInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditInfoInputFile->text();
		if ( ui->checkBoxInfoVerbose->isChecked() )
			arguments << "--verbose";
		break;
	case QCHDMAN_PRJ_VERIFY:
		projectTypeName = tr("Verify");
		arguments << "verify";
		if ( !ui->lineEditVerifyInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditVerifyInputFile->text();
		if ( !ui->lineEditVerifyParentInputFile->text().isEmpty() )
			arguments << "--inputparent" << ui->lineEditVerifyParentInputFile->text();
		break;
	case QCHDMAN_PRJ_COPY:
		projectTypeName = tr("Copy");
		arguments << "copy";
		if ( !ui->lineEditCopyInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditCopyInputFile->text();
		if ( !ui->lineEditCopyParentInputFile->text().isEmpty() )
			arguments << "--parentinput" << ui->lineEditCopyParentInputFile->text();
		if ( !ui->lineEditCopyOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditCopyOutputFile->text();
		if ( !ui->lineEditCopyParentOutputFile->text().isEmpty() )
			arguments << "--parentoutput" << ui->lineEditCopyParentOutputFile->text();
		if ( ui->checkBoxCopyForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxCopyInputStartByte->value() >= 0 )
			arguments << "--inputstartbyte" << QString::number(ui->spinBoxCopyInputStartByte->value());
		if ( ui->spinBoxCopyInputStartHunk->value() >= 0 )
			arguments << "--inputstarthunk" << QString::number(ui->spinBoxCopyInputStartHunk->value());
		if ( ui->spinBoxCopyInputBytes->value() >= 0 )
			arguments << "--inputbytes" << QString::number(ui->spinBoxCopyInputBytes->value());
		if ( ui->spinBoxCopyInputHunks->value() >= 0 )
			arguments << "--inputhunks" << QString::number(ui->spinBoxCopyInputHunks->value());
		if ( ui->spinBoxCopyHunkSize->value() >= 0 )
			arguments << "--hunksize" << QString::number(ui->spinBoxCopyHunkSize->value());
		if ( !copyCompressors.isEmpty() )
			arguments << "--compression" << copyCompressors.join(",");
		else if ( ui->comboBoxCopyCompression->currentText() != tr("default") )
			arguments << "--compression" << "none";
		if ( ui->spinBoxCopyProcessors->value() >= 1 )
			arguments << "--numprocessors" << QString::number(ui->spinBoxCopyProcessors->value());
		break;
	case QCHDMAN_PRJ_CREATE_RAW:
		projectTypeName = tr("CreateRaw");
		arguments << "createraw";
		if ( !ui->lineEditCreateRawInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditCreateRawInputFile->text();
		if ( !ui->lineEditCreateRawOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditCreateRawOutputFile->text();
		if ( !ui->lineEditCreateRawParentOutputFile->text().isEmpty() )
			arguments << "--parentoutput" << ui->lineEditCreateRawParentOutputFile->text();
		if ( ui->checkBoxCreateRawForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxCreateRawInputStartByte->value() >= 0 )
			arguments << "--inputstartbyte" << QString::number(ui->spinBoxCreateRawInputStartByte->value());
		if ( ui->spinBoxCreateRawInputStartHunk->value() >= 0 )
			arguments << "--inputstarthunk" << QString::number(ui->spinBoxCreateRawInputStartHunk->value());
		if ( ui->spinBoxCreateRawInputBytes->value() >= 0 )
			arguments << "--inputbytes" << QString::number(ui->spinBoxCreateRawInputBytes->value());
		if ( ui->spinBoxCreateRawInputHunks->value() >= 0 )
			arguments << "--inputhunks" << QString::number(ui->spinBoxCreateRawInputHunks->value());
		if ( ui->spinBoxCreateRawHunkSize->value() > 0 )
			arguments << "--hunksize" << QString::number(ui->spinBoxCreateRawHunkSize->value());
		if ( ui->spinBoxCreateRawUnitSize->value() > 0 )
			arguments << "--unitsize" << QString::number(ui->spinBoxCreateRawUnitSize->value());
		if ( !createRawCompressors.isEmpty() )
			arguments << "--compression" << createRawCompressors.join(",");
		else if ( ui->comboBoxCreateRawCompression->currentText() != tr("default") )
			arguments << "--compression" << "none";
		if ( ui->spinBoxCreateRawProcessors->value() >= 1 )
			arguments << "--numprocessors" << QString::number(ui->spinBoxCreateRawProcessors->value());
		break;
	case QCHDMAN_PRJ_CREATE_HD:
		projectTypeName = tr("CreateHD");
		arguments << "createhd";
		if ( !ui->lineEditCreateHDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditCreateHDInputFile->text();
		if ( !ui->lineEditCreateHDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditCreateHDOutputFile->text();
		if ( !ui->lineEditCreateHDParentOutputFile->text().isEmpty() )
			arguments << "--parentoutput" << ui->lineEditCreateHDParentOutputFile->text();
		if ( ui->checkBoxCreateHDForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxCreateHDInputStartByte->value() >= 0 )
			arguments << "--inputstartbyte" << QString::number(ui->spinBoxCreateHDInputStartByte->value());
		if ( ui->spinBoxCreateHDInputStartHunk->value() >= 0 )
			arguments << "--inputstarthunk" << QString::number(ui->spinBoxCreateHDInputStartHunk->value());
		if ( ui->spinBoxCreateHDInputBytes->value() >= 0 )
			arguments << "--inputbytes" << QString::number(ui->spinBoxCreateHDInputBytes->value());
		if ( ui->spinBoxCreateHDInputHunks->value() >= 0 )
			arguments << "--inputhunks" << QString::number(ui->spinBoxCreateHDInputHunks->value());
		if ( ui->spinBoxCreateHDHunkSize->value() >= 0 )
			arguments << "--hunksize" << QString::number(ui->spinBoxCreateHDHunkSize->value());
		if ( !createHDCompressors.isEmpty() )
			arguments << "--compression" << createHDCompressors.join(",");
		else if ( ui->comboBoxCreateHDCompression->currentText() != tr("default") )
			arguments << "--compression" << "none";
		if ( ui->spinBoxCreateHDProcessors->value() >= 1 )
			arguments << "--numprocessors" << QString::number(ui->spinBoxCreateHDProcessors->value());
		if ( !ui->lineEditCreateHDIdentFile->text().isEmpty() )
			arguments << "--ident" << ui->lineEditCreateHDIdentFile->text();
		if ( ui->spinBoxCreateHDSectorSize->value() >= 0 )
			arguments << "--sectorsize" << QString::number(ui->spinBoxCreateHDSectorSize->value());
		if ( ui->spinBoxCreateHDCylinders->value() >= 0 ) {
			QString chs = QString::number(ui->spinBoxCreateHDCylinders->value()) + ",";
			if ( ui->spinBoxCreateHDHeads->value() >= 0 )
				chs += QString::number(ui->spinBoxCreateHDHeads->value()) + ",";
			else
				chs += "1,";
			if ( ui->spinBoxCreateHDSectors->value() >= 0 )
				chs += QString::number(ui->spinBoxCreateHDSectors->value());
			else
				chs += "1";
			arguments << "--chs" << chs;
		}
		break;
	case QCHDMAN_PRJ_CREATE_CD:
		projectTypeName = tr("CreateCD");
		arguments << "createcd";
		if ( !ui->lineEditCreateCDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditCreateCDInputFile->text();
		if ( !ui->lineEditCreateCDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditCreateCDOutputFile->text();
		if ( !ui->lineEditCreateCDParentOutputFile->text().isEmpty() )
			arguments << "--parentoutput" << ui->lineEditCreateCDParentOutputFile->text();
		if ( ui->checkBoxCreateCDForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxCreateCDHunkSize->value() >= 0 )
			arguments << "--hunksize" << QString::number(ui->spinBoxCreateCDHunkSize->value());
		if ( !createCDCompressors.isEmpty() )
			arguments << "--compression" << createCDCompressors.join(",");
		else if ( ui->comboBoxCreateCDCompression->currentText() != tr("default") )
			arguments << "--compression" << "none";
		if ( ui->spinBoxCreateCDProcessors->value() >= 1 )
			arguments << "--numprocessors" << QString::number(ui->spinBoxCreateCDProcessors->value());
		break;
	case QCHDMAN_PRJ_CREATE_LD:
		projectTypeName = tr("CreateLD");
		arguments << "createld";
		if ( !ui->lineEditCreateLDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditCreateLDInputFile->text();
		if ( !ui->lineEditCreateLDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditCreateLDOutputFile->text();
		if ( !ui->lineEditCreateLDParentOutputFile->text().isEmpty() )
			arguments << "--parentoutput" << ui->lineEditCreateLDParentOutputFile->text();
		if ( ui->checkBoxCreateLDForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxCreateLDInputStartFrame->value() >= 0 )
			arguments << "--inputstartframe" << QString::number(ui->spinBoxCreateLDInputStartFrame->value());
		if ( ui->spinBoxCreateLDInputFrames->value() >= 0 )
			arguments << "--inputframes" << QString::number(ui->spinBoxCreateLDInputFrames->value());
		if ( ui->spinBoxCreateLDHunkSize->value() >= 0 )
			arguments << "--hunksize" << QString::number(ui->spinBoxCreateLDHunkSize->value());
		if ( !createLDCompressors.isEmpty() )
			arguments << "--compression" << createLDCompressors.join(",");
		else if ( ui->comboBoxCreateLDCompression->currentText() != tr("default") )
			arguments << "--compression" << "none";
		if ( ui->spinBoxCreateLDProcessors->value() >= 1 )
			arguments << "--numprocessors" << QString::number(ui->spinBoxCreateLDProcessors->value());
		break;
	case QCHDMAN_PRJ_EXTRACT_RAW:
		projectTypeName = tr("ExtractRaw");
		arguments << "extractraw";
		if ( !ui->lineEditExtractRawInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditExtractRawInputFile->text();
		if ( !ui->lineEditExtractRawParentInputFile->text().isEmpty() )
			arguments << "--parentinput" << ui->lineEditExtractRawParentInputFile->text();
		if ( !ui->lineEditExtractRawOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditExtractRawOutputFile->text();
		if ( ui->checkBoxExtractRawForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxExtractRawInputStartByte->value() >= 0 )
			arguments << "--inputstartbyte" << QString::number(ui->spinBoxExtractRawInputStartByte->value());
		if ( ui->spinBoxExtractRawInputStartHunk->value() >= 0 )
			arguments << "--inputstarthunk" << QString::number(ui->spinBoxExtractRawInputStartHunk->value());
		if ( ui->spinBoxExtractRawInputBytes->value() >= 0 )
			arguments << "--inputbytes" << QString::number(ui->spinBoxExtractRawInputBytes->value());
		if ( ui->spinBoxExtractRawInputHunks->value() >= 0 )
			arguments << "--inputhunks" << QString::number(ui->spinBoxExtractRawInputHunks->value());
		break;
	case QCHDMAN_PRJ_EXTRACT_HD:
		projectTypeName = tr("ExtractHD");
		arguments << "extracthd";
		if ( !ui->lineEditExtractHDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditExtractHDInputFile->text();
		if ( !ui->lineEditExtractHDParentInputFile->text().isEmpty() )
			arguments << "--parentinput" << ui->lineEditExtractHDParentInputFile->text();
		if ( !ui->lineEditExtractHDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditExtractHDOutputFile->text();
		if ( ui->checkBoxExtractHDForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxExtractHDInputStartByte->value() >= 0 )
			arguments << "--inputstartbyte" << QString::number(ui->spinBoxExtractHDInputStartByte->value());
		if ( ui->spinBoxExtractHDInputStartHunk->value() >= 0 )
			arguments << "--inputstarthunk" << QString::number(ui->spinBoxExtractHDInputStartHunk->value());
		if ( ui->spinBoxExtractHDInputBytes->value() >= 0 )
			arguments << "--inputbytes" << QString::number(ui->spinBoxExtractHDInputBytes->value());
		if ( ui->spinBoxExtractHDInputHunks->value() >= 0 )
			arguments << "--inputhunks" << QString::number(ui->spinBoxExtractHDInputHunks->value());
		break;
	case QCHDMAN_PRJ_EXTRACT_CD:
		projectTypeName = tr("ExtractCD");
		arguments << "extractcd";
		if ( !ui->lineEditExtractCDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditExtractCDInputFile->text();
		if ( !ui->lineEditExtractCDParentInputFile->text().isEmpty() )
			arguments << "--parentinput" << ui->lineEditExtractCDParentInputFile->text();
		if ( !ui->lineEditExtractCDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditExtractCDOutputFile->text();
		if ( !ui->lineEditExtractCDOutputBinFile->text().isEmpty() )
			arguments << "--outputbin" << ui->lineEditExtractCDOutputBinFile->text();
		if ( ui->checkBoxExtractCDForce->isChecked() )
			arguments << "--force";
		break;
	case QCHDMAN_PRJ_EXTRACT_LD:
		projectTypeName = tr("ExtractLD");
		arguments << "extractld";
		if ( !ui->lineEditExtractLDInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditExtractLDInputFile->text();
		if ( !ui->lineEditExtractLDOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditExtractLDOutputFile->text();
		if ( !ui->lineEditExtractLDParentInputFile->text().isEmpty() )
			arguments << "--parentinput" << ui->lineEditExtractLDParentInputFile->text();
		if ( ui->checkBoxExtractLDForce->isChecked() )
			arguments << "--force";
		if ( ui->spinBoxExtractLDInputStartFrame->value() >= 0 )
			arguments << "--inputstartframe" << QString::number(ui->spinBoxExtractLDInputStartFrame->value());
		if ( ui->spinBoxExtractLDInputFrames->value() >= 0 )
			arguments << "--inputframes" << QString::number(ui->spinBoxExtractLDInputFrames->value());
		break;
	case QCHDMAN_PRJ_DUMP_META:
		projectTypeName = tr("DumpMeta");
		arguments << "dumpmeta";
		if ( !ui->lineEditDumpMetaInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditDumpMetaInputFile->text();
		if ( !ui->lineEditDumpMetaOutputFile->text().isEmpty() )
			arguments << "--output" << ui->lineEditDumpMetaOutputFile->text();
		if ( ui->checkBoxDumpMetaForce->isChecked() )
			arguments << "--force";
		if ( !ui->lineEditDumpMetaTag->text().isEmpty() )
			arguments << "--tag" << ui->lineEditDumpMetaTag->text();
		if ( ui->spinBoxDumpMetaIndex->value() > 0 )
			arguments << "--index" << QString::number(ui->spinBoxDumpMetaIndex->value());
		break;
	case QCHDMAN_PRJ_ADD_META:
		projectTypeName = tr("AddMeta");
		arguments << "addmeta";
		if ( !ui->lineEditAddMetaInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditAddMetaInputFile->text();
		if ( !ui->lineEditAddMetaValueFile->text().isEmpty() )
			arguments << "--valuefile" << ui->lineEditAddMetaValueFile->text();
		if ( !ui->lineEditAddMetaValueText->text().isEmpty() )
			arguments << "--valuetext" << ui->lineEditAddMetaValueText->text();
		if ( !ui->lineEditAddMetaTag->text().isEmpty() )
			arguments << "--tag" << ui->lineEditAddMetaTag->text();
		if ( ui->spinBoxAddMetaIndex->value() > 0 )
			arguments << "--index" << QString::number(ui->spinBoxAddMetaIndex->value());
		if ( ui->checkBoxAddMetaNoCheckSum->isChecked() )
			arguments << "--nochecksum";
		break;
	case QCHDMAN_PRJ_DEL_META:
		projectTypeName = tr("DelMeta");
		arguments << "delmeta";
		if ( !ui->lineEditDelMetaInputFile->text().isEmpty() )
			arguments << "--input" << ui->lineEditDelMetaInputFile->text();
		if ( !ui->lineEditDelMetaTag->text().isEmpty() )
			arguments << "--tag" << ui->lineEditDelMetaTag->text();
		if ( ui->spinBoxDelMetaIndex->value() > 0 )
			arguments << "--index" << QString::number(ui->spinBoxDelMetaIndex->value());
		break;
	}

	if ( refreshArgsOnly )
		return;

	ui->progressBar->setRange(0, 100);

	chdmanProc = new QProcess(this);

	connect(chdmanProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
	connect(chdmanProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
	connect(chdmanProc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
	connect(chdmanProc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
	connect(chdmanProc, SIGNAL(started()), this, SLOT(started()));

	ui->plainTextEditProjectLog->clear();
	log(tr("starting process"));

	stdoutOutput.clear();
	stderrOutput.clear();

	terminatedOnDemand = false;
	chdmanProc->start(globalConfig->preferencesChdmanBinary(), arguments);
	ui->toolButtonRun->setEnabled(false);
	ui->comboBoxProjectType->setEnabled(false);
	menuMorphActions->setEnabled(false);
	actionLoad->setEnabled(false);
	ui->progressBar->setFormat(tr("Starting"));
	emit progressFormatChanged(tr("Starting"));
	ui->progressBar->setValue(0);
}

void ProjectWidget::on_toolButtonStop_clicked()
{
	log(tr("terminating process"));
	terminatedOnDemand = true;
	chdmanProc->terminate();
}

void ProjectWidget::started()
{
	projectTimer.start();
	lastRc = 0;
	runningProjects++;
#if defined(Q_OS_WIN)
	log(tr("process started: PID = %1").arg(chdmanProc->pid()->dwProcessId));
#else
	log(tr("process started: PID = %1").arg(chdmanProc->pid()));
#endif
	status = QCHDMAN_PRJSTAT_RUNNING;
	ui->toolButtonStop->setEnabled(true);
	ui->progressBar->setFormat(tr("Running"));
	emit progressFormatChanged(tr("Running"));
	emit processStarted(this);
}

void ProjectWidget::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	runningProjects--;
	QString statusString = tr("normal");
	status = QCHDMAN_PRJSTAT_FINISHED;
	if ( exitStatus == QProcess::CrashExit ) {
		if ( terminatedOnDemand ) {
			statusString = tr("terminated");
			status = QCHDMAN_PRJSTAT_TERMINATED;
		} else {
			statusString = tr("crashed");
			status = QCHDMAN_PRJSTAT_CRASHED;
		}
	}
	lastRc = exitCode;
	QTime execTime;
	execTime = execTime.addMSecs(projectTimer.elapsed());
	log(tr("process finished: exitCode = %1, exitStatus = %2, execTime = %3").arg(exitCode).arg(statusString).arg(execTime.toString("hh:mm:ss.zzz")));
	if ( !isScriptElement )
		parentWidget()->setWindowIcon(mainWindow->iconMap[ui->comboBoxProjectType->currentIndex()]);
	currentIcon = QIcon();
	ui->toolButtonRun->setEnabled(true);
	ui->toolButtonStop->setEnabled(false);
	ui->comboBoxProjectType->setEnabled(true);
	menuMorphActions->setEnabled(true);
	actionLoad->setEnabled(true);
	ui->progressBar->setFormat(tr("Idle"));
	emit progressFormatChanged(tr("Idle"));
	ui->progressBar->setValue(0);
	emit processFinished(this);
}

void ProjectWidget::readyReadStandardOutput()
{
	QString s = chdmanProc->readAllStandardOutput();
	stdoutOutput += s;
	QStringList sl = s.split("\n");
	int i;
	for (i = 0; i < sl.count(); i++) {
		s = sl[i];
		s.remove(QRegExp("\\s+$"));
		if ( !s.isEmpty() ) {
			if ( globalConfig->preferencesLogChannelNames() )
				log(tr("stdout") + ": " + s);
			else
				log(s);
		}
	}
}

void ProjectWidget::readyReadStandardError()
{
	QString s = chdmanProc->readAllStandardError();
	stderrOutput += s;
	QStringList sl = s.split(QRegExp("(\\\n|\\\r)"), QString::SkipEmptyParts);
	sl.removeDuplicates();
	int percent = 0;
	for (int i = 0; i < sl.count(); i++) {
		s = sl[i];
		s.remove(QRegExp("\\s+$"));
		if ( !s.isEmpty() ) {
			if ( globalConfig->preferencesLogChannelNames() )
				log(tr("stderr") + ": " + s);
			else
				log(s);
			switch ( ui->comboBoxProjectType->currentIndex() ) {
			case QCHDMAN_PRJ_VERIFY:
			case QCHDMAN_PRJ_COPY:
			case QCHDMAN_PRJ_CREATE_RAW:
			case QCHDMAN_PRJ_CREATE_HD:
			case QCHDMAN_PRJ_CREATE_CD:
			case QCHDMAN_PRJ_CREATE_LD:
			case QCHDMAN_PRJ_EXTRACT_RAW:
			case QCHDMAN_PRJ_EXTRACT_HD:
			case QCHDMAN_PRJ_EXTRACT_CD:
			case QCHDMAN_PRJ_EXTRACT_LD:
				if ( s.contains(QRegExp(", \\d+\\.\\d+\\%\\ complete\\.\\.\\.")) ) {
					QRegExp rx(", (\\d+)\\.(\\d+)\\%\\ complete\\.\\.\\.");
					int pos = rx.indexIn(s);
					if ( pos > -1 ) {
						int decimal = rx.cap(2).toInt();
						percent = rx.cap(1).toInt() + (decimal >= 5 ? 1 : 0);
					}
				} else if ( s.contains("Compression complete ... final ratio =") || s.contains("Extraction complete") )
					percent = 100;
				break;
			}
		}
	}
	ui->progressBar->setValue(percent);

	QPixmap pm(mainWindow->iconMap[ui->comboBoxProjectType->currentIndex()].pixmap(64, 64));
	QPainter p(&pm);
	int w = int((qreal)pm.height() * (qreal)percent / 100.0) + 1;
	p.fillRect(0, 47, 64, 16, QColor(64, 64, 64, 64));
	p.fillRect(0, 47, w, 16, QBrush(linearGradient));
	p.end();
	QIcon icon;
	icon.addPixmap(pm);
	if ( !isScriptElement )
		parentWidget()->setWindowIcon(icon);
	currentIcon = icon;
}

void ProjectWidget::error(QProcess::ProcessError processError)
{
	QString errString;
	bool doLog = true;

	switch ( processError ) {
	case QProcess::FailedToStart:
		errString = tr("failed to start");
		status = QCHDMAN_PRJSTAT_ERROR;
		break;
	case QProcess::Crashed:
		errString = tr("crashed");
		status = QCHDMAN_PRJSTAT_CRASHED;
		if ( terminatedOnDemand ) {
			doLog = false;
			status = QCHDMAN_PRJSTAT_TERMINATED;
		}
		else
			break;
	case QProcess::Timedout:
		errString = tr("timed out");
		status = QCHDMAN_PRJSTAT_ERROR;
		break;
	case QProcess::WriteError:
		errString = tr("write error");
		status = QCHDMAN_PRJSTAT_ERROR;
		break;
	case QProcess::ReadError:
		errString = tr("read error");
		status = QCHDMAN_PRJSTAT_ERROR;
		break;
	case QProcess::UnknownError:
	default:
		errString = tr("unknown");
		status = QCHDMAN_PRJSTAT_ERROR;
		break;
	}

	if ( doLog )
		log(tr("process error: %1").arg(errString));

	ui->toolButtonRun->setEnabled(true);
	ui->toolButtonStop->setEnabled(false);
	ui->comboBoxProjectType->setEnabled(true);
	menuMorphActions->setEnabled(true);
	actionLoad->setEnabled(true);
	ui->progressBar->setFormat(tr("Idle"));
	emit progressFormatChanged(tr("Idle"));
	ui->progressBar->setValue(0);
	lastRc = -1;
}

void ProjectWidget::on_toolButtonBrowseInfoInputFile_clicked()
{
	QString folder = ui->lineEditInfoInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditInfoInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyInputFile_clicked()
{
	QString folder = ui->lineEditVerifyInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditVerifyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyParentInputFile_clicked()
{
	QString folder = ui->lineEditVerifyParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditVerifyParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyInputFile_clicked()
{
	QString folder = ui->lineEditCopyInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCopyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyOutputFile_clicked()
{
	QString folder = ui->lineEditCopyOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCopyOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentInputFile_clicked()
{
	QString folder = ui->lineEditCopyParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCopyParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentOutputFile_clicked()
{
	QString folder = ui->lineEditCopyParentOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCopyParentOutputFile->setText(s);
}

void ProjectWidget::on_comboBoxCopyCompression_currentIndexChanged(int index)
{
	updateCompression(ui->comboBoxCopyCompression, &copyCompressors, index);
}

void ProjectWidget::on_toolButtonBrowseCreateRawInputFile_clicked()
{
	QString folder = ui->lineEditCreateRawInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateRawInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateRawOutputFile_clicked()
{
	QString folder = ui->lineEditCreateRawOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateRawOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateRawParentOutputFile_clicked()
{
	QString folder = ui->lineEditCreateRawParentOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateRawParentOutputFile->setText(s);
}

void ProjectWidget::on_comboBoxCreateRawCompression_currentIndexChanged(int index)
{
	updateCompression(ui->comboBoxCreateRawCompression, &createRawCompressors, index);
}

void ProjectWidget::on_toolButtonBrowseCreateHDInputFile_clicked()
{
	QString folder = ui->lineEditCreateHDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateHDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDOutputFile_clicked()
{
	QString folder = ui->lineEditCreateHDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateHDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDParentOutputFile_clicked()
{
	QString folder = ui->lineEditCreateHDParentOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateHDParentOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDIdentFile_clicked()
{
	QString folder = ui->lineEditCreateHDIdentFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose ident file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateHDIdentFile->setText(s);
}

void ProjectWidget::on_comboBoxCreateHDCompression_currentIndexChanged(int index)
{
	updateCompression(ui->comboBoxCreateHDCompression, &createHDCompressors, index);
}

void ProjectWidget::on_comboBoxCreateHDFromTemplate_currentIndexChanged(int index)
{
	if ( index > 0 ) {
		QStringList entryInfo = ui->comboBoxCreateHDFromTemplate->currentText().split(": ", QString::SkipEmptyParts);
		QString vendorName = entryInfo[0];
		QString diskName = entryInfo[1].remove(QRegExp(" \\(.*\\)$"));
		QList<DiskGeometry> geoList = MainWindow::hardDiskTemplates[vendorName];
		bool found = false;
		DiskGeometry geo;
		for (int i = 0; i < geoList.count() && !found; i++) {
			geo = geoList[i];
			found = ( geo.name == diskName );
		}
		if ( found ) {
			ui->spinBoxCreateHDCylinders->setValue(geo.cyls);
			ui->spinBoxCreateHDHeads->setValue(geo.heads);
			ui->spinBoxCreateHDSectors->setValue(geo.sectors);
			ui->spinBoxCreateHDSectorSize->setValue(geo.sectorSize);
			int noneIndex = ui->comboBoxCreateHDCompression->findText(tr("none"));
			if ( noneIndex >= 0 )
				ui->comboBoxCreateHDCompression->setCurrentIndex(noneIndex);
		}
		ui->comboBoxCreateHDFromTemplate->setCurrentIndex(0);
	}
}

void ProjectWidget::updateCreateHDDiskCapacity()
{
	qreal cyls = ui->spinBoxCreateHDCylinders->value();
	qreal heads = ui->spinBoxCreateHDHeads->value();
	qreal sectors = ui->spinBoxCreateHDSectors->value();
	qreal bps = ui->spinBoxCreateHDSectorSize->value();

	if ( cyls < 0 || heads < 0 || sectors < 0 || bps < 0 )
		ui->labelCreateHDDiskCapacity->setText(tr("unknown"));
	else
		ui->labelCreateHDDiskCapacity->setText(mainWindow->humanReadable(cyls * heads * sectors * bps));
}

void ProjectWidget::on_toolButtonBrowseCreateCDInputFile_clicked()
{
	QString folder = ui->lineEditCreateCDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("Compatible files (*.cue *.toc *.gdi *.nrg *.iso)") + ";;" + tr("CUE files (*.cue)") + ";;" + tr("TOC files (*.toc)") + ";;" + tr("GDI files (*.gdi)") + ";;" + tr("NRG files (*.nrg)") + ";;" + tr("ISO files (*.iso)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateCDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateCDOutputFile_clicked()
{
	QString folder = ui->lineEditCreateCDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateCDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateCDParentOutputFile_clicked()
{
	QString folder = ui->lineEditCreateCDParentOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateCDParentOutputFile->setText(s);
}

void ProjectWidget::on_comboBoxCreateCDCompression_currentIndexChanged(int index)
{
	updateCompression(ui->comboBoxCreateCDCompression, &createCDCompressors, index);
}

void ProjectWidget::on_toolButtonBrowseCreateLDInputFile_clicked()
{
	QString folder = ui->lineEditCreateLDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("AVI files (*.avi)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose LD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateLDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateLDOutputFile_clicked()
{
	QString folder = ui->lineEditCreateLDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateLDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateLDParentOutputFile_clicked()
{
	QString folder = ui->lineEditCreateLDParentOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDOutputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditCreateLDParentOutputFile->setText(s);
}

void ProjectWidget::on_comboBoxCreateLDCompression_currentIndexChanged(int index)
{
	updateCompression(ui->comboBoxCreateLDCompression, &createLDCompressors, index);
}

void ProjectWidget::on_toolButtonBrowseExtractRawInputFile_clicked()
{
	QString folder = ui->lineEditExtractRawInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractRawInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractRawOutputFile_clicked()
{
	QString folder = ui->lineEditExtractRawOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("Raw disk image (*.img)") + ";;" + tr("Mac disk image (*.dmg)") + ";;" + tr("Apple IIgs disk image (*.2mg)") + ";;" + tr("FM-Towns disk image (*.h0 *.h1 *.h2 *.h3 *.h4)") + ";;" + tr("IDE64 disk image (*.hdd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractRawOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractRawParentInputFile_clicked()
{
	QString folder = ui->lineEditExtractRawParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractRawParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractHDInputFile_clicked()
{
	QString folder = ui->lineEditExtractHDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractHDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractHDOutputFile_clicked()
{
	QString folder = ui->lineEditExtractHDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("Raw disk image (*.img)") + ";;" +
			tr("Mac disk image (*.dmg)") + ";;" +
			tr("Apple IIgs disk image (*.2mg)") + ";;" +
			tr("FM-Towns disk image (*.h0 *.h1 *.h2 *.h3 *.h4)") + ";;" +
			tr("IDE64 disk image (*.hdd)") + ";;" +
			tr("X68k SASI disk image (*.hdf)") + ";;" +
			tr("X68k SCSI disk image (*.hds)") + ";;" +
			tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractHDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractHDParentInputFile_clicked()
{
	QString folder = ui->lineEditExtractHDParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractHDParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractCDInputFile_clicked()
{
	QString folder = ui->lineEditExtractCDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractCDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractCDOutputFile_clicked()
{
	QString folder = ui->lineEditExtractCDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("Compatible files (*.cue *.toc *.gdi *.nrg *.iso)") + ";;" + tr("CUE files (*.cue)") + ";;" + tr("TOC files (*.toc)") + ";;" + tr("GDI files (*.gdi)") + ";;" + tr("NRG files (*.nrg)") + ";;" + tr("ISO files (*.iso)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractCDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractCDParentInputFile_clicked()
{
	QString folder = ui->lineEditExtractCDParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractCDParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractCDOutputBinFile_clicked()
{
	QString folder = ui->lineEditExtractCDOutputBinFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("Binary CD files (*.bin)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose binary output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractCDOutputBinFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractLDInputFile_clicked()
{
	QString folder = ui->lineEditExtractLDInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractLDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractLDOutputFile_clicked()
{
	QString folder = ui->lineEditExtractLDOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("AVI files (*.avi)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractLDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractLDParentInputFile_clicked()
{
	QString folder = ui->lineEditExtractLDParentInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditExtractLDParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseDumpMetaInputFile_clicked()
{
	QString folder = ui->lineEditDumpMetaInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditDumpMetaInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseDumpMetaOutputFile_clicked()
{
	QString folder = ui->lineEditDumpMetaOutputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredOutputFolder;
	QString filter = tr("All files (*)");
	QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditDumpMetaOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseAddMetaInputFile_clicked()
{
	QString folder = ui->lineEditAddMetaInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditAddMetaInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseAddMetaValueFile_clicked()
{
	QString folder = ui->lineEditAddMetaValueFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredInputFolder;
	QString filter = tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose meta-data value file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditAddMetaValueFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseDelMetaInputFile_clicked()
{
	QString folder = ui->lineEditDelMetaInputFile->text();
	if ( folder.isEmpty() )
		folder = mainWindow->preferredCHDInputFolder;
	QString filter = tr("CHD files (*.chd)") + ";;" + tr("All files (*)");
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, filter, 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditDelMetaInputFile->setText(s);
}

void ProjectWidget::init()
{
	on_comboBoxProjectType_currentIndexChanged(QCHDMAN_PRJ_INFO);
}

void ProjectWidget::log(QString message)
{
	if ( isScriptElement ) {
		message.prepend(scriptId + ": " + projectTypeName + ": ");
		scriptEngine->log(message);
	} else {
		message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + projectTypeName + ": ");
		ui->plainTextEditProjectLog->appendPlainText(message);
	}
}

void ProjectWidget::setLogFont(QFont f)
{
	ui->plainTextEditProjectLog->setFont(f);
}

void ProjectWidget::setProjectType(int newType)
{
	ui->comboBoxProjectType->setCurrentIndex(newType);
	qApp->processEvents();
	on_comboBoxProjectType_currentIndexChanged(-1);
}

void ProjectWidget::copyStdoutToClipboard()
{
	qApp->clipboard()->setText(stdoutOutput);
}

void ProjectWidget::copyStderrToClipboard()
{
	qApp->clipboard()->setText(stderrOutput);
}

void ProjectWidget::copyCommandToClipboard()
{
	on_toolButtonRun_clicked(true);
	QString command = globalConfig->preferencesChdmanBinary();
	foreach (QString arg, arguments) {
		if ( arg.contains(QRegExp("\\s")) )
			command += " \"" + arg + "\"";
		else
			command += " " + arg;
	}
	qApp->clipboard()->setText(command);
}

void ProjectWidget::updateCompression(QComboBox *cb, QStringList *cmp, int index)
{
	bool isDefault = false;

	if ( index == 2 ) { // default
		isDefault = true;
		cmp->clear();
		for (int i = 5; i < cb->count(); i++) {
			cb->setItemIcon(i, QIcon(":/images/inactive.png"));
			cb->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
		}
	} else if ( index == 3 ) { // none
		cmp->clear();
		for (int i = 5; i < cb->count(); i++) {
			cb->setItemIcon(i, QIcon(":/images/inactive.png"));
			cb->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
		}
	} else if ( index > 4 ) { // toggles
		cmp->clear();
		if ( cb->itemData(index, Qt::WhatsThisRole).toString() == QCHDMAN_ITEM_ACTIVE ) {
			cb->setItemIcon(index, QIcon(":/images/inactive.png"));
			cb->setItemData(index, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
		} else {
			cb->setItemIcon(index, QIcon(":/images/active.png"));
			cb->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
		}
		for (int i = 5; i < cb->count(); i++)
			if ( cb->itemData(i, Qt::WhatsThisRole).toString() == QCHDMAN_ITEM_ACTIVE )
				*cmp << cb->itemText(i).split(" ", QString::SkipEmptyParts)[0];
	}

	if ( isDefault )
		cb->setItemText(0, tr("default"));
	else if ( cmp->isEmpty() )
		cb->setItemText(0, tr("none"));
	else
		cb->setItemText(0, cmp->join(","));

	cb->blockSignals(true);
	cb->setCurrentIndex(0);
	cb->blockSignals(false);
}

void ProjectWidget::load(const QString &fileName, QString *buffer)
{
	QString projectName = fileName;

	if ( buffer == NULL && projectName.isEmpty() ) {
		projectName = QFileDialog::getOpenFileName(this, tr("Choose project file"), QString(), tr("Project files (*.prj)") + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( projectName.isNull() )
			return;
	}

	QFile loadFile(projectName);
	QTextStream ts;
	QByteArray ba(buffer != NULL ? buffer->toUtf8().constData() : "");
	QBuffer baBuffer(&ba);

	if ( buffer != NULL ) {
		baBuffer.open(QBuffer::ReadOnly);
		ts.setDevice(&baBuffer);
	} else if ( loadFile.open(QIODevice::ReadOnly | QIODevice::Text) )
		ts.setDevice(&loadFile);

	if ( ts.device() ) {
		int projectType = QCHDMAN_PRJ_UNKNOWN;
		while ( !ts.atEnd() && projectType == QCHDMAN_PRJ_UNKNOWN ) {
			QString line = ts.readLine().trimmed();
			if ( line.startsWith("ProjectType = ") ) {
				projectType = mainWindow->projectTypes.indexOf(line.remove("ProjectType = "));
			} else
				continue;
		}
		if ( projectType != QCHDMAN_PRJ_UNKNOWN ) {
			ts.seek(0);
			while ( !ts.atEnd() ) {
				QString line = ts.readLine().trimmed();
				if ( line.startsWith("#") )
					continue;
				switch ( projectType ) {
				case QCHDMAN_PRJ_INFO:
					if ( line.startsWith("InfoInputFile = ") )
						ui->lineEditInfoInputFile->setText(line.remove("InfoInputFile = "));
					if ( line.startsWith("InfoVerbose = ") )
						ui->checkBoxInfoVerbose->setChecked(line.remove("InfoVerbose = ").toInt());
					break;
				case QCHDMAN_PRJ_VERIFY:
					if ( line.startsWith("VerifyInputFile = ") )
						ui->lineEditVerifyInputFile->setText(line.remove("VerifyInputFile = "));
					if ( line.startsWith("VerifyParentInputFile = ") )
						ui->lineEditVerifyParentInputFile->setText(line.remove("VerifyParentInputFile = "));
					break;
				case QCHDMAN_PRJ_COPY:
					if ( line.startsWith("CopyInputFile = ") )
						ui->lineEditCopyInputFile->setText(line.remove("CopyInputFile = "));
					if ( line.startsWith("CopyOutputFile = ") )
						ui->lineEditCopyOutputFile->setText(line.remove("CopyOutputFile = "));
					if ( line.startsWith("CopyParentInputFile = ") )
						ui->lineEditCopyParentInputFile->setText(line.remove("CopyParentInputFile = "));
					if ( line.startsWith("CopyParentOutputFile = ") )
						ui->lineEditCopyParentOutputFile->setText(line.remove("CopyParentOutputFile = "));
					if ( line.startsWith("CopyForce = ") )
						ui->checkBoxCopyForce->setChecked(line.remove("CopyForce = ").toInt());
					if ( line.startsWith("CopyProcessors = ") )
						ui->spinBoxCopyProcessors->setValue(line.remove("CopyProcessors = ").toInt());
					if ( line.startsWith("CopyInputStartByte = ") )
						ui->spinBoxCopyInputStartByte->setValue(line.remove("CopyInputStartByte = ").toInt());
					if ( line.startsWith("CopyInputStartHunk = ") )
						ui->spinBoxCopyInputStartHunk->setValue(line.remove("CopyInputStartHunk = ").toInt());
					if ( line.startsWith("CopyInputBytes = ") )
						ui->spinBoxCopyInputBytes->setValue(line.remove("CopyInputBytes = ").toInt());
					if ( line.startsWith("CopyInputHunks = ") )
						ui->spinBoxCopyInputHunks->setValue(line.remove("CopyInputHunks = ").toInt());
					if ( line.startsWith("CopyHunkSize = ") )
						ui->spinBoxCopyHunkSize->setValue(line.remove("CopyHunkSize = ").toInt());
					if ( line.startsWith("CopyCompression = ") ) {
						QString compression = line.remove("CopyCompression = ");
						if ( compression == "none" )
							ui->comboBoxCopyCompression->setCurrentIndex(3);
						else if ( compression != "default" ) {
							copyCompressors.clear();
							foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
								if ( mainWindow->compressionTypes.contains(cmp) ) {
									int index = ui->comboBoxCopyCompression->findText(cmp + " ", Qt::MatchStartsWith);
									if ( index > 4 ) {
										ui->comboBoxCopyCompression->setItemIcon(index, QIcon(":/images/active.png"));
										ui->comboBoxCopyCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
										copyCompressors << cmp;
									}
								}
							}
							on_comboBoxCopyCompression_currentIndexChanged(-1);
						}
					}
					break;
				case QCHDMAN_PRJ_CREATE_RAW:
					if ( line.startsWith("CreateRawInputFile = ") )
						ui->lineEditCreateRawInputFile->setText(line.remove("CreateRawInputFile = "));
					if ( line.startsWith("CreateRawOutputFile = ") )
						ui->lineEditCreateRawOutputFile->setText(line.remove("CreateRawOutputFile = "));
					if ( line.startsWith("CreateRawParentOutputFile = ") )
						ui->lineEditCreateRawParentOutputFile->setText(line.remove("CreateRawParentOutputFile = "));
					if ( line.startsWith("CreateRawForce = ") )
						ui->checkBoxCreateRawForce->setChecked(line.remove("CreateRawForce = ").toInt());
					if ( line.startsWith("CreateRawProcessors = ") )
						ui->spinBoxCreateRawProcessors->setValue(line.remove("CreateRawProcessors = ").toInt());
					if ( line.startsWith("CreateRawInputStartByte = ") )
						ui->spinBoxCreateRawInputStartByte->setValue(line.remove("CreateRawInputStartByte = ").toInt());
					if ( line.startsWith("CreateRawInputStartHunk = ") )
						ui->spinBoxCreateRawInputStartHunk->setValue(line.remove("CreateRawInputStartHunk = ").toInt());
					if ( line.startsWith("CreateRawInputBytes = ") )
						ui->spinBoxCreateRawInputBytes->setValue(line.remove("CreateRawInputBytes = ").toInt());
					if ( line.startsWith("CreateRawInputHunks = ") )
						ui->spinBoxCreateRawInputHunks->setValue(line.remove("CreateRawInputHunks = ").toInt());
					if ( line.startsWith("CreateRawHunkSize = ") )
						ui->spinBoxCreateRawHunkSize->setValue(line.remove("CreateRawHunkSize = ").toInt());
					if ( line.startsWith("CreateRawUnitSize = ") )
						ui->spinBoxCreateRawUnitSize->setValue(line.remove("CreateRawUnitSize = ").toInt());
					if ( line.startsWith("CreateRawCompression = ") ) {
						QString compression = line.remove("CreateRawCompression = ");
						if ( compression == "none" )
							ui->comboBoxCreateRawCompression->setCurrentIndex(3);
						else if ( compression != "default" ) {
							createRawCompressors.clear();
							foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
								if ( mainWindow->compressionTypes.contains(cmp) ) {
									int index = ui->comboBoxCreateRawCompression->findText(cmp + " ", Qt::MatchStartsWith);
									if ( index > 4 ) {
										ui->comboBoxCreateRawCompression->setItemIcon(index, QIcon(":/images/active.png"));
										ui->comboBoxCreateRawCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
										createRawCompressors << cmp;
									}
								}
							}
							on_comboBoxCreateRawCompression_currentIndexChanged(-1);
						}
					}
					break;
				case QCHDMAN_PRJ_CREATE_HD:
					if ( line.startsWith("CreateHDInputFile = ") )
						ui->lineEditCreateHDInputFile->setText(line.remove("CreateHDInputFile = "));
					if ( line.startsWith("CreateHDOutputFile = ") )
						ui->lineEditCreateHDOutputFile->setText(line.remove("CreateHDOutputFile = "));
					if ( line.startsWith("CreateHDParentOutputFile = ") )
						ui->lineEditCreateHDParentOutputFile->setText(line.remove("CreateHDParentOutputFile = "));
					if ( line.startsWith("CreateHDForce = ") )
						ui->checkBoxCreateHDForce->setChecked(line.remove("CreateHDForce = ").toInt());
					if ( line.startsWith("CreateHDProcessors = ") )
						ui->spinBoxCreateHDProcessors->setValue(line.remove("CreateHDProcessors = ").toInt());
					if ( line.startsWith("CreateHDInputStartByte = ") )
						ui->spinBoxCreateHDInputStartByte->setValue(line.remove("CreateHDInputStartByte = ").toInt());
					if ( line.startsWith("CreateHDInputStartHunk = ") )
						ui->spinBoxCreateHDInputStartHunk->setValue(line.remove("CreateHDInputStartHunk = ").toInt());
					if ( line.startsWith("CreateHDInputBytes = ") )
						ui->spinBoxCreateHDInputBytes->setValue(line.remove("CreateHDInputBytes = ").toInt());
					if ( line.startsWith("CreateHDInputHunks = ") )
						ui->spinBoxCreateHDInputHunks->setValue(line.remove("CreateHDInputHunks = ").toInt());
					if ( line.startsWith("CreateHDHunkSize = ") )
						ui->spinBoxCreateHDHunkSize->setValue(line.remove("CreateHDHunkSize = ").toInt());
					if ( line.startsWith("CreateHDCompression = ") ) {
						QString compression = line.remove("CreateHDCompression = ");
						if ( compression == "none" )
							ui->comboBoxCreateHDCompression->setCurrentIndex(3);
						else if ( compression != "default" ) {
							createHDCompressors.clear();
							foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
								if ( mainWindow->compressionTypes.contains(cmp) ) {
									int index = ui->comboBoxCreateHDCompression->findText(cmp + " ", Qt::MatchStartsWith);
									if ( index > 4 ) {
										ui->comboBoxCreateHDCompression->setItemIcon(index, QIcon(":/images/active.png"));
										ui->comboBoxCreateHDCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
										createHDCompressors << cmp;
									}
								}
							}
							on_comboBoxCreateHDCompression_currentIndexChanged(-1);
						}
					}
					if ( line.startsWith("CreateHDIdentFile = ") )
						ui->lineEditCreateHDIdentFile->setText(line.remove("CreateHDIdentFile = "));
					if ( line.startsWith("CreateHDSectorSize = ") )
						ui->spinBoxCreateHDSectorSize->setValue(line.remove("CreateHDSectorSize = ").toInt());
					if ( line.startsWith("CreateHDCylinders = ") )
						ui->spinBoxCreateHDCylinders->setValue(line.remove("CreateHDCylinders = ").toInt());
					if ( line.startsWith("CreateHDHeads = ") )
						ui->spinBoxCreateHDHeads->setValue(line.remove("CreateHDHeads = ").toInt());
					if ( line.startsWith("CreateHDSectors = ") )
						ui->spinBoxCreateHDSectors->setValue(line.remove("CreateHDSectors = ").toInt());
					break;
				case QCHDMAN_PRJ_CREATE_CD:
					if ( line.startsWith("CreateCDInputFile = ") )
						ui->lineEditCreateCDInputFile->setText(line.remove("CreateCDInputFile = "));
					if ( line.startsWith("CreateCDOutputFile = ") )
						ui->lineEditCreateCDOutputFile->setText(line.remove("CreateCDOutputFile = "));
					if ( line.startsWith("CreateCDParentOutputFile = ") )
						ui->lineEditCreateCDParentOutputFile->setText(line.remove("CreateCDParentOutputFile = "));
					if ( line.startsWith("CreateCDForce = ") )
						ui->checkBoxCreateCDForce->setChecked(line.remove("CreateCDForce = ").toInt());
					if ( line.startsWith("CreateCDProcessors = ") )
						ui->spinBoxCreateCDProcessors->setValue(line.remove("CreateCDProcessors = ").toInt());
					if ( line.startsWith("CreateCDHunkSize = ") )
						ui->spinBoxCreateCDHunkSize->setValue(line.remove("CreateCDHunkSize = ").toInt());
					if ( line.startsWith("CreateCDCompression = ") ) {
						QString compression = line.remove("CreateCDCompression = ");
						if ( compression == "none" )
							ui->comboBoxCreateCDCompression->setCurrentIndex(3);
						else if ( compression != "default" ) {
							createCDCompressors.clear();
							foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
								if ( mainWindow->compressionTypes.contains(cmp) ) {
									int index = ui->comboBoxCreateCDCompression->findText(cmp + " ", Qt::MatchStartsWith);
									if ( index > 4 ) {
										ui->comboBoxCreateCDCompression->setItemIcon(index, QIcon(":/images/active.png"));
										ui->comboBoxCreateCDCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
										createCDCompressors << cmp;
									}
								}
							}
							on_comboBoxCreateCDCompression_currentIndexChanged(-1);
						}
					}
					break;
				case QCHDMAN_PRJ_CREATE_LD:
					if ( line.startsWith("CreateLDInputFile = ") )
						ui->lineEditCreateLDInputFile->setText(line.remove("CreateLDInputFile = "));
					if ( line.startsWith("CreateLDOutputFile = ") )
						ui->lineEditCreateLDOutputFile->setText(line.remove("CreateLDOutputFile = "));
					if ( line.startsWith("CreateLDParentOutputFile = ") )
						ui->lineEditCreateLDParentOutputFile->setText(line.remove("CreateLDParentOutputFile = "));
					if ( line.startsWith("CreateLDForce = ") )
						ui->checkBoxCreateLDForce->setChecked(line.remove("CreateLDForce = ").toInt());
					if ( line.startsWith("CreateLDProcessors = ") )
						ui->spinBoxCreateLDProcessors->setValue(line.remove("CreateLDProcessors = ").toInt());
					if ( line.startsWith("CreateLDInputStartFrame = ") )
						ui->spinBoxCreateLDInputStartFrame->setValue(line.remove("CreateLDInputStartFrame = ").toInt());
					if ( line.startsWith("CreateLDInputFrames = ") )
						ui->spinBoxCreateLDInputFrames->setValue(line.remove("CreateLDInputFrames = ").toInt());
					if ( line.startsWith("CreateLDHunkSize = ") )
						ui->spinBoxCreateLDHunkSize->setValue(line.remove("CreateLDHunkSize = ").toInt());
					if ( line.startsWith("CreateLDCompression = ") ) {
						QString compression = line.remove("CreateLDCompression = ");
						if ( compression == "none" )
							ui->comboBoxCreateLDCompression->setCurrentIndex(3);
						else if ( compression != "default" ) {
							createLDCompressors.clear();
							foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
								if ( mainWindow->compressionTypes.contains(cmp) ) {
									int index = ui->comboBoxCreateLDCompression->findText(cmp + " ", Qt::MatchStartsWith);
									if ( index > 4 ) {
										ui->comboBoxCreateLDCompression->setItemIcon(index, QIcon(":/images/active.png"));
										ui->comboBoxCreateLDCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
										createLDCompressors << cmp;
									}
								}
							}
							on_comboBoxCreateLDCompression_currentIndexChanged(-1);
						}
					}
					break;
				case QCHDMAN_PRJ_EXTRACT_RAW:
					if ( line.startsWith("ExtractRawInputFile = ") )
						ui->lineEditExtractRawInputFile->setText(line.remove("ExtractRawInputFile = "));
					if ( line.startsWith("ExtractRawOutputFile = ") )
						ui->lineEditExtractRawOutputFile->setText(line.remove("ExtractRawOutputFile = "));
					if ( line.startsWith("ExtractRawParentInputFile = ") )
						ui->lineEditExtractRawParentInputFile->setText(line.remove("ExtractRawParentInputFile = "));
					if ( line.startsWith("ExtractRawForce = ") )
						ui->checkBoxExtractRawForce->setChecked(line.remove("ExtractRawForce = ").toInt());
					if ( line.startsWith("ExtractRawInputStartByte = ") )
						ui->spinBoxExtractRawInputStartByte->setValue(line.remove("ExtractRawInputStartByte = ").toInt());
					if ( line.startsWith("ExtractRawInputStartHunk = ") )
						ui->spinBoxExtractRawInputStartHunk->setValue(line.remove("ExtractRawInputStartHunk = ").toInt());
					if ( line.startsWith("ExtractRawInputBytes = ") )
						ui->spinBoxExtractRawInputBytes->setValue(line.remove("ExtractRawInputBytes = ").toInt());
					if ( line.startsWith("ExtractRawInputHunks = ") )
						ui->spinBoxExtractRawInputHunks->setValue(line.remove("ExtractRawInputHunks = ").toInt());
					break;
				case QCHDMAN_PRJ_EXTRACT_HD:
					if ( line.startsWith("ExtractHDInputFile = ") )
						ui->lineEditExtractHDInputFile->setText(line.remove("ExtractHDInputFile = "));
					if ( line.startsWith("ExtractHDOutputFile = ") )
						ui->lineEditExtractHDOutputFile->setText(line.remove("ExtractHDOutputFile = "));
					if ( line.startsWith("ExtractHDParentInputFile = ") )
						ui->lineEditExtractHDParentInputFile->setText(line.remove("ExtractHDParentInputFile = "));
					if ( line.startsWith("ExtractHDForce = ") )
						ui->checkBoxExtractHDForce->setChecked(line.remove("ExtractHDForce = ").toInt());
					if ( line.startsWith("ExtractHDInputStartByte = ") )
						ui->spinBoxExtractHDInputStartByte->setValue(line.remove("ExtractHDInputStartByte = ").toInt());
					if ( line.startsWith("ExtractHDInputStartHunk = ") )
						ui->spinBoxExtractHDInputStartHunk->setValue(line.remove("ExtractHDInputStartHunk = ").toInt());
					if ( line.startsWith("ExtractHDInputBytes = ") )
						ui->spinBoxExtractHDInputBytes->setValue(line.remove("ExtractHDInputBytes = ").toInt());
					if ( line.startsWith("ExtractHDInputHunks = ") )
						ui->spinBoxExtractHDInputHunks->setValue(line.remove("ExtractHDInputHunks = ").toInt());
					break;
				case QCHDMAN_PRJ_EXTRACT_CD:
					if ( line.startsWith("ExtractCDInputFile = ") )
						ui->lineEditExtractCDInputFile->setText(line.remove("ExtractCDInputFile = "));
					if ( line.startsWith("ExtractCDOutputFile = ") )
						ui->lineEditExtractCDOutputFile->setText(line.remove("ExtractCDOutputFile = "));
					if ( line.startsWith("ExtractCDParentInputFile = ") )
						ui->lineEditExtractCDParentInputFile->setText(line.remove("ExtractCDParentInputFile = "));
					if ( line.startsWith("ExtractCDOutputBinFile = ") )
						ui->lineEditExtractCDOutputBinFile->setText(line.remove("ExtractCDOutputBinFile = "));
					if ( line.startsWith("ExtractCDForce = ") )
						ui->checkBoxExtractCDForce->setChecked(line.remove("ExtractCDForce = ").toInt());
					break;
				case QCHDMAN_PRJ_EXTRACT_LD:
					if ( line.startsWith("ExtractLDInputFile = ") )
						ui->lineEditExtractLDInputFile->setText(line.remove("ExtractLDInputFile = "));
					if ( line.startsWith("ExtractLDOutputFile = ") )
						ui->lineEditExtractLDOutputFile->setText(line.remove("ExtractLDOutputFile = "));
					if ( line.startsWith("ExtractLDParentOutputFile = ") )
						ui->lineEditExtractLDParentInputFile->setText(line.remove("ExtractLDParentInputFile = "));
					if ( line.startsWith("ExtractLDForce = ") )
						ui->checkBoxExtractLDForce->setChecked(line.remove("ExtractLDForce = ").toInt());
					if ( line.startsWith("ExtractLDInputStartFrame = ") )
						ui->spinBoxExtractLDInputStartFrame->setValue(line.remove("ExtractLDInputStartFrame = ").toInt());
					if ( line.startsWith("ExtractLDInputFrames = ") )
						ui->spinBoxExtractLDInputFrames->setValue(line.remove("ExtractLDInputFrames = ").toInt());
					break;
				case QCHDMAN_PRJ_DUMP_META:
					if ( line.startsWith("DumpMetaInputFile = ") )
						ui->lineEditDumpMetaInputFile->setText(line.remove("DumpMetaInputFile = "));
					if ( line.startsWith("DumpMetaOutputFile = ") )
						ui->lineEditDumpMetaOutputFile->setText(line.remove("DumpMetaOutputFile = "));
					if ( line.startsWith("DumpMetaForce = ") )
						ui->checkBoxDumpMetaForce->setChecked(line.remove("DumpMetaForce = ").toInt());
					if ( line.startsWith("DumpMetaTag = ") )
						ui->lineEditDumpMetaTag->setText(line.remove("DumpMetaTag = "));
					if ( line.startsWith("DumpMetaIndex = ") )
						ui->spinBoxDumpMetaIndex->setValue(line.remove("DumpMetaIndex = ").toInt());
					break;
				case QCHDMAN_PRJ_ADD_META:
					if ( line.startsWith("AddMetaInputFile = ") )
						ui->lineEditAddMetaInputFile->setText(line.remove("AddMetaInputFile = "));
					if ( line.startsWith("AddMetaValueFile = ") )
						ui->lineEditAddMetaValueFile->setText(line.remove("AddMetaValueFile = "));
					if ( line.startsWith("AddMetaValueText = ") )
						ui->lineEditAddMetaValueText->setText(line.remove("AddMetaValueText = "));
					if ( line.startsWith("AddMetaTag = ") )
						ui->lineEditAddMetaTag->setText(line.remove("AddMetaTag = "));
					if ( line.startsWith("AddMetaIndex = ") )
						ui->spinBoxAddMetaIndex->setValue(line.remove("AddMetaIndex = ").toInt());
					if ( line.startsWith("AddMetaNoCheckSum = ") )
						ui->checkBoxAddMetaNoCheckSum->setChecked(line.remove("AddMetaNoCheckSum = ").toInt());
					break;
				case QCHDMAN_PRJ_DEL_META:
					if ( line.startsWith("DelMetaInputFile = ") )
						ui->lineEditDelMetaInputFile->setText(line.remove("DelMetaInputFile = "));
					if ( line.startsWith("DelMetaTag = ") )
						ui->lineEditDelMetaTag->setText(line.remove("DelMetaTag = "));
					if ( line.startsWith("DelMetaIndex = ") )
						ui->spinBoxDelMetaIndex->setValue(line.remove("DelMetaIndex = ").toInt());
					break;
				}
			}
			ui->comboBoxProjectType->setCurrentIndex(projectType);
			qApp->processEvents();
			on_comboBoxProjectType_currentIndexChanged(projectType);
		}

		if ( buffer != NULL )
			baBuffer.close();
		else
			loadFile.close();

		if ( !isScriptElement && !projectName.isEmpty() ) {
			((ProjectWindow *)parentWidget())->projectName = projectName;
			parentWidget()->setWindowTitle(projectName);
			mainWindow->addRecentFile(projectName);
		}
	} else if ( !projectName.isEmpty() )
		mainWindow->statusBar()->showMessage(tr("Failed loading project '%1'").arg(projectName), QCHDMAN_STATUS_MSGTIME);
}

void ProjectWidget::save(QString *buffer)
{
	QString projectName;

	if ( !isScriptElement )
		projectName = ((ProjectWindow *)parentWidget())->projectName;

	if ( projectName.startsWith(tr("Noname-%1").arg("")) )
		askFileName = true;

	saveAs(projectName, buffer);

	askFileName = false;
}

void ProjectWidget::saveAs(const QString &fileName, QString *buffer)
{
	QString projectName = fileName;

	if ( buffer == NULL && (projectName.isEmpty() || askFileName) ) {
		if ( !isScriptElement )
			projectName = ((ProjectWindow *)parentWidget())->projectName;
		if ( projectName.startsWith(tr("Noname-%1").arg("")) || projectName.isEmpty() || askFileName ) {
			QString s = QFileDialog::getSaveFileName(this, tr("Choose project file"), projectName, tr("Project files (*.prj)") + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
			if ( s.isNull() )
				return;
			else
				projectName = s;
		}
	}

	QFile saveFile(projectName);
	QTextStream ts;
	QByteArray ba;
	QBuffer baBuffer(&ba);

	if ( buffer != NULL ) {
		baBuffer.open(QBuffer::WriteOnly);
		ts.setDevice(&baBuffer);
	} else if ( saveFile.open(QIODevice::WriteOnly | QIODevice::Text) )
		ts.setDevice(&saveFile);

	if ( ts.device() ) {
		int projectType = ui->comboBoxProjectType->currentIndex();
		ts << "# " << tr("Qt CHDMAN GUI project file -- please do not edit manually") << "\n";
		ts << "ApplicationVersion = " << QCHDMAN_APP_VERSION << "\n";
		ts << "ProjectFormatVersion = " << QCHDMAN_PRJ_FMT_VERSION << "\n";
		ts << "ProjectType = " << mainWindow->projectTypes[projectType] << "\n";
		switch ( projectType ) {
		case QCHDMAN_PRJ_INFO:
			ts << "InfoInputFile = " << ui->lineEditInfoInputFile->text() << "\n";
			ts << "InfoVerbose = " << ui->checkBoxInfoVerbose->isChecked() << "\n";
			break;
		case QCHDMAN_PRJ_VERIFY:
			ts << "VerifyInputFile = " << ui->lineEditVerifyInputFile->text() << "\n";
			ts << "VerifyParentInputFile = " << ui->lineEditVerifyParentInputFile->text() << "\n";
			break;
		case QCHDMAN_PRJ_COPY:
			ts << "CopyInputFile = " << ui->lineEditCopyInputFile->text() << "\n";
			ts << "CopyOutputFile = " << ui->lineEditCopyOutputFile->text() << "\n";
			ts << "CopyParentInputFile = " << ui->lineEditCopyParentInputFile->text() << "\n";
			ts << "CopyParentOutputFile = " << ui->lineEditCopyParentOutputFile->text() << "\n";
			ts << "CopyForce = " << ui->checkBoxCopyForce->isChecked() << "\n";
			ts << "CopyProcessors = " << ui->spinBoxCopyProcessors->value() << "\n";
			ts << "CopyInputStartByte = " << ui->spinBoxCopyInputStartByte->value() << "\n";
			ts << "CopyInputStartHunk = " << ui->spinBoxCopyInputStartHunk->value() << "\n";
			ts << "CopyInputBytes = " << ui->spinBoxCopyInputBytes->value() << "\n";
			ts << "CopyInputHunks = " << ui->spinBoxCopyInputHunks->value() << "\n";
			ts << "CopyHunkSize = " << ui->spinBoxCopyHunkSize->value() << "\n";
			ts << "CopyCompression = ";
			if ( !copyCompressors.isEmpty() )
				ts << copyCompressors.join(",");
			else if ( ui->comboBoxCopyCompression->currentText() == tr("default") )
				ts << "default";
			else
				ts << "none";
			ts << "\n";
			break;
		case QCHDMAN_PRJ_CREATE_RAW:
			ts << "CreateRawInputFile = " << ui->lineEditCreateRawInputFile->text() << "\n";
			ts << "CreateRawOutputFile = " << ui->lineEditCreateRawOutputFile->text() << "\n";
			ts << "CreateRawParentOutputFile = " << ui->lineEditCreateRawParentOutputFile->text() << "\n";
			ts << "CreateRawForce = " << ui->checkBoxCreateRawForce->isChecked() << "\n";
			ts << "CreateRawProcessors = " << ui->spinBoxCreateRawProcessors->value() << "\n";
			ts << "CreateRawInputStartByte = " << ui->spinBoxCreateRawInputStartByte->value() << "\n";
			ts << "CreateRawInputStartHunk = " << ui->spinBoxCreateRawInputStartHunk->value() << "\n";
			ts << "CreateRawInputBytes = " << ui->spinBoxCreateRawInputBytes->value() << "\n";
			ts << "CreateRawInputHunks = " << ui->spinBoxCreateRawInputHunks->value() << "\n";
			ts << "CreateRawHunkSize = " << ui->spinBoxCreateRawHunkSize->value() << "\n";
			ts << "CreateRawUnitSize = " << ui->spinBoxCreateRawUnitSize->value() << "\n";
			ts << "CreateRawCompression = ";
			if ( !createRawCompressors.isEmpty() )
				ts << createRawCompressors.join(",");
			else if ( ui->comboBoxCreateRawCompression->currentText() == tr("default") )
				ts << "default";
			else
				ts << "none";
			ts << "\n";
			break;
		case QCHDMAN_PRJ_CREATE_HD:
			ts << "CreateHDInputFile = " << ui->lineEditCreateHDInputFile->text() << "\n";
			ts << "CreateHDOutputFile = " << ui->lineEditCreateHDOutputFile->text() << "\n";
			ts << "CreateHDParentOutputFile = " << ui->lineEditCreateHDParentOutputFile->text() << "\n";
			ts << "CreateHDForce = " << ui->checkBoxCreateHDForce->isChecked() << "\n";
			ts << "CreateHDProcessors = " << ui->spinBoxCreateHDProcessors->value() << "\n";
			ts << "CreateHDInputStartByte = " << ui->spinBoxCreateHDInputStartByte->value() << "\n";
			ts << "CreateHDInputStartHunk = " << ui->spinBoxCreateHDInputStartHunk->value() << "\n";
			ts << "CreateHDInputBytes = " << ui->spinBoxCreateHDInputBytes->value() << "\n";
			ts << "CreateHDInputHunks = " << ui->spinBoxCreateHDInputHunks->value() << "\n";
			ts << "CreateHDHunkSize = " << ui->spinBoxCreateHDHunkSize->value() << "\n";
			ts << "CreateHDCompression = ";
			if ( !createHDCompressors.isEmpty() )
				ts << createHDCompressors.join(",");
			else if ( ui->comboBoxCreateHDCompression->currentText() == tr("default") )
				ts << "default";
			else
				ts << "none";
			ts << "\n";
			ts << "CreateHDIdentFile = " << ui->lineEditCreateHDIdentFile->text() << "\n";
			ts << "CreateHDSectorSize = " << ui->spinBoxCreateHDSectorSize->value() << "\n";
			ts << "CreateHDCylinders = " << ui->spinBoxCreateHDCylinders->value() << "\n";
			ts << "CreateHDHeads = " << ui->spinBoxCreateHDHeads->value() << "\n";
			ts << "CreateHDSectors = " << ui->spinBoxCreateHDSectors->value() << "\n";
			break;
		case QCHDMAN_PRJ_CREATE_CD:
			ts << "CreateCDInputFile = " << ui->lineEditCreateCDInputFile->text() << "\n";
			ts << "CreateCDOutputFile = " << ui->lineEditCreateCDOutputFile->text() << "\n";
			ts << "CreateCDParentOutputFile = " << ui->lineEditCreateCDParentOutputFile->text() << "\n";
			ts << "CreateCDForce = " << ui->checkBoxCreateCDForce->isChecked() << "\n";
			ts << "CreateCDProcessors = " << ui->spinBoxCreateCDProcessors->value() << "\n";
			ts << "CreateCDHunkSize = " << ui->spinBoxCreateCDHunkSize->value() << "\n";
			ts << "CreateCDCompression = ";
			if ( !createCDCompressors.isEmpty() )
				ts << createCDCompressors.join(",");
			else if ( ui->comboBoxCreateCDCompression->currentText() == tr("default") )
				ts << "default";
			else
				ts << "none";
			ts << "\n";
			break;
		case QCHDMAN_PRJ_CREATE_LD:
			ts << "CreateLDInputFile = " << ui->lineEditCreateLDInputFile->text() << "\n";
			ts << "CreateLDOutputFile = " << ui->lineEditCreateLDOutputFile->text() << "\n";
			ts << "CreateLDParentOutputFile = " << ui->lineEditCreateLDParentOutputFile->text() << "\n";
			ts << "CreateLDForce = " << ui->checkBoxCreateLDForce->isChecked() << "\n";
			ts << "CreateLDProcessors = " << ui->spinBoxCreateLDProcessors->value() << "\n";
			ts << "CreateLDInputStartFrame = " << ui->spinBoxCreateLDInputStartFrame->value() << "\n";
			ts << "CreateLDInputFrames = " << ui->spinBoxCreateLDInputFrames->value() << "\n";
			ts << "CreateLDHunkSize = " << ui->spinBoxCreateLDHunkSize->value() << "\n";
			ts << "CreateLDCompression = ";
			if ( !createLDCompressors.isEmpty() )
				ts << createLDCompressors.join(",");
			else if ( ui->comboBoxCreateLDCompression->currentText() == tr("default") )
				ts << "default";
			else
				ts << "none";
			ts << "\n";
			break;
		case QCHDMAN_PRJ_EXTRACT_RAW:
			ts << "ExtractRawInputFile = " << ui->lineEditExtractRawInputFile->text() << "\n";
			ts << "ExtractRawOutputFile = " << ui->lineEditExtractRawOutputFile->text() << "\n";
			ts << "ExtractRawParentInputFile = " << ui->lineEditExtractRawParentInputFile->text() << "\n";
			ts << "ExtractRawForce = " << ui->checkBoxExtractRawForce->isChecked() << "\n";
			ts << "ExtractRawInputStartByte = " << ui->spinBoxExtractRawInputStartByte->value() << "\n";
			ts << "ExtractRawInputStartHunk = " << ui->spinBoxExtractRawInputStartHunk->value() << "\n";
			ts << "ExtractRawInputBytes = " << ui->spinBoxExtractRawInputBytes->value() << "\n";
			ts << "ExtractRawInputHunks = " << ui->spinBoxExtractRawInputHunks->value() << "\n";
			break;
		case QCHDMAN_PRJ_EXTRACT_HD:
			ts << "ExtractHDInputFile = " << ui->lineEditExtractHDInputFile->text() << "\n";
			ts << "ExtractHDOutputFile = " << ui->lineEditExtractHDOutputFile->text() << "\n";
			ts << "ExtractHDParentInputFile = " << ui->lineEditExtractHDParentInputFile->text() << "\n";
			ts << "ExtractHDForce = " << ui->checkBoxExtractHDForce->isChecked() << "\n";
			ts << "ExtractHDInputStartByte = " << ui->spinBoxExtractHDInputStartByte->value() << "\n";
			ts << "ExtractHDInputStartHunk = " << ui->spinBoxExtractHDInputStartHunk->value() << "\n";
			ts << "ExtractHDInputBytes = " << ui->spinBoxExtractHDInputBytes->value() << "\n";
			ts << "ExtractHDInputHunks = " << ui->spinBoxExtractHDInputHunks->value() << "\n";
			break;
		case QCHDMAN_PRJ_EXTRACT_CD:
			ts << "ExtractCDInputFile = " << ui->lineEditExtractCDInputFile->text() << "\n";
			ts << "ExtractCDOutputFile = " << ui->lineEditExtractCDOutputFile->text() << "\n";
			ts << "ExtractCDParentInputFile = " << ui->lineEditExtractCDParentInputFile->text() << "\n";
			ts << "ExtractCDOutputBinFile = " << ui->lineEditExtractCDOutputBinFile->text() << "\n";
			ts << "ExtractCDForce = " << ui->checkBoxExtractCDForce->isChecked() << "\n";
			break;
		case QCHDMAN_PRJ_EXTRACT_LD:
			ts << "ExtractLDInputFile = " << ui->lineEditExtractLDInputFile->text() << "\n";
			ts << "ExtractLDOutputFile = " << ui->lineEditExtractLDOutputFile->text() << "\n";
			ts << "ExtractLDParentInputFile = " << ui->lineEditExtractLDParentInputFile->text() << "\n";
			ts << "ExtractLDForce = " << ui->checkBoxExtractLDForce->isChecked() << "\n";
			ts << "ExtractLDInputStartFrame = " << ui->spinBoxExtractLDInputStartFrame->value() << "\n";
			ts << "ExtractLDInputFrames = " << ui->spinBoxExtractLDInputFrames->value() << "\n";
			break;
		case QCHDMAN_PRJ_DUMP_META:
			ts << "DumpMetaInputFile = " << ui->lineEditDumpMetaInputFile->text() << "\n";
			ts << "DumpMetaOutputFile = " << ui->lineEditDumpMetaOutputFile->text() << "\n";
			ts << "DumpMetaForce = " << ui->checkBoxDumpMetaForce->isChecked() << "\n";
			ts << "DumpMetaTag = " << ui->lineEditDumpMetaTag->text() << "\n";
			ts << "DumpMetaIndex = " << ui->spinBoxDumpMetaIndex->value() << "\n";
			break;
		case QCHDMAN_PRJ_ADD_META:
			ts << "AddMetaInputFile = " << ui->lineEditAddMetaInputFile->text() << "\n";
			ts << "AddMetaValueFile = " << ui->lineEditAddMetaValueFile->text() << "\n";
			ts << "AddMetaValueText = " << ui->lineEditAddMetaValueText->text() << "\n";
			ts << "AddMetaTag = " << ui->lineEditAddMetaTag->text() << "\n";
			ts << "AddMetaIndex = " << ui->spinBoxAddMetaIndex->value() << "\n";
			ts << "AddMetaNoCheckSum = " << ui->checkBoxAddMetaNoCheckSum->isChecked() << "\n";
			break;
		case QCHDMAN_PRJ_DEL_META:
			ts << "DelMetaInputFile = " << ui->lineEditDelMetaInputFile->text() << "\n";
			ts << "DelMetaTag = " << ui->lineEditDelMetaTag->text() << "\n";
			ts << "DelMetaIndex = " << ui->spinBoxDelMetaIndex->value() << "\n";
			break;
		}

		if ( buffer != NULL ) {
			baBuffer.close();
			*buffer = QString(ba);
		} else
			saveFile.close();

		if ( !isScriptElement ) {
			((ProjectWindow *)parentWidget())->projectName = projectName;
			parentWidget()->setWindowTitle(projectName);
			mainWindow->addRecentFile(projectName);
		}

		if ( !projectName.isEmpty() )
			mainWindow->statusBar()->showMessage(tr("Project '%1' saved").arg(projectName), QCHDMAN_STATUS_MSGTIME);
	} else if ( !projectName.isEmpty() )
		mainWindow->statusBar()->showMessage(tr("Failed saving project '%1'").arg(projectName), QCHDMAN_STATUS_MSGTIME);
}

QString ProjectWidget::toString()
{
	QString buffer;
	save(&buffer);
	return buffer;
}

void ProjectWidget::fromString(QString buffer)
{
	load(QString(), &buffer);
}

void ProjectWidget::triggerSaveAs()
{
	askFileName = true;
	saveAs();
	askFileName = false;
}

void ProjectWidget::clone()
{
	QAction *action = (QAction *)sender();

	on_toolButtonRun_clicked(true);

	int cloneType = cloneActionMap[action];

	if ( copyGroups.contains(cloneType) ) {
		ProjectWindow *projectWindow = mainWindow->createProjectWindow(QCHDMAN_MDI_PROJECT);
		ProjectWidget *projectWidget = projectWindow->projectWidget;
		projectWidget->setProjectType(cloneType);
		QList<QWidget *> sourceWidgets = copyGroups[ui->comboBoxProjectType->currentIndex()];
		QList<int> sourceTypes = mainWindow->copyTypes[ui->comboBoxProjectType->currentIndex()];
		QList<QWidget *> targetWidgets = projectWidget->copyGroups[cloneType];
		QList<int> targetTypes = mainWindow->copyTypes[cloneType];
		for (int i = 0; i < sourceWidgets.count() && i < targetWidgets.count(); i++) {
			QWidget *sW = sourceWidgets[i];
			int sT = sourceTypes[i];
			QWidget *tW = targetWidgets[i];
			int tT = targetTypes[i];
			if ( sT != tT || !sW || !tW )
				continue;
			switch ( sT ) {
			case QCHDMAN_TYPE_LINEEDIT:
				((QLineEdit *)tW)->setText(((QLineEdit *)sW)->text());
				break;
			case QCHDMAN_TYPE_SPINBOX:
				((QSpinBox *)tW)->setValue(((QSpinBox *)sW)->value());
				break;
			case QCHDMAN_TYPE_CHECKBOX:
				((QCheckBox *)tW)->setChecked(((QCheckBox *)sW)->isChecked());
				break;
			case QCHDMAN_TYPE_COMBOBOX: {
				QString compression = ((QComboBox *)sW)->currentText();
				QComboBox *cb = (QComboBox *)tW;
				if ( compression == tr("none") )
					cb->setCurrentIndex(3);
				else if ( compression != tr("default") ) {
					switch ( cloneType ) {
					case QCHDMAN_PRJ_COPY:
						projectWidget->copyCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_RAW:
						projectWidget->createRawCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_HD:
						projectWidget->createHDCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_CD:
						projectWidget->createCDCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_LD:
						projectWidget->createLDCompressors.clear();
						break;
					}
					foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
						if ( mainWindow->compressionTypes.contains(cmp) ) {
							int index = cb->findText(cmp + " ", Qt::MatchStartsWith);
							if ( index > 4 ) {
								cb->setItemIcon(index, QIcon(":/images/active.png"));
								cb->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
								switch ( cloneType ) {
								case QCHDMAN_PRJ_COPY:
									projectWidget->copyCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_RAW:
									projectWidget->createRawCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_HD:
									projectWidget->createHDCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_CD:
									projectWidget->createCDCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_LD:
									projectWidget->createLDCompressors << cmp;
									break;
								}
							}
						}
					}
					switch ( cloneType ) {
					case QCHDMAN_PRJ_COPY:
						projectWidget->on_comboBoxCopyCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_RAW:
						projectWidget->on_comboBoxCreateRawCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_HD:
						projectWidget->on_comboBoxCreateHDCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_CD:
						projectWidget->on_comboBoxCreateCDCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_LD:
						projectWidget->on_comboBoxCreateLDCompression_currentIndexChanged(-1);
						break;
					}
				}
			}
				break;
			}
		}
	} else
		log(tr("cloning to '%1' is not supported yet").arg(action->text()));
}

void ProjectWidget::morph()
{
	QAction *action = (QAction *)sender();

	on_toolButtonRun_clicked(true);

	int morphType = morphActionMap[action];

	if ( copyGroups.contains(morphType) ) {
		QList<QWidget *> sourceWidgets = copyGroups[ui->comboBoxProjectType->currentIndex()];
		QList<int> sourceTypes = mainWindow->copyTypes[ui->comboBoxProjectType->currentIndex()];
		QList<QWidget *> targetWidgets = copyGroups[morphType];
		QList<int> targetTypes = mainWindow->copyTypes[morphType];
		setProjectType(morphType);
		for (int i = 0; i < sourceWidgets.count() && i < targetWidgets.count(); i++) {
			QWidget *sW = sourceWidgets[i];
			int sT = sourceTypes[i];
			QWidget *tW = targetWidgets[i];
			int tT = targetTypes[i];
			if ( sT != tT || !sW || !tW )
				continue;
			switch ( sT ) {
			case QCHDMAN_TYPE_LINEEDIT:
				((QLineEdit *)tW)->setText(((QLineEdit *)sW)->text());
				break;
			case QCHDMAN_TYPE_SPINBOX:
				((QSpinBox *)tW)->setValue(((QSpinBox *)sW)->value());
				break;
			case QCHDMAN_TYPE_CHECKBOX:
				((QCheckBox *)tW)->setChecked(((QCheckBox *)sW)->isChecked());
				break;
			case QCHDMAN_TYPE_COMBOBOX: {
				QString compression = ((QComboBox *)sW)->currentText();
				QComboBox *cb = (QComboBox *)tW;
				if ( compression == tr("none") )
					cb->setCurrentIndex(3);
				else if ( compression != tr("default") ) {
					switch ( morphType ) {
					case QCHDMAN_PRJ_COPY:
						copyCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_RAW:
						createRawCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_HD:
						createHDCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_CD:
						createCDCompressors.clear();
						break;
					case QCHDMAN_PRJ_CREATE_LD:
						createLDCompressors.clear();
						break;
					}
					foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
						if ( mainWindow->compressionTypes.contains(cmp) ) {
							int index = cb->findText(cmp + " ", Qt::MatchStartsWith);
							if ( index > 4 ) {
								cb->setItemIcon(index, QIcon(":/images/active.png"));
								cb->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
								switch ( morphType ) {
								case QCHDMAN_PRJ_COPY:
									copyCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_RAW:
									createRawCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_HD:
									createHDCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_CD:
									createCDCompressors << cmp;
									break;
								case QCHDMAN_PRJ_CREATE_LD:
									createLDCompressors << cmp;
									break;
								}
							}
						}
					}
					switch ( morphType ) {
					case QCHDMAN_PRJ_COPY:
						on_comboBoxCopyCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_RAW:
						on_comboBoxCreateRawCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_HD:
						on_comboBoxCreateHDCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_CD:
						on_comboBoxCreateCDCompression_currentIndexChanged(-1);
						break;
					case QCHDMAN_PRJ_CREATE_LD:
						on_comboBoxCreateLDCompression_currentIndexChanged(-1);
						break;
					}
				}
			}
				break;
			}
		}
	} else
		log(tr("morphing to '%1' is not supported yet").arg(action->text()));
}

void ProjectWidget::run()
{
	if ( ui->toolButtonRun->isEnabled() )
		ui->toolButtonRun->animateClick();
}

void ProjectWidget::stop()
{
	if ( ui->toolButtonStop->isEnabled() )
		ui->toolButtonStop->animateClick();
}

void ProjectWidget::signalProgressUpdate(int value)
{
	emit progressValueChanged(this, value);
}
