#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

public slots:
    void initialSetup();
    void on_pushButtonOk_clicked();

    // GUI

    // Paths
    void on_toolButtonBrowseChdmanBinary_clicked();

protected:
    void showEvent(QShowEvent *);

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
