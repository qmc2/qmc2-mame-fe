#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>
#include <QResizeEvent>
#include <QRect>

class ScriptEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	explicit ScriptEditor(QWidget *parent = 0);
	virtual ~ScriptEditor();

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void updateLineNumberAreaWidth(int w = 0);
	void updateLineNumberArea(const QRect &, int);

private:
	QWidget *mLineNumberArea;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(ScriptEditor *editor) : QWidget(editor) {
		mScriptEditor = editor;
	}

	QSize sizeHint() const {
		return QSize(mScriptEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) {
		mScriptEditor->lineNumberAreaPaintEvent(event);
	}

private:
	ScriptEditor *mScriptEditor;
};

#endif // SCRIPTEDITOR_H
