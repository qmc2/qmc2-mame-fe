#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include <QPlainTextEdit>

class ConsoleWindow : public QPlainTextEdit
{
	Q_OBJECT

public:
	explicit ConsoleWindow(QWidget *parent = 0);
	virtual ~ConsoleWindow();

signals:

public slots:
	void loadSettings();
	void saveSettings();
};

#endif // CONSOLEWINDOW_H
