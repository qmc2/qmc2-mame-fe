#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QMdiSubWindow>
#if QT_VERSION >= 0x050000
#include <QComboBox>
#endif
#include "projectwidget.h"
#include "scriptwidget.h"
#include "macros.h"

class ProjectWindow : public QMdiSubWindow
{
	Q_OBJECT

public:
	bool closeOk;
	int subWindowType;
	QString projectName;
	ProjectWidget *projectWidget;
	ScriptWidget *scriptWidget;

	explicit ProjectWindow(QString pn = QString(), int type = QCHDMAN_MDI_PROJECT, QWidget *parent = 0);
	virtual ~ProjectWindow();

	void setProjectName(QString);

public slots:
	void myWindowStateChanged(Qt::WindowStates, Qt::WindowStates);
	void systemMenuAction(QAction *);

protected:
	void closeEvent(QCloseEvent *);
};

#endif // PROJECTWINDOW_H
