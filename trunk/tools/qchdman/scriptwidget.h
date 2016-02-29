#ifndef SCRIPTWIDGET_H
#define SCRIPTWIDGET_H

#include <QtGui>
#include <QSpinBox>

#include "projectwidget.h"
#include "scriptengine.h"

class ScriptEngine;

namespace Ui {
class ScriptWidget;
}

class ScriptWidget : public QWidget
{
	Q_OBJECT

	friend class ScriptEngine;

public:
	bool askFileName;
	bool isRunning;
	QMenu *menuActions;
	QAction *actionLoad;
	QAction *actionSave;
	QAction *actionSaveAs;
	QAction *actionClone;
	QAction *actionCopyLogToClipboard;
	QSpinBox *spinBoxLimitScriptLog;

	explicit ScriptWidget(QWidget *parent = 0);
	virtual ~ScriptWidget();

	ScriptEngine *engine() { return scriptEngine; }

public slots:
	// Callbacks
	void on_toolButtonRun_clicked();
	void on_toolButtonStop_clicked();
	void on_progressBar_valueChanged(int);

	// Other
	void adjustFonts();
	void log(QString);
	void load(const QString &fileName = QString(), QString *buffer = NULL);
	void save(QString *buffer = NULL);
	void saveAs(const QString &fileName = QString(), QString *buffer = NULL);
	QString toString();
	void fromString(QString);
	void triggerSaveAs();
	void resetProgressBar();
	void clone();
	void copyLogToClipboard();
	void saveSettings();
	void restoreSettings();
	void setLogLimit(int limit = 0);
	void on_tabWidget_currentChanged(int);

private:
	Ui::ScriptWidget *ui;
	ScriptEngine *scriptEngine;
	QLinearGradient linearGradient;
};

#endif // SCRIPTWIDGET_H
