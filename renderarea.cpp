#include "renderarea.h"

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    QColor color = qvariant_cast<QColor>("mediumslateblue");
    _penColor = color;
    _penWidth = 2;
//    rotationAngle = 0;
    setBackgroundRole(QPalette::Base);
}

RenderArea::RenderArea(const QPainterPath &path, QWidget *parent)
    : QWidget(parent), _path(path)
{
    QColor color = qvariant_cast<QColor>("mediumslateblue");
    _penColor = color;
    _penWidth = 2;
    _path.moveTo(0, height()/2);
//    rotationAngle = 0;
    setBackgroundRole(QPalette::Base);
}

QSize RenderArea::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize RenderArea::sizeHint() const
{
    return QSize(100, 100);
}


void RenderArea::setPenWidth(int width)
{
    _penWidth = width;
    update();
}

void RenderArea::setPenColor(const QColor &color)
{
    _penColor = color;
    update();
}

void RenderArea::setFillRule(Qt::FillRule rule)
{
    _path.setFillRule(rule);
    update();
}

void RenderArea::setFillGradient(const QColor &color1, const QColor &color2)
{
    _fillColor1 = color1;
    _fillColor2 = color2;
    update();
}

//void RenderArea::setRotationAngle(int degrees)
//{
//    _rotationAngle = degrees;
//    update();
//}

void RenderArea::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//    painter.scale(width() / 100.0, height() / 100.0);
//    painter.translate(50.0, 50.0);
//    painter.rotate(-rotationAngle);
//    painter.translate(-50.0, -50.0);
//    painter.drawRect(0, 0, width()-1, height()-1);

    painter.setPen(QPen(_penColor, _penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QLinearGradient gradient(0, 0, 0, 100);
    gradient.setColorAt(0.0, _fillColor1);
    gradient.setColorAt(1.0, _fillColor2);
    painter.setBrush(gradient);

    painter.drawPath(_path);

//    for(unsigned i=0; i<width(); i++)
//    {
//        painter.drawPoint(i, height()/2*(sin((double)i/100*2*3.1415926))+height()/2);
//    }
}
