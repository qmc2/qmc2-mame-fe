#include <QApplication>
#include <QPainterPath>
#include <QFontMetrics>
#include <QFont>
#include <QPainter>
#include <QMovie>
#include <QPixmap>
#include <QTimer>

#include "aspectratiolabel.h"

AspectRatioLabel::AspectRatioLabel(QWidget *parent)
	: QLabel(parent)
{
	setAlignment(Qt::AlignCenter);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}

void AspectRatioLabel::adjustMovieSize()
{
	QMovie::MovieState state = movie()->state();
	int frame = movie()->currentFrameNumber();
	movie()->setFileName(movie()->fileName());
	QSize sz(200, 200);
	sz.scale(size() * 0.66, Qt::KeepAspectRatio);
	movie()->setScaledSize(sz);
	movie()->jumpToFrame(frame);
	if ( state == QMovie::Running )
		movie()->start();
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

void AspectRatioLabel::paintEvent(QPaintEvent *e)
{
	QLabel::paintEvent(e);

	QSize s(size());
	if ( movie() )
		s = movie()->currentPixmap().size() - QSize(20, 20);
	QPixmap pm(s);
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
	QFont f(qApp->font());
	f.setWeight(QFont::Bold);
	p.setFont(f);
	QFontMetrics fm(f);
	int adjustment = fm.height() / 2;
	QRect r = pm.rect();
	r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
	QRect outerRect = p.boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, m_labelText);
	r = p.boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, m_labelText);
	r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
	QPainterPath pp;
	pp.addRoundedRect(r, 5, 5);
	p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
	p.setPen(QPen(QColor(255, 255, 255, 255)));
	p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, m_labelText);
	p.end();

	QPainter painter(this);
	painter.drawPixmap((width() - pm.width()) / 2, (height() - pm.height()) / 2, pm);
	painter.end();
}
