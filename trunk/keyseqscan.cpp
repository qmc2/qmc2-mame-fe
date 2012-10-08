#include "keyseqscan.h"
#include "macros.h"
#include "options.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;

KeySequenceScanner::KeySequenceScanner(QWidget *parent, bool special, bool onlyOne)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeySequenceScanner::KeySequenceScanner(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

  setupUi(this);

  onlyOneKey = onlyOne;
  specialKey = special;
  if ( specialKey ) {
    labelStatus->setText(tr("Scanning special key"));
    setWindowTitle(tr("Scanning special key"));
  } else {
    labelStatus->setText(tr("Scanning shortcut"));
    setWindowTitle(tr("Scanning shortcut"));
  }
  keySequence = 0;
  animSeq = 0;
  animationTimeout();
  connect(&animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));
  animTimer.start(QMC2_ANIMATION_TIMEOUT);
  grabKeyboard();
}

KeySequenceScanner::~KeySequenceScanner()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: KeySequenceScanner::~KeySequenceScanner()");
#endif

  releaseKeyboard();
}

void KeySequenceScanner::animationTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: KeySequenceScanner::animationTimeout()");
#endif

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
  if ( ++animSeq > 9 ) animSeq = 0;
}

void KeySequenceScanner::keyPressEvent(QKeyEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeySequenceScanner::keyPressEvent(QKeyEvent *event = %1)").arg((qulonglong)event));
#endif

  keySequence = event->key();
  seqModifiers = event->modifiers();
  if ( keySequence != 0 && keySequence != Qt::Key_unknown ) {
    animTimer.stop();
    keySequence += seqModifiers;
    QString keySeqString(QKeySequence(keySequence).toString());
    if ( onlyOneKey ) keySequence -= seqModifiers;
    currentKeySequence = keySeqString;
    QStringList words = keySeqString.split("+");
    keySeqString.clear();
    if ( onlyOneKey ) {
      keySeqString = QObject::tr(words[0].toLatin1());
    } else {
      int i;
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) keySeqString += "+";
        keySeqString += QObject::tr(words[i].toLatin1());
      }
    }
    labelKeySequence->setText(keySeqString);

    if ( onlyOneKey ) {
      if ( words.count() > 0 ) {
        pushButtonOk->setEnabled(true);
      } else {
        pushButtonOk->setEnabled(false);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else if ( specialKey ) {
      if ( words.isEmpty() ) {
        pushButtonOk->setEnabled(false);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      } else {
        pushButtonOk->setEnabled(!labelKeySequence->text().endsWith("??"));
	if ( labelKeySequence->text() == "??" ) {
          pushButtonOk->setEnabled(false);
          animSeq = 0;
          animTimer.start(QMC2_ANIMATION_TIMEOUT);
        }
      }
    } else if ( labelKeySequence->text().endsWith("??") ) {
      pushButtonOk->setEnabled(false);
      if ( labelKeySequence->text() == "??" ) {
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else
      pushButtonOk->setEnabled(true);
  } else {
    pushButtonOk->setEnabled(false);
    animationTimeout();
    animTimer.start(QMC2_ANIMATION_TIMEOUT);
  }

  event->accept();
}

void KeySequenceScanner::keyReleaseEvent(QKeyEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeySequenceScanner::keyReleaseEvent(QKeyEvent *event = %1)").arg((qulonglong)event));
#endif

  keySequence = event->key();
  if ( seqModifiers != event->modifiers() && labelKeySequence->text().endsWith("??") ) {
    seqModifiers = event->modifiers();
    animTimer.stop();
    keySequence += seqModifiers;
    QString keySeqString(QKeySequence(keySequence).toString());
    if ( onlyOneKey ) keySequence -= seqModifiers;
    currentKeySequence = keySeqString;
    QStringList words = keySeqString.split("+");
    keySeqString.clear();
    if ( onlyOneKey ) {
      keySeqString = QObject::tr(words[0].toLatin1());
    } else {
      int i;
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) keySeqString += "+";
        keySeqString += QObject::tr(words[i].toLatin1());
      }
    }
    labelKeySequence->setText(keySeqString);

    if ( onlyOneKey ) {
      if ( words.count() > 0 ) {
        pushButtonOk->setEnabled(true);
      } else {
        pushButtonOk->setEnabled(false);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else if ( specialKey ) {
      if ( words.isEmpty() ) {
        pushButtonOk->setEnabled(false);
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      } else {
        pushButtonOk->setEnabled(!labelKeySequence->text().endsWith("??"));
	if ( labelKeySequence->text() == "??" ) {
          pushButtonOk->setEnabled(false);
          animSeq = 0;
          animTimer.start(QMC2_ANIMATION_TIMEOUT);
        }
      }
    } else if ( labelKeySequence->text().endsWith("??") ) {
      pushButtonOk->setEnabled(false);
      if ( labelKeySequence->text() == "??" ) {
        animSeq = 0;
        animTimer.start(QMC2_ANIMATION_TIMEOUT);
      }
    } else
      pushButtonOk->setEnabled(true);
  }

  event->accept();
}
