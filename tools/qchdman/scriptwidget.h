#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QtGui>
#include "projectwidget.h"

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ScriptWidget(QWidget *parent = 0);
    ~ScriptWidget();

    bool resizePending;

public slots:
    // Callbacks
    void on_toolButtonInputOutput_toggled(bool enable);
    void on_toolButtonRun_clicked();
    void on_toolButtonStop_clicked();
    void on_toolButtonAddGroup_clicked();
    void on_toolButtonRemoveGroup_clicked();
    void on_toolButtonAddProject_clicked();
    void on_toolButtonRemoveProject_clicked();
    void on_toolButtonAddCommand_clicked();
    void on_toolButtonRemoveCommand_clicked();

    // Other
    void doCleanUp();
    void doPendingResize();
    void tableWidgetInputOutput_sectionClicked(int);

protected:
    void resizeEvent(QResizeEvent *);

private:
    Ui::ScriptWidget *ui;
    int groupSeqNum, projectSeqNum, commandSeqNum;
    int lastWidgetWidth;
};

#endif // SCRIPTWIDGET_H
