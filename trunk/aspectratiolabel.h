#ifndef ASPECTRATIOPIXMAPLABEL_H
#define ASPECTRATIOPIXMAPLABEL_H

#include <QLabel>
#include <QString>
#include <QResizeEvent>
#include <QPaintEvent>

class AspectRatioLabel : public QLabel
{
	Q_OBJECT

	public:
		explicit AspectRatioLabel(QWidget *parent = 0);

	public slots:
		void adjustMovieSize();
		void setLabelText(QString text = QString());

	protected:
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);

	private:
		QString m_labelText;
};

#endif
