#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "projectwindow.h"
#include "projectwidget.h"
#include "ui_projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;
extern quint64 runningProjects;

ProjectWidget::ProjectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectWidget)
{
    ui->setupUi(this);

    chdmanProc = NULL;
    terminatedOnDemand = askFileName = false;
    needsTabbedUiAdjustment = needsWindowedUiAdjustment = true;

    // compression codecs
    compressionTypes["avhu"] = tr("avhu (A/V Huffman)");
    compressionTypes["cdfl"] = tr("cdfl (CD FLAC)");
    compressionTypes["cdlz"] = tr("cdlz (CD LZMA)");
    compressionTypes["cdzl"] = tr("cdzl (CD Deflate)");
    compressionTypes["flac"] = tr("flac (FLAC)");
    compressionTypes["huff"] = tr("huff (Huffman)");
    compressionTypes["lzma"] = tr("lzma (LZMA)");
    compressionTypes["zlib"] = tr("zlib (Deflate)");

    // prepare HD geometry template selector
    ui->comboBoxCreateHDFromTemplate->addItem(tr("Select"));
    ui->comboBoxCreateHDFromTemplate->insertSeparator(1);
    QMapIterator<QString, QList<DiskGeometry> > it(mainWindow->hardDiskTemplates);
    while ( it.hasNext() ) {
       it.next();
       foreach (DiskGeometry geo, it.value())
           ui->comboBoxCreateHDFromTemplate->addItem(it.key() + ": " + geo.name);
    }

    // prepare morph & clone functionality
    // FIXME: missing QCHDMAN_PRJ_CREATE_CD, QCHDMAN_PRJ_CREATE_LD, QCHDMAN_PRJ_EXTRACT_RAW, QCHDMAN_PRJ_EXTRACT_CD and QCHDMAN_PRJ_EXTRACT_LD
    copyGroups[QCHDMAN_PRJ_INFO]
            << ui->lineEditInfoInputFile << ui->checkBoxInfoVerbose;
    copyTypes[QCHDMAN_PRJ_INFO]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_CHECKBOX;
    copyGroups[QCHDMAN_PRJ_VERIFY]
            << ui->lineEditVerifyInputFile << ui->lineEditVerifyParentInputFile;
    copyTypes[QCHDMAN_PRJ_VERIFY]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT;
    copyGroups[QCHDMAN_PRJ_COPY]
            << ui->lineEditCopyInputFile << ui->lineEditCopyParentInputFile << ui->lineEditCopyOutputFile << ui->lineEditCopyParentOutputFile
            << ui->checkBoxCopyForce << ui->spinBoxCopyProcessors << ui->spinBoxCopyInputStartByte << ui->spinBoxCopyInputStartHunk
            << ui->spinBoxCopyInputBytes << ui->spinBoxCopyInputHunks << ui->spinBoxCopyHunkSize << ui->comboBoxCopyCompression;
    copyTypes[QCHDMAN_PRJ_COPY]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX;
    copyGroups[QCHDMAN_PRJ_CREATE_RAW]
            << ui->lineEditCreateRawInputFile << 0 << ui->lineEditCreateRawOutputFile << ui->lineEditCreateRawParentOutputFile
            << ui->checkBoxCreateRawForce << ui->spinBoxCreateRawProcessors << ui->spinBoxCreateRawInputStartByte << ui->spinBoxCreateRawInputStartHunk
            << ui->spinBoxCreateRawInputBytes << ui->spinBoxCreateRawInputHunks << ui->spinBoxCreateRawHunkSize << ui->comboBoxCreateRawCompression
            << ui->spinBoxCreateRawUnitSize;
    copyTypes[QCHDMAN_PRJ_CREATE_RAW]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX
            << QCHDMAN_TYPE_SPINBOX;
    copyGroups[QCHDMAN_PRJ_CREATE_HD]
            << ui->lineEditCreateHDInputFile << 0 << ui->lineEditCreateHDOutputFile << ui->lineEditCreateHDParentOutputFile
            << ui->checkBoxCreateHDForce << ui->spinBoxCreateHDProcessors << ui->spinBoxCreateHDInputStartByte << ui->spinBoxCreateHDInputStartHunk
            << ui->spinBoxCreateHDInputBytes << ui->spinBoxCreateHDInputHunks << ui->spinBoxCreateHDHunkSize << ui->comboBoxCreateHDCompression
            << 0 << ui->spinBoxCreateHDCylinders << ui->spinBoxCreateHDHeads << ui->spinBoxCreateHDSectors
            << ui->lineEditCreateHDIdentFile << ui->spinBoxCreateHDSectorSize;
    copyTypes[QCHDMAN_PRJ_CREATE_HD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_SPINBOX;
    copyGroups[QCHDMAN_PRJ_EXTRACT_HD]
            << ui->lineEditExtractHDInputFile << ui->lineEditExtractHDParentInputFile << ui->lineEditExtractHDOutputFile << 0
            << ui->checkBoxExtractHDForce << 0 << ui->spinBoxExtractHDInputStartByte << ui->spinBoxExtractHDInputStartHunk
            << ui->spinBoxExtractHDInputBytes << ui->spinBoxExtractHDInputHunks;
    copyTypes[QCHDMAN_PRJ_EXTRACT_HD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX;

    // prepare compression selectors
    copyCompressors.clear();
    createRawCompressors.clear();
    createHDCompressors.clear();
    QList<QComboBox *> compressionBoxes;
    compressionBoxes << ui->comboBoxCopyCompression << ui->comboBoxCreateRawCompression << ui->comboBoxCreateHDCompression;
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
        foreach (QString cmp, compressionTypes) {
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
    cloneActionMap[menuCloneActions->addAction(tr("Info"), this, SLOT(clone()))] = QCHDMAN_PRJ_INFO;
    cloneActionMap[menuCloneActions->addAction(tr("Verify"), this, SLOT(clone()))] = QCHDMAN_PRJ_VERIFY;
    cloneActionMap[menuCloneActions->addAction(tr("Copy"), this, SLOT(clone()))] = QCHDMAN_PRJ_COPY;
    cloneActionMap[menuCloneActions->addAction(tr("CreateRaw"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_RAW;
    cloneActionMap[menuCloneActions->addAction(tr("CreadeHD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_HD;
    cloneActionMap[menuCloneActions->addAction(tr("CreateCD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_CD;
    cloneActionMap[menuCloneActions->addAction(tr("CreateLD"), this, SLOT(clone()))] = QCHDMAN_PRJ_CREATE_LD;
    cloneActionMap[menuCloneActions->addAction(tr("ExtractRaw"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_RAW;
    cloneActionMap[menuCloneActions->addAction(tr("ExtractHD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_HD;
    cloneActionMap[menuCloneActions->addAction(tr("ExtractCD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_CD;
    cloneActionMap[menuCloneActions->addAction(tr("ExtractLD"), this, SLOT(clone()))] = QCHDMAN_PRJ_EXTRACT_LD;
    actionCloneMenu = menuActions->addMenu(menuCloneActions);

    menuMorphActions = new QMenu(tr("Morph to"), this);
    morphActionMap[menuMorphActions->addAction(tr("Info"), this, SLOT(morph()))] = QCHDMAN_PRJ_INFO;
    morphActionMap[menuMorphActions->addAction(tr("Verify"), this, SLOT(morph()))] = QCHDMAN_PRJ_VERIFY;
    morphActionMap[menuMorphActions->addAction(tr("Copy"), this, SLOT(morph()))] = QCHDMAN_PRJ_COPY;
    morphActionMap[menuMorphActions->addAction(tr("CreateRaw"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_RAW;
    morphActionMap[menuMorphActions->addAction(tr("CreadeHD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_HD;
    morphActionMap[menuMorphActions->addAction(tr("CreateCD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_CD;
    morphActionMap[menuMorphActions->addAction(tr("CreateLD"), this, SLOT(morph()))] = QCHDMAN_PRJ_CREATE_LD;
    morphActionMap[menuMorphActions->addAction(tr("ExtractRaw"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_RAW;
    morphActionMap[menuMorphActions->addAction(tr("ExtractHD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_HD;
    morphActionMap[menuMorphActions->addAction(tr("ExtractCD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_CD;
    morphActionMap[menuMorphActions->addAction(tr("ExtractLD"), this, SLOT(morph()))] = QCHDMAN_PRJ_EXTRACT_LD;
    actionMorphMenu = menuActions->addMenu(menuMorphActions);

    actionCopyStdoutToClipboard = menuActions->addAction(tr("Copy stdout to clipboard"), this, SLOT(copyStdoutToClipboard()));
    actionCopyStderrToClipboard = menuActions->addAction(tr("Copy stderr to clipboard"), this, SLOT(copyStderrToClipboard()));
    actionCopyCommandToClipboard = menuActions->addAction(tr("Copy command to clipboard"), this, SLOT(copyCommandToClipboard()));

    menuActions->insertSeparator(actionSave);
    menuActions->insertSeparator(actionCloneMenu);
    menuActions->insertSeparator(actionCopyStdoutToClipboard);
    menuActions->insertSeparator(actionCopyCommandToClipboard);
    ui->toolButtonActions->setMenu(menuActions);

    QFont f;
    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    ui->plainTextEditProjectLog->setFont(f);

    connect(ui->spinBoxCreateHDCylinders, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
    connect(ui->spinBoxCreateHDHeads, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
    connect(ui->spinBoxCreateHDSectors, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));
    connect(ui->spinBoxCreateHDSectorSize, SIGNAL(valueChanged(int)), this, SLOT(updateCreateHDDiskCapacity()));

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
        parentWidget()->setWindowIcon(QIcon(":/images/info.png"));
        break;
    case QCHDMAN_PRJ_VERIFY:
        widgetHeight = ui->frameVerify->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelVerifyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
        if ( isAquaStyle )
            widgetHeight -= ui->labelVerifyHelp->margin();
#endif
        parentWidget()->setWindowIcon(QIcon(":/images/verify.png"));
        break;
    case QCHDMAN_PRJ_COPY:
        widgetHeight = ui->frameCopy->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelCopyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
        if ( isAquaStyle )
            widgetHeight -= ui->labelCopyHelp->margin();
#endif
        parentWidget()->setWindowIcon(QIcon(":/images/copy.png"));
        break;
    case QCHDMAN_PRJ_CREATE_RAW:
        widgetHeight = ui->frameCreateRaw->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelCreateRawHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
        if ( isAquaStyle )
            widgetHeight -= ui->labelCreateRawHelp->margin();
#endif
        parentWidget()->setWindowIcon(QIcon(":/images/createraw.png"));
        break;
    case QCHDMAN_PRJ_CREATE_HD:
        widgetHeight = ui->frameCreateHD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelCreateHDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
        if ( isAquaStyle )
            widgetHeight -= ui->labelCreateHDHelp->margin();
#endif
        parentWidget()->setWindowIcon(QIcon(":/images/createhd.png"));
        break;
    case QCHDMAN_PRJ_CREATE_CD:
        parentWidget()->setWindowIcon(QIcon(":/images/createcd.png"));
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        parentWidget()->setWindowIcon(QIcon(":/images/createld.png"));
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        parentWidget()->setWindowIcon(QIcon(":/images/extractraw.png"));
        break;
    case QCHDMAN_PRJ_EXTRACT_HD:
        widgetHeight = ui->frameExtractHD->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelExtractHDHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
#if defined(Q_OS_MAC)
        if ( isAquaStyle )
            widgetHeight -= ui->labelExtractHDHelp->margin();
#endif
        parentWidget()->setWindowIcon(QIcon(":/images/extracthd.png"));
        break;
    case QCHDMAN_PRJ_EXTRACT_CD:
        parentWidget()->setWindowIcon(QIcon(":/images/extractcd.png"));
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        parentWidget()->setWindowIcon(QIcon(":/images/extractld.png"));
        break;
    }

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
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        projectTypeName = tr("CreateLD");
        arguments << "createld";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        projectTypeName = tr("ExtractRaw");
        arguments << "extractraw";
        // FIXME
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
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        projectTypeName = tr("ExtractLD");
        arguments << "extractld";
        // FIXME
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
    connect(chdmanProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));

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
    runningProjects++;
#if defined(Q_OS_WIN)
    log(tr("process started: PID = %1").arg(chdmanProc->pid()->dwProcessId));
#else
    log(tr("process started: PID = %1").arg(chdmanProc->pid()));
#endif
    ui->toolButtonStop->setEnabled(true);
    ui->progressBar->setFormat(tr("Running"));
}

void ProjectWidget::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    runningProjects--;
    QString statusString = tr("normal");
    if ( exitStatus == QProcess::CrashExit ) {
        if ( terminatedOnDemand )
            statusString = tr("terminated");
        else
            statusString = tr("crashed");
    }
    log(tr("process finished: exitCode = %1, exitStatus = %2").arg(exitCode).arg(statusString));
    ui->toolButtonRun->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
    ui->comboBoxProjectType->setEnabled(true);
    menuMorphActions->setEnabled(true);
    actionLoad->setEnabled(true);
    ui->progressBar->setFormat(tr("Idle"));
    ui->progressBar->setValue(0);
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
    QStringList sl = s.split("\n");
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
            case QCHDMAN_PRJ_EXTRACT_HD:
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
}

void ProjectWidget::error(QProcess::ProcessError processError)
{
    QString errString;
    bool doLog = true;

    switch ( processError ) {
    case QProcess::FailedToStart:
        errString = tr("failed to start");
        break;
    case QProcess::Crashed:
        errString = tr("crashed");
        if ( terminatedOnDemand )
            doLog = false;
        else
        break;
    case QProcess::Timedout:
        errString = tr("timed out");
        break;
    case QProcess::WriteError:
        errString = tr("write error");
        break;
    case QProcess::ReadError:
        errString = tr("read error");
        break;
    case QProcess::UnknownError:
    default:
        errString = tr("unknown");
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
    ui->progressBar->setValue(0);
}

void ProjectWidget::stateChanged(QProcess::ProcessState)
{
    // NOP
}

void ProjectWidget::on_toolButtonBrowseInfoInputFile_clicked()
{
    QString folder = ui->lineEditInfoInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditInfoInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyInputFile_clicked()
{
    QString folder = ui->lineEditVerifyInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditVerifyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyParentInputFile_clicked()
{
    QString folder = ui->lineEditVerifyParentInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditVerifyParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyInputFile_clicked()
{
    QString folder = ui->lineEditCopyInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCopyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyOutputFile_clicked()
{
    QString folder = ui->lineEditCopyOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCopyOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentInputFile_clicked()
{
    QString folder = ui->lineEditCopyParentInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCopyParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentOutputFile_clicked()
{
    QString folder = ui->lineEditCopyParentOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
    QString s = QFileDialog::getOpenFileName(this, tr("Choose input file"), folder, tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCreateRawInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateRawOutputFile_clicked()
{
    QString folder = ui->lineEditCreateRawOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCreateRawOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateRawParentOutputFile_clicked()
{
    QString folder = ui->lineEditCreateRawParentOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
    QString s = QFileDialog::getOpenFileName(this, tr("Choose input file"), folder, tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCreateHDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDOutputFile_clicked()
{
    QString folder = ui->lineEditCreateHDOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCreateHDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDParentOutputFile_clicked()
{
    QString folder = ui->lineEditCreateHDParentOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose parent CHD output file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditCreateHDParentOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCreateHDIdentFile_clicked()
{
    QString folder = ui->lineEditCreateHDIdentFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose ident file"), folder, tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
        QString category = entryInfo[0];
        QString entry = entryInfo[1];
        QList<DiskGeometry> geoList = mainWindow->hardDiskTemplates[category];
        bool found = false;
        DiskGeometry geo;
        for (int i = 0; i < geoList.count() && !found; i++) {
            geo = geoList[i];
            found = ( geo.name == entry );
        }
        if ( found ) {
            ui->spinBoxCreateHDCylinders->setValue(geo.cyls);
            ui->spinBoxCreateHDHeads->setValue(geo.heads);
            ui->spinBoxCreateHDSectors->setValue(geo.sectors);
            ui->spinBoxCreateHDSectorSize->setValue(geo.sectorSize);
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

void ProjectWidget::on_toolButtonBrowseExtractHDInputFile_clicked()
{
    QString folder = ui->lineEditExtractHDInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditExtractHDInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractHDOutputFile_clicked()
{
    QString folder = ui->lineEditExtractHDOutputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredOutputFolder;
    QString s = QFileDialog::getSaveFileName(this, tr("Choose output file"), folder, tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditExtractHDOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseExtractHDParentInputFile_clicked()
{
    QString folder = ui->lineEditExtractHDParentInputFile->text();
    if ( folder.isEmpty() )
        folder = mainWindow->preferredCHDInputFolder;
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), folder, tr("CHD files (*.chd);;All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !s.isNull() )
        ui->lineEditExtractHDParentInputFile->setText(s);
}

void ProjectWidget::init()
{
    on_comboBoxProjectType_currentIndexChanged(QCHDMAN_PRJ_INFO);
}

void ProjectWidget::log(QString message)
{
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + projectTypeName + ": ");
    ui->plainTextEditProjectLog->appendPlainText(message);
    ui->plainTextEditProjectLog->verticalScrollBar()->setValue(ui->plainTextEditProjectLog->verticalScrollBar()->maximum());
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

void ProjectWidget::load(const QString &fileName)
{
    QString fName = fileName;
    if ( fName.isEmpty() ) {
        fName = QFileDialog::getOpenFileName(this, tr("Choose file"), QString(), tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
        if ( fName.isNull() )
            return;
    }

    QFile loadFile(fName);
    if ( loadFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        QTextStream ts(&loadFile);
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
                                if ( compressionTypes.contains(cmp) ) {
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
                                if ( compressionTypes.contains(cmp) ) {
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
                                if ( compressionTypes.contains(cmp) ) {
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
                    // FIXME
                    break;
                case QCHDMAN_PRJ_CREATE_LD:
                    // FIXME
                    break;
                case QCHDMAN_PRJ_EXTRACT_RAW:
                    // FIXME
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
                    // FIXME
                    break;
                case QCHDMAN_PRJ_EXTRACT_LD:
                    // FIXME
                    break;
                }
            }
            ui->comboBoxProjectType->setCurrentIndex(projectType);
            qApp->processEvents();
            on_comboBoxProjectType_currentIndexChanged(projectType);
        }
        loadFile.close();
        ((ProjectWindow *)parentWidget())->projectName = fName;
        parentWidget()->setWindowTitle(fName);
        mainWindow->addRecentFile(fName);
    } else
        mainWindow->statusBar()->showMessage(tr("Failed loading project '%1'").arg(fName), QCHDMAN_STATUS_MSGTIME);
}

void ProjectWidget::save()
{
    QString projectName = ((ProjectWindow *)parentWidget())->projectName;

    if ( projectName.startsWith(tr("Noname-%1").arg("")) )
        askFileName = true;

    saveAs(projectName);

    askFileName = false;
}

void ProjectWidget::saveAs(const QString &fileName)
{
    QString projectName = fileName;

    if ( projectName.isEmpty() || askFileName ) {
        projectName = ((ProjectWindow *)parentWidget())->projectName;
        if ( projectName.startsWith(tr("Noname-%1").arg("")) || projectName.isEmpty() || askFileName ) {
            QString s = QFileDialog::getSaveFileName(this, tr("Choose file"), projectName, tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
            if ( s.isNull() )
                return;
            else
                projectName = s;
        }
    }

    QFile saveFile(projectName);
    if ( saveFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        QTextStream ts(&saveFile);
        int projectType = ui->comboBoxProjectType->currentIndex();
        ts << "# " << tr("Qt CHDMAN project file -- please do not edit manually") << "\n";
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
            break;
        case QCHDMAN_PRJ_CREATE_LD:
            break;
        case QCHDMAN_PRJ_EXTRACT_RAW:
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
            break;
        case QCHDMAN_PRJ_EXTRACT_LD:
            break;
        }
        saveFile.close();
        ((ProjectWindow *)parentWidget())->projectName = projectName;
        parentWidget()->setWindowTitle(projectName);
        mainWindow->statusBar()->showMessage(tr("Project '%1' saved").arg(projectName), QCHDMAN_STATUS_MSGTIME);
        mainWindow->addRecentFile(projectName);
    } else
        mainWindow->statusBar()->showMessage(tr("Failed saving project '%1'").arg(projectName), QCHDMAN_STATUS_MSGTIME);
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
        ProjectWindow *projectWindow = mainWindow->createProjectWindow();
        ProjectWidget *projectWidget = projectWindow->projectWidget;
        projectWidget->setProjectType(cloneType);
        QList<QWidget *> sourceWidgets = copyGroups[ui->comboBoxProjectType->currentIndex()];
        QList<int> sourceTypes = copyTypes[ui->comboBoxProjectType->currentIndex()];
        QList<QWidget *> targetWidgets = projectWidget->copyGroups[cloneType];
        QList<int> targetTypes = projectWidget->copyTypes[cloneType];
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
                        }
                        foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
                            if ( compressionTypes.contains(cmp) ) {
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
        QList<int> sourceTypes = copyTypes[ui->comboBoxProjectType->currentIndex()];
        QList<QWidget *> targetWidgets = copyGroups[morphType];
        QList<int> targetTypes = copyTypes[morphType];
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
                        }
                        foreach (QString cmp, compression.split(",", QString::SkipEmptyParts)) {
                            if ( compressionTypes.contains(cmp) ) {
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
