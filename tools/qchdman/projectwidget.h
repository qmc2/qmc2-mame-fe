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
    QString stdoutOutput;
    QString stderrOutput;
    QString jobName;
    QProcess *chdmanProc;
    QMenu *menuActions;
    QAction *actionCopyStdoutToClipboard;
    QAction *actionCopyStderrToClipboard;
    QAction *actionCopyCommandToClipboard;
    QAction *actionLoad;
    QAction *actionSave;
    QAction *actionSaveAs;
    bool terminatedOnDemand;
    QMap<QString, QString> compressionTypes;
    QStringList copyCompressors;
    QStringList arguments;
    bool askFileName;

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
    void copyStdoutToClipboard();
    void copyStderrToClipboard();
    void copyCommandToClipboard();
    void load(const QString &fileName = QString());
    void save();
    void saveAs(const QString &fileName = QString());
    void triggerSaveAs();
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
