#ifndef ECMASCRIPTHIGHLIGHTER_H
#define ECMASCRIPTHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class ECMAScriptHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	explicit ECMAScriptHighlighter(QTextDocument *parent);

protected:
	virtual void highlightBlock(const QString &);

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> mHighlightingRules;

	QRegExp mMultiLineCommentStartExpression;
	QRegExp mMultiLineCommentEndExpression;
	QRegExp mSingleLineCommentExpression;

	QTextCharFormat mKeywordFormat;
	QTextCharFormat mSingleLineCommentFormat;
	QTextCharFormat mMultiLineCommentFormat;
	QTextCharFormat mQuotationFormat;
	QTextCharFormat mFunctionFormat;
	QTextCharFormat mScriptEngineFormat;
};

#endif // ECMASCRIPTHIGHLIGHTER_H
