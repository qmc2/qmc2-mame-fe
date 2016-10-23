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
		explicit AspectRatioLabel(QWidget *parent = 0, qreal scale = 0.5);

		void setScale(qreal scale) { m_scale = scale; }
		qreal scale() { return m_scale; }

	public slots:
		void adjustMovieSize();
		void setLabelText(QString text = QString());
		void clearLabelText() { setLabelText(); }

	protected:
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);
		void showEvent(QShowEvent *);

	private:
		QString m_labelText;
		QPainter m_painter;
		qreal m_scale;
};

#endif
