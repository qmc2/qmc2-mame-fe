#ifndef PROJECTWIDGET_H
#define PROJECTWIDGET_H

#include <QtGui>

namespace Ui {
class ProjectWidget;
}

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
    QStringList arguments;

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

    // CreateHardDisk

    // CreateCD

    // CreateLaserDisc

    // ExtractRaw

    // ExtractHardDisk

    // ExtractCD

    // ExtractLaserDisc

    // Other
    void init();
    void log(QString);
    void setLogFont(QFont);
    void setProjectType(int);
    void copyStdoutToClipboard();
    void copyStderrToClipboard();
    void copyCommandToClipboard();
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
