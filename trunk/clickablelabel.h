#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel
{
	Q_OBJECT 

	public:
		explicit ClickableLabel(QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());

	signals:
		void clicked();

	protected:
		void mousePressEvent(QMouseEvent* event);
};

#endif // CLICKABLELABEL_H
