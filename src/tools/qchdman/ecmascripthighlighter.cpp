#include "ecmascripthighlighter.h"

ECMAScriptHighlighter::ECMAScriptHighlighter(QTextDocument *parent) :
	QSyntaxHighlighter(parent)
{
	QStringList keywords;
	keywords << "break" << "do" << "instanceof" << "typeof"
		 << "case" << "else" << "new" << "var"
		 << "catch" << "finally" << "return" << "void"
		 << "continue" << "for" << "switch" << "while"
		 << "debugger" << "function" << "this" << "with"
		 << "default" << "if" << "throw" << "delete"
		 << "in" << "try";

	mKeywordFormat.setForeground(Qt::darkBlue);
	mKeywordFormat.setFontWeight(QFont::Bold);
	HighlightingRule rule;
	foreach (QString keyword, keywords) {
		rule.pattern = QRegExp("\\b" + keyword + "\\b");
		rule.format = mKeywordFormat;
		mHighlightingRules.append(rule);
	}

	keywords.clear();
	keywords << "scriptEngine" << "qchdman";
	mScriptEngineFormat.setForeground(Qt::darkMagenta);
	mScriptEngineFormat.setFontWeight(QFont::Bold);
	foreach (QString keyword, keywords) {
		rule.pattern = QRegExp("\\b" + keyword + "\\b");
		rule.format = mScriptEngineFormat;
		mHighlightingRules.append(rule);
	}

	mQuotationFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("\"[^\"]*\"|'[^']*'");
	rule.format = mQuotationFormat;
	mHighlightingRules.append(rule);

	mFunctionFormat.setForeground(Qt::blue);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format = mFunctionFormat;
	mHighlightingRules.append(rule);

	mSingleLineCommentExpression = QRegExp("(?=[^\"]*)//(?=[^\"]*)|(?=[^']*)//(?=[^']*)");
	mSingleLineCommentFormat.setForeground(Qt::darkGray);
	mSingleLineCommentFormat.setFontItalic(true);

	mMultiLineCommentStartExpression = QRegExp("(?=[^\"]*)/\\*(?=[^\"]*)|(?=[^']*)/\\*(?=[^']*)");
	mMultiLineCommentEndExpression = QRegExp("(?=[^\"]*)\\*/(?=[^\"]*)|(?=[^']*)\\*/(?=[^']*)");
	mMultiLineCommentFormat.setForeground(Qt::darkGray);
	mMultiLineCommentFormat.setFontItalic(true);
}

void ECMAScriptHighlighter::highlightBlock(const QString &text)
{
	foreach (const HighlightingRule &rule, mHighlightingRules) {
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while ( index >= 0 ) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}

	setCurrentBlockState(0);

	int commentStartIndex = 0, singleLineCommentStartIndex = -1;
	bool isSingleLineComment = false;

	if ( previousBlockState() != 1 ) {
		commentStartIndex = mMultiLineCommentStartExpression.indexIn(text);
		singleLineCommentStartIndex = mSingleLineCommentExpression.indexIn(text);
		if ( singleLineCommentStartIndex >= 0 && (singleLineCommentStartIndex < commentStartIndex || commentStartIndex < 0) ) {
			isSingleLineComment = true;
			commentStartIndex = singleLineCommentStartIndex;
		}
	}

	if ( commentStartIndex >= 0 )
		if ( format(commentStartIndex) == mQuotationFormat )
			return;

	if ( isSingleLineComment )
		setFormat(commentStartIndex, text.length() - singleLineCommentStartIndex, mSingleLineCommentFormat);
	else {
		while ( commentStartIndex >= 0 ) {
			int multiLineCommentEndIndex = mMultiLineCommentEndExpression.indexIn(text, commentStartIndex);
			int commentLength;
			if ( multiLineCommentEndIndex == -1 ) {
				setCurrentBlockState(1);
				commentLength = text.length() - commentStartIndex;
			} else {
				commentLength = multiLineCommentEndIndex - commentStartIndex + mMultiLineCommentEndExpression.matchedLength();
			}
			setFormat(commentStartIndex, commentLength, mMultiLineCommentFormat);
			commentStartIndex = mMultiLineCommentStartExpression.indexIn(text, commentStartIndex + commentLength);
		}
	}
}
