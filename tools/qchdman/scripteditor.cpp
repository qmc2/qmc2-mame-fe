#include <QTextBlock>
#include <QPainter>
#include <QLatin1Char>

#include "scripteditor.h"

ScriptEditor::ScriptEditor(QWidget *parent) :
	QPlainTextEdit(parent)
{
	mLineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

	updateLineNumberAreaWidth();
}

ScriptEditor::~ScriptEditor()
{
	delete mLineNumberArea;
}

int ScriptEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());

	while ( max >= 10 ) {
		max /= 10;
		digits++;
	}

	return 1 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void ScriptEditor::updateLineNumberAreaWidth(int /* w */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void ScriptEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if ( dy )
		mLineNumberArea->scroll(0, dy);
	else
		mLineNumberArea->update(0, rect.y(), mLineNumberArea->width(), rect.height());

	if ( rect.contains(viewport()->rect()) )
		updateLineNumberAreaWidth();
}

void ScriptEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	mLineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void ScriptEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(mLineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while ( block.isValid() && top <= event->rect().bottom() ) {
		if ( block.isVisible() && bottom >= event->rect().top() ) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, mLineNumberArea->width(), fontMetrics().height(), Qt::AlignRight, number);
		}
		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}
