#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <preferencesdialog.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    bool closeOk;
    bool forceQuit;
    QLabel *statisticsLabel;
    QTimer statusTimer;
    int nextProjectID;
    PreferencesDialog *preferencesDialog;
    QStringList projectTypes;
    QStringList recentFiles;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    // File menu
    void on_actionProjectNew_triggered(bool checked = true);
    void on_actionProjectLoad_triggered(bool checked = true);
    void on_actionProjectSave_triggered(bool checked = true);
    void on_actionProjectSaveAs_triggered(bool checked = true);
    void on_actionProjectSaveAll_triggered(bool checked = true);
    void on_actionProjectPreferences_triggered(bool checked = true);
    void on_actionProjectExit_triggered(bool checked = true);

    // Window menu
    void on_actionWindowNext_triggered(bool checked = true);
    void on_actionWindowPrevious_triggered(bool checked = true);
    void on_actionWindowTile_triggered(bool checked = true);
    void on_actionWindowCascade_triggered(bool checked = true);
    void on_actionWindowClose_triggered(bool checked = true);
    void on_actionWindowCloseAll_triggered(bool checked = true);
    void on_actionWindowViewModeWindowed_triggered(bool checked = true);
    void on_actionWindowViewModeTabbed_triggered(bool checked = true);

    // Help menu
    void on_actionHelpAbout_triggered(bool checked = true);
    void on_actionHelpAboutQt_triggered(bool checked = true);

    // Other
    void updateStatus();
    void applySettings();
    void addRecentFile(const QString &);
    void loadRecentFile();
    void resetCloseFlag() { closeOk = true; }
    void on_mdiArea_subWindowActivated(QMdiSubWindow *);

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
