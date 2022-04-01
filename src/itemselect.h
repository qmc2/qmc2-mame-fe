#ifndef ITEMSELECT_H
#define ITEMSELECT_H

#include <QStringList>
#include "ui_itemselect.h"

class ItemSelector : public QDialog, public Ui::ItemSelector
{
	Q_OBJECT

	public:
		ItemSelector(QWidget *, QStringList &);
};

#endif
