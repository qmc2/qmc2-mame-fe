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
    QString jobName;
    QProcess *chdmanProc;

    explicit ProjectWidget(QWidget *parent = 0);
    ~ProjectWidget();

    QSize splitterSizes();

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
    void log(QString);
    void on_comboBoxProjectType_currentIndexChanged(int);
    void on_toolButtonRun_clicked();
    void on_splitter_splitterMoved(int, int);

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
