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
    bool terminatedOnDemand;

    explicit ProjectWidget(QWidget *parent = 0);
    ~ProjectWidget();

public slots:
    // Info
    void on_toolButtonBrowseInfoInputFile_clicked();

    // Verify
    void on_toolButtonBrowseVerifyInputFile_clicked();
    void on_toolButtonBrowseVerifyParentInputFile_clicked();

    // CreateRaw

    // CreateHardDisk

    // CreateCD

    // CreateLaserDisc

    // ExtractRaw

    // ExtractHardDisk

    // ExtractCD

    // ExtractLaserDisc

    // Copy

    // Other
    void init();
    void log(QString);
    void setLogFont(QFont);
    void copyStdoutToClipboard();
    void copyStderrToClipboard();
    void on_comboBoxProjectType_currentIndexChanged(int);
    void on_toolButtonRun_clicked();
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
