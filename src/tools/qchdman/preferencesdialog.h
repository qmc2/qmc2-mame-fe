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
	void restoreSettings();
	void applySettings();
	void on_pushButtonOk_clicked();
	void on_pushButtonApply_clicked();
	void on_pushButtonCancel_clicked();
	void on_toolButtonBrowseChdmanBinary_clicked();
	void on_toolButtonBrowsePreferredCHDInputPath_clicked();
	void on_toolButtonBrowsePreferredInputPath_clicked();
	void on_toolButtonBrowsePreferredCHDOutputPath_clicked();
	void on_toolButtonBrowsePreferredOutputPath_clicked();

protected:
	void showEvent(QShowEvent *);

private:
	Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
