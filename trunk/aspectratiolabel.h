#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QString>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QPainter>

class AspectRatioLabel : public QLabel
{
	Q_OBJECT

	public:
		explicit AspectRatioLabel(QWidget *parent = 0);

	public slots:
		void adjustMovieSize();
		void setLabelText(QString text = QString());
		void clearLabelText() { setLabelText(); }

	protected:
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);

	private:
		QString m_labelText;
		QPainter m_painter;
};

#endif
