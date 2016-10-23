#include <QApplication>
#include <QFontMetrics>
#include <QPainterPath>
#include <QFont>
#include <QMovie>
#include <QPixmap>
#include <QTimer>

#include "aspectratiolabel.h"

AspectRatioLabel::AspectRatioLabel(QWidget *parent, qreal scale) :
	QLabel(parent),
	m_scale(scale)
{
	setAlignment(Qt::AlignCenter);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void AspectRatioLabel::adjustMovieSize()
{
	setUpdatesEnabled(false);
	QMovie::MovieState state = movie()->state();
	int frame = movie()->currentFrameNumber();
	movie()->setFileName(movie()->fileName());
	QSize sz(250, 250);
	sz.scale(size() * m_scale, Qt::KeepAspectRatio);
	movie()->setScaledSize(sz);
	movie()->jumpToFrame(frame);
	switch ( state ) {
		case QMovie::Running:
			movie()->start();
			break;
		case QMovie::Paused:
			movie()->setPaused(true);
			break;
		default:
			movie()->stop();
			break;
	}
	setUpdatesEnabled(true);
}

void AspectRatioLabel::setLabelText(QString text)
{
	m_labelText = text;
	update();
}

void AspectRatioLabel::resizeEvent(QResizeEvent *e)
{
	QLabel::resizeEvent(e);
	if ( movie() )
		adjustMovieSize();
}

void AspectRatioLabel::showEvent(QShowEvent *e)
{
	QLabel::showEvent(e);
	if ( movie() )
		adjustMovieSize();
}

void AspectRatioLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);
	if ( !m_labelText.isEmpty() ) {
		m_painter.begin(this);
		m_painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
		QFont f(qApp->font());
		f.setWeight(QFont::Bold);
		m_painter.setFont(f);
		QFontMetrics fm(f);
		int adjustment = fm.height() / 2;
		QRect r = rect();
		r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
		r = m_painter.boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, m_labelText);
		r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
		QPainterPath pp;
		pp.addRoundedRect(r, 5, 5);
		m_painter.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		m_painter.setPen(QPen(QColor(255, 255, 255, 255)));
		m_painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, m_labelText);
		m_painter.end();
	}
}
