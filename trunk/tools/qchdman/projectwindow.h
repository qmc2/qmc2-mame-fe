#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMdiSubWindow>
#include "projectwidget.h"
#include "macros.h"

class ProjectWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    bool closeOk;
    int subWindowType;
    QString projectName;
    ProjectWidget *projectWidget;

    ProjectWindow(QString pn = QString(), int type = QCHDMAN_MDI_PROJECT, QWidget *parent = 0);
    ~ProjectWindow();

    void setProjectName(QString);

public slots:
    void myWindowStateChanged(Qt::WindowStates, Qt::WindowStates);
    void systemMenuAction(QAction *);

protected:
    void closeEvent(QCloseEvent *);
};

#endif // PROJECTWINDOW_H
