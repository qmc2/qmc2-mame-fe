#ifndef _RANKITEMDELEGATE_H_
#define _RANKITEMDELEGATE_H_

#include <QStyledItemDelegate>
#include <QWidget>

class RankItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	public:
		explicit RankItemDelegate(QWidget *parent = 0);

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void setEditorData(QWidget *editor, const QModelIndex &index) const;
		void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

	private slots:
		void commitAndCloseEditor();
};

#endif 
