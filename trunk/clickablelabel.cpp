#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent, Qt::WindowFlags f) :
	QLabel(parent)
{
	// NOP
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
	emit clicked();
}
