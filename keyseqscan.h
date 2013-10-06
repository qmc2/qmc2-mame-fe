#ifndef _KEYSEQSCAN_H_
#define _KEYSEQSCAN_H_

#include <QTimer>
#include "ui_keyseqscan.h"

class KeySequenceScanner : public QDialog, public Ui::KeySequenceScanner
{
	Q_OBJECT

	public:
		QTimer animTimer;
		int animSeq;
		int keySequence;
		Qt::KeyboardModifiers seqModifiers;
		QString currentKeySequence;
		bool specialKey;
		bool onlyOneKey;
		bool clearClicked;

		KeySequenceScanner(QWidget *parent = 0, bool special = false, bool onlyOne = false, bool showClearButton = false);

	public slots:
		void animationTimeout();
		void on_pushButtonClear_clicked() { clearClicked = true; }

	protected:
		void keyPressEvent(QKeyEvent *);
		void keyReleaseEvent(QKeyEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
