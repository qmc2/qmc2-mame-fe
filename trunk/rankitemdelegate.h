#ifndef RANKITEMDELEGATE_H
#define RANKITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTreeView>
#include <QWidget>

#include "machinelistmodel.h"

class RankItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		explicit RankItemDelegate(QWidget *parent = 0);

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

	private:
		QTreeView *m_treeView;

		void updateForeignItems(MachineListModelItem *);
};

#endif 
