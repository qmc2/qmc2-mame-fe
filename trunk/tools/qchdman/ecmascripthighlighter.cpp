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

    mSingleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegExp("//[^\n]*");
    rule.format = mSingleLineCommentFormat;
    mHighlightingRules.append(rule);

    mMultiLineCommentFormat.setForeground(Qt::red);

    mQuotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = mQuotationFormat;
    mHighlightingRules.append(rule);

    mFunctionFormat.setForeground(Qt::blue);
    mFunctionFormat.setFontItalic(true);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = mFunctionFormat;
    mHighlightingRules.append(rule);

    mCommentStartExpression = QRegExp("/\\*");
    mCommentEndExpression = QRegExp("\\*/");
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

    int startIndex = 0;
    if ( previousBlockState() != 1 )
        startIndex = mCommentStartExpression.indexIn(text);

    while ( startIndex >= 0 ) {
        int endIndex = mCommentEndExpression.indexIn(text, startIndex);
        int commentLength;
        if ( endIndex == -1 ) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + mCommentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, mMultiLineCommentFormat);
        startIndex = mCommentStartExpression.indexIn(text, startIndex + commentLength);
    }
}
