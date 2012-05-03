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
    explicit ProjectWidget(QWidget *parent = 0);
    ~ProjectWidget();

    QSize splitterSizes();

public slots:
    void on_comboBoxProjectType_currentIndexChanged(int);
    void on_toolButtonRun_clicked();
    void on_splitter_splitterMoved(int, int);

    // Verify
    void on_toolButtonBrowseVerifyInputFile_clicked();
    void on_toolButtonBrowseVerifyParentInputFile_clicked();

    void log(QString);

private:
    Ui::ProjectWidget *ui;
};

#endif // PROJECTWIDGET_H
