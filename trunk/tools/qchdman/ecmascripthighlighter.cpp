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

    int startIndex = 0;
    bool isSingleLineComment = false;
    if ( previousBlockState() != 1 ) {
        startIndex = mMultiLineCommentStartExpression.indexIn(text);
        int singleLineCommentIndex = mSingleLineCommentExpression.indexIn(text);
        if ( singleLineCommentIndex >= 0 && (singleLineCommentIndex < startIndex || startIndex < 0) ) {
            isSingleLineComment = true;
            startIndex = singleLineCommentIndex;
        }
    }

    if ( isSingleLineComment )
        setFormat(startIndex, text.length() - startIndex, mSingleLineCommentFormat);
    else {
        while ( startIndex >= 0 ) {
            int endIndex = mMultiLineCommentEndExpression.indexIn(text, startIndex);
            int commentLength;
            if ( endIndex == -1 ) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex + mMultiLineCommentEndExpression.matchedLength();
            }
            setFormat(startIndex, commentLength, mMultiLineCommentFormat);
            startIndex = mMultiLineCommentStartExpression.indexIn(text, startIndex + commentLength);
        }
    }
}
