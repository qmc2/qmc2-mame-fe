#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    int nextProjectID;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    // File menu
    void on_actionProjectNew_triggered(bool);
    void on_actionProjectLoad_triggered(bool);
    void on_actionProjectSave_triggered(bool);
    void on_actionProjectSaveAs_triggered(bool);
    void on_actionProjectPreferences_triggered(bool);
    void on_actionProjectExit_triggered(bool);

    // Window menu
    void on_actionWindowNext_triggered(bool);
    void on_actionWindowPrevious_triggered(bool);
    void on_actionWindowTile_triggered(bool);
    void on_actionWindowCascade_triggered(bool);
    void on_actionWindowClose_triggered(bool);
    void on_actionWindowCloseAll_triggered(bool);
    void on_actionWindowViewModeWindowed_triggered(bool);
    void on_actionWindowViewModeTabbed_triggered(bool);

    // Help menu
    void on_actionHelpAbout_triggered(bool);
    void on_actionHelpAboutQt_triggered(bool);

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
