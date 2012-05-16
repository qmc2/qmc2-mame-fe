#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

#include <QtGui>

namespace Ui {
class ProjectWidget;
}

class DiskGeometry
{
public:
    QString name;
    int cyls, heads, sectors, sectorSize;

    DiskGeometry()
    {
        name.clear();
        cyls = heads = sectors = sectorSize = 0;
    }

    DiskGeometry(QString n, int c, int h, int s, int ss)
    {
        name = n;
        cyls = c;
        heads = h;
        sectors = s;
        sectorSize = ss;
    }
};

class ProjectWidget : public QWidget
{
    Q_OBJECT

public:
    bool terminatedOnDemand;
    bool askFileName;
    bool needsTabbedUiAdjustment;
    bool needsWindowedUiAdjustment;
    QString stdoutOutput;
    QString stderrOutput;
    QString projectTypeName;
    QString lastLogMessage;
    QProcess *chdmanProc;
    QMenu *menuActions;
    QMenu *menuMorphActions;
    QMenu *menuCloneActions;
    QAction *actionCopyStdoutToClipboard;
    QAction *actionCopyStderrToClipboard;
    QAction *actionCopyCommandToClipboard;
    QAction *actionLoad;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionMorphMenu;
    QAction *actionCloneMenu;
    QMap<QAction *, int> morphActionMap;
    QMap<QAction *, int> cloneActionMap;
    QMap<QString, QString> compressionTypes;
    QMap<int, QList<QWidget *> > copyGroups;
    QMap<int, QList<int> > copyTypes;
    QStringList copyCompressors;
    QStringList createRawCompressors;
    QStringList createHDCompressors;
    QStringList createCDCompressors;
    QStringList arguments;
    QElapsedTimer projectTimer;
    QIcon currentIcon;

    explicit ProjectWidget(QWidget *parent = 0);
    ~ProjectWidget();

public slots:
    // Info
    void on_toolButtonBrowseInfoInputFile_clicked();

    // Verify
    void on_toolButtonBrowseVerifyInputFile_clicked();
    void on_toolButtonBrowseVerifyParentInputFile_clicked();

    // Copy
    void on_toolButtonBrowseCopyInputFile_clicked();
    void on_toolButtonBrowseCopyOutputFile_clicked();
    void on_toolButtonBrowseCopyParentInputFile_clicked();
    void on_toolButtonBrowseCopyParentOutputFile_clicked();
    void on_comboBoxCopyCompression_currentIndexChanged(int);

    // CreateRaw
    void on_toolButtonBrowseCreateRawInputFile_clicked();
    void on_toolButtonBrowseCreateRawOutputFile_clicked();
    void on_toolButtonBrowseCreateRawParentOutputFile_clicked();
    void on_comboBoxCreateRawCompression_currentIndexChanged(int);

    // CreateHD
    void on_toolButtonBrowseCreateHDInputFile_clicked();
    void on_toolButtonBrowseCreateHDOutputFile_clicked();
    void on_toolButtonBrowseCreateHDParentOutputFile_clicked();
    void on_toolButtonBrowseCreateHDIdentFile_clicked();
    void on_comboBoxCreateHDCompression_currentIndexChanged(int);
    void on_comboBoxCreateHDFromTemplate_currentIndexChanged(int);
    void updateCreateHDDiskCapacity();

    // CreateCD
    void on_toolButtonBrowseCreateCDInputFile_clicked();
    void on_toolButtonBrowseCreateCDOutputFile_clicked();
    void on_toolButtonBrowseCreateCDParentOutputFile_clicked();
    void on_comboBoxCreateCDCompression_currentIndexChanged(int);

    // CreateLD

    // ExtractRaw

    // ExtractHD
    void on_toolButtonBrowseExtractHDInputFile_clicked();
    void on_toolButtonBrowseExtractHDOutputFile_clicked();
    void on_toolButtonBrowseExtractHDParentInputFile_clicked();

    // ExtractCD

    // ExtractLD

    // DumpMeta

    // AddMeta

    // DelMeta

    // Other
    void init();
    void log(QString);
    void setLogFont(QFont);
    void setProjectType(int);
    void copyStdoutToClipboard();
    void copyStderrToClipboard();
    void copyCommandToClipboard();
    void updateCompression(QComboBox *, QStringList *, int);
    void load(const QString &fileName = QString());
    void save();
    void saveAs(const QString &fileName = QString());
    void triggerSaveAs();
    void triggerUpdate() { on_comboBoxProjectType_currentIndexChanged(-1); }
    void clone();
    void morph();
    void run();
    void stop();
    void on_comboBoxProjectType_currentIndexChanged(int);
    void on_toolButtonRun_clicked(bool refreshArgsOnly = false);
    void on_toolButtonStop_clicked();

    // Process control
    void started();
    void finished(int, QProcess::ExitStatus);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void error(QProcess::ProcessError);
    void stateChanged(QProcess::ProcessState);

private:
    Ui::ProjectWidget *ui;
};

#endif // PROJECTWIDGET_H
