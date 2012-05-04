#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMdiSubWindow>
#include "projectwidget.h"

class ProjectWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    bool firstShowEvent;
    QString projectName;
    ProjectWidget *projectWidget;

    ProjectWindow(QString pn = QString(), QWidget *parent = 0);
    ~ProjectWindow();

protected:
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void closeEvent(QCloseEvent *);
};

#endif // PROJECTWINDOW_H
