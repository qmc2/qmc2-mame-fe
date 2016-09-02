#include "rankitemdelegate.h"
#include "rankitemwidget.h"
#include "macros.h"

RankItemDelegate::RankItemDelegate(QWidget *parent) :
	QStyledItemDelegate(parent)
{
	// NOP
}

void RankItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// FIXME
	QStyledItemDelegate::paint(painter, option, index);
}

QSize RankItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// FIXME
	return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *RankItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	// FIXME
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void RankItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	QStyledItemDelegate::setEditorData(editor, index);
}

void RankItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	// FIXME
	QStyledItemDelegate::setModelData(editor, model, index);
}

void RankItemDelegate::commitAndCloseEditor()
{
	// FIXME
	RankItemWidget *riw = qobject_cast<RankItemWidget *>(sender());
	emit commitData(riw);
	emit closeEditor(riw);
}
