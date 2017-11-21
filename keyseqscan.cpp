#include "keyseqscan.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;

KeySequenceScanner::KeySequenceScanner(QWidget *parent, bool special, bool onlyOne, bool showClearButton)
	: QDialog(parent)
{
	setupUi(this);

	if ( !showClearButton )
		pushButtonClear->hide();

	onlyOneKey = onlyOne;
	specialKey = special;
	clearClicked = false;
	if ( specialKey ) {
		labelStatus->setText(tr("Scanning special key"));
		setWindowTitle(tr("Scanning special key"));
	} else {
		labelStatus->setText(tr("Scanning shortcut"));
		setWindowTitle(tr("Scanning shortcut"));
	}
	keySequence = 0;

	animSeq = 0;
	animTimer = new QTimer(this);
	connect(animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
	QTimer::singleShot(0, this, SLOT(animationTimeout()));
}

KeySequenceScanner::~KeySequenceScanner()
{
	delete animTimer;
}

void KeySequenceScanner::hideEvent(QHideEvent *e)
{
	releaseKeyboard();
	QDialog::hideEvent(e);
}
 
void KeySequenceScanner::showEvent(QShowEvent *e)
{
	QDialog::showEvent(e);
	grabKeyboard();
}

void KeySequenceScanner::animationTimeout()
{
	switch ( animSeq ) {
		case 0:
			labelKeySequence->setText("<   >");
			break;
		case 1:
			labelKeySequence->setText("< <   > >");
			break;
		case 2:
			labelKeySequence->setText("< < <   > > >");
			break;
		case 3:
			labelKeySequence->setText("< < < <   > > > >");
			break;
		case 4:
			labelKeySequence->setText("< < < < <   > > > > >");
			break;
		case 5:
			labelKeySequence->setText("> > > > >   < < < < <");
			break;
		case 6:
			labelKeySequence->setText("> > > >   < < < <");
			break;
		case 7:
			labelKeySequence->setText("> > >   < < <");
			break;
		case 8:
			labelKeySequence->setText("> >   < <");
			break;
		case 9:
			labelKeySequence->setText(">   <");
			break;
	}

	if ( ++animSeq > 9 )
		animSeq = 0;

	animTimer->start(QMC2_ANIMATION_TIMEOUT);
}

void KeySequenceScanner::keyPressEvent(QKeyEvent *event)
{
	keySequence = event->key();
	seqModifiers = event->modifiers();
	if ( keySequence != 0 && keySequence != Qt::Key_unknown ) {
		animTimer->stop();
		keySequence += seqModifiers;
		if ( seqModifiers & Qt::KeypadModifier )
			keySequence -= Qt::KeypadModifier;
		QString keySeqString(QKeySequence(keySequence).toString());
		if ( onlyOneKey )
			keySequence -= seqModifiers;
		currentKeySequence = keySeqString;
		QStringList words;
		if ( keySeqString == "+" )
			words << "+";
		else
			words = keySeqString.split("+");
		keySeqString.clear();
		if ( onlyOneKey )
			keySeqString = QObject::tr(words[0].toLatin1());
		else {
			int i;
			for (i = 0; i < words.count(); i++) {
				if ( i > 0 )
					keySeqString += "+";
				keySeqString += QObject::tr(words[i].toLatin1());
			}
		}
		labelKeySequence->setText(keySeqString);

		if ( onlyOneKey ) {
			if ( words.count() > 0 )
				pushButtonOk->setEnabled(true);
			else {
				pushButtonOk->setEnabled(false);
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			}
		} else if ( specialKey ) {
			if ( words.isEmpty() ) {
				pushButtonOk->setEnabled(false);
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			} else {
				pushButtonOk->setEnabled(!labelKeySequence->text().endsWith("??"));
				if ( labelKeySequence->text() == "??" ) {
			  		pushButtonOk->setEnabled(false);
			  		animSeq = 0;
				  	animTimer->start(QMC2_ANIMATION_TIMEOUT);
				}
			}
		} else if ( labelKeySequence->text().endsWith("??") ) {
			pushButtonOk->setEnabled(false);
			if ( labelKeySequence->text() == "??" ) {
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			}
		} else
		pushButtonOk->setEnabled(true);
	} else {
		pushButtonOk->setEnabled(false);
		animationTimeout();
		animTimer->start(QMC2_ANIMATION_TIMEOUT);
	}

	event->accept();
}

void KeySequenceScanner::keyReleaseEvent(QKeyEvent *event)
{
	keySequence = event->key();
	if ( seqModifiers != event->modifiers() && labelKeySequence->text().endsWith("??") ) {
		seqModifiers = event->modifiers();
		animTimer->stop();
		keySequence += seqModifiers;
		if ( seqModifiers & Qt::KeypadModifier )
			keySequence -= Qt::KeypadModifier;
		QString keySeqString(QKeySequence(keySequence).toString());
		if ( onlyOneKey )
			keySequence -= seqModifiers;
		currentKeySequence = keySeqString;
		QStringList words;
		if ( keySeqString == "+" )
			words << "+";
		else
			words = keySeqString.split("+");
		keySeqString.clear();
		if ( onlyOneKey )
			keySeqString = QObject::tr(words[0].toLatin1());
		else {
			int i;
			for (i = 0; i < words.count(); i++) {
				if ( i > 0 )
					keySeqString += "+";
				keySeqString += QObject::tr(words[i].toLatin1());
			}
		}
		labelKeySequence->setText(keySeqString);

		if ( onlyOneKey ) {
			if ( words.count() > 0 )
				pushButtonOk->setEnabled(true);
			else {
				pushButtonOk->setEnabled(false);
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			}
		} else if ( specialKey ) {
			if ( words.isEmpty() ) {
				pushButtonOk->setEnabled(false);
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			} else {
				pushButtonOk->setEnabled(!labelKeySequence->text().endsWith("??"));
				if ( labelKeySequence->text() == "??" ) {
					pushButtonOk->setEnabled(false);
					animSeq = 0;
					animTimer->start(QMC2_ANIMATION_TIMEOUT);
				}
			}
		} else if ( labelKeySequence->text().endsWith("??") ) {
			pushButtonOk->setEnabled(false);
			if ( labelKeySequence->text() == "??" ) {
				animSeq = 0;
				animTimer->start(QMC2_ANIMATION_TIMEOUT);
			}
		} else
			pushButtonOk->setEnabled(true);
	}

	event->accept();
}
