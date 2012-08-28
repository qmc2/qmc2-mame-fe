#include <QToolButton>
#include <QApplication>
#include <QFontMetrics>
#include <QStyle>

#include "iconlineedit.h"

IconLineEdit::IconLineEdit(QIcon icon, int align, QWidget *parent) : QLineEdit(parent)
{
	alignment = align;
	dummyButton = new QToolButton(this);
	dummyButton->setIcon(icon);
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height(), fm.height());
	dummyButton->setIconSize(iconSize);
	dummyButton->setStyleSheet("QToolButton { border: none; padding: 0px; }");
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	if ( alignment == QMC2_ALIGN_LEFT )
		setStyleSheet(QString("QLineEdit { padding-left: %1px; } ").arg(dummyButton->sizeHint().width() + frameWidth + 1));
	else
		setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(dummyButton->sizeHint().width() + frameWidth + 1));
	//QSize msz = minimumSizeHint();
	//setMinimumSize(qMax(msz.width(), dummyButton->sizeHint().height() + frameWidth * 2 + 2), qMax(msz.height(), dummyButton->sizeHint().height() + frameWidth * 2 + 2));
}

void IconLineEdit::centerIcon()
{
	int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
	if ( alignment == QMC2_ALIGN_LEFT )
		dummyButton->move(frameWidth, (rect().bottom() + 1 - dummyButton->size().height()) / 2);
	else {
		QSize sz = dummyButton->size();
		dummyButton->move(rect().right() - frameWidth - sz.width(), (rect().bottom() + 1 - sz.height())/2);
	}
}

void IconLineEdit::showEvent(QShowEvent *)
{
	centerIcon();
}

void IconLineEdit::resizeEvent(QResizeEvent *)
{
	centerIcon();
}
