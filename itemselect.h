#ifndef _ITEMSELECT_H_
#define _ITEMSELECT_H_

#include <QStringList>
#include "ui_itemselect.h"

class ItemSelector : public QDialog, public Ui::ItemSelector
{
	Q_OBJECT

	public:
		ItemSelector(QWidget *, QStringList &);
};

#endif
