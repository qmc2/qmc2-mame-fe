#ifndef ICONLINEEDIT_H
#define ICONLINEEDIT_H

#include <QLineEdit>
#include <QToolButton>

#include "macros.h"

class IconLineEdit : public QLineEdit
{
	Q_OBJECT

	public:
		IconLineEdit(QIcon, int align = QMC2_ALIGN_LEFT, QWidget *parent = 0);

		void setIconSize(const QSize &size) { dummyButton->setIconSize(size); }
		QToolButton *button() { return dummyButton; }

	protected:
		void resizeEvent(QResizeEvent *);
		void showEvent(QShowEvent *);

	public slots:
		void centerIcon();

	private:
		int alignment;
		QToolButton *dummyButton;
};

#endif
