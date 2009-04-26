#ifndef _SMPCHECK_H_
#define _SMPCHECK_H_

#include <QDialog>
#include <QProcess>
#include <QTime>
#include <QStringList>
#include "ui_sampcheck.h"

class SampleChecker : public QDialog, public Ui::SampleChecker
{
  Q_OBJECT

  public:
    QProcess *verifyProc;
    QTime verifyTimer;
    QStringList sampleSets;
    bool ignoreResizeAndMove;
    QString stdoutLastLine, stderrLastLine;

    SampleChecker(QWidget *parent = 0);
    ~SampleChecker();

  public slots:
    void restoreLayout();
    void selectItem(QString);
    void recursiveFileList(const QString &, QStringList &);
    void verify();
    void verifyStarted();
    void verifyFinished(int, QProcess::ExitStatus);
    void verifyObsolete();
    void verifyReadyReadStandardOutput();
    void verifyReadyReadStandardError();
    void verifyError(QProcess::ProcessError);
    void verifyStateChanged(QProcess::ProcessState);

    // callback handlers
    void on_pushButtonSamplesCheck_clicked();
    void on_pushButtonSamplesRemoveObsolete_clicked();
    void on_listWidgetSamplesGood_itemSelectionChanged();
    void on_listWidgetSamplesGood_clicked(const QModelIndex &) { on_listWidgetSamplesGood_itemSelectionChanged(); }
    void on_listWidgetSamplesBad_itemSelectionChanged();
    void on_listWidgetSamplesBad_clicked(const QModelIndex &) { on_listWidgetSamplesBad_itemSelectionChanged(); }

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
