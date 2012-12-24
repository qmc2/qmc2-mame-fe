#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QMainWindow>
#include <QLabel>
#endif

#include "preferencesdialog.h"
#include "projectwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    bool closeOk;
    bool forceQuit;
    QString humanReadableString;
    QLabel *statisticsLabel;
    QTimer statusTimer;
    int nextProjectID;
    PreferencesDialog *preferencesDialog;
    QStringList projectTypes;
    QStringList recentFiles;
    QMap<QString, QString> compressionTypes;
    QMap<QString, QList<DiskGeometry> > hardDiskTemplates;
    QString preferredCHDInputFolder;
    QString preferredInputFolder;
    QString preferredCHDOutputFolder;
    QString preferredOutputFolder;
    QMap<int, QList<int> > copyTypes;
    QMap<int, QIcon> iconMap;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QMdiArea *mdiArea();
    ProjectWindow *createProjectWindow();
    QString humanReadable(qreal);

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
    void updateSubWindows();
    void addRecentFile(const QString &);
    void loadRecentFile();
    void enableActions(bool enable = true);
    void disableActions() { enableActions(false); }
    void disableActionsRequiringTwo();
    void resetCloseFlag() { closeOk = true; }
    void on_mdiArea_subWindowActivated(QMdiSubWindow *);

protected:
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
