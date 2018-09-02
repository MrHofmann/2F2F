#include "renderarea.h"

#include <iostream>


RenderArea::RenderArea(QWidget *parent, FilterState *filter_state)
    : QWidget(parent), _filter_state(filter_state)
{
    _penColor = qvariant_cast<QColor>("black");
    _penWidth = 3;
//    rotationAngle = 0;
    setBackgroundRole(QPalette::Base);
}

RenderArea::RenderArea(const QPainterPath &path, QWidget *parent)
    : QWidget(parent), _path(path)
{
    _penColor = qvariant_cast<QColor>("cornsilk");
    _penWidth = 3;
    _path.moveTo(0, 0);
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



void RenderArea::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
//    painter.scale(width() / 100.0, height() / 100.0);
//    painter.translate(50.0, 50.0);
//    painter.rotate(-rotationAngle);
//    painter.translate(-50.0, -50.0);
//    painter.drawRect(0, 0, width()-1, height()-1);
//    QColor color1 = qvariant_cast<QColor>("mediumslateblue");

    _penColor = qvariant_cast<QColor>("cornsilk");
    painter.setPen(QPen(_penColor, _penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QLinearGradient gradient(0, 0, 0, 100);
    gradient.setColorAt(0.0, _fillColor1);
    gradient.setColorAt(1.0, _fillColor2);
    painter.setBrush(gradient);


    if(_filter_state->filter_enabled == true && _filter_state->filters.empty() == false)
    {
        Filter *filter = _filter_state->filters[0];
        std::vector<double> kernel = filter->kernel();
        double w = _filter_state->width;
        double lx = _location.x();
        double c1 = lx + w/2;
        double c2 = lx - w/2;

        _filter_state->cutoff1 = locationToFrequency(QPoint(c1, _location.y()));
        _filter_state->cutoff2 = locationToFrequency(QPoint(c2, _location.y()));
        filter->update_kernel();

        std::vector<QPoint> responsePoints;
        for(unsigned i=0; i<kernel.size()/2; i++)
        {
            double freq_max = filter->sample_rate()/2;
            double freq = i*(freq_max*2/(kernel.size()));

//            double x = std::log(i+1)*width()/std::log(kernel.size()/2);
//            double x = i*width()/(kernel.size()/2);
            double x = freq*width()/freq_max;
            double y = height() - (kernel[i]*height())/(3.0);

            QPoint p(x, y);
            responsePoints.push_back(p);
//            painter.drawPoint(x, y);
        }

        std::vector<QPainterPath> responsePath;
        for(unsigned i=0; i<responsePoints.size()-4; i+=5)
        {
            QPoint start = responsePoints[i];
            QPoint mid = responsePoints[i+2];
            QPoint end = responsePoints[i+4];
            QPoint c1 = (start+mid)/2;
            QPoint c2 = (mid+end)/2;

            QPainterPath bezierPath;
            bezierPath.moveTo(start);
            bezierPath.cubicTo(c1, c2, end);
            painter.drawPath(bezierPath);
        }

    }
}

void RenderArea::mousePressEvent(QMouseEvent *event)
{
    if(_filter_state->filter_enabled)
    {
        QPoint p = mapFromGlobal(QCursor::pos());
        _location = p;

        double x = locationToFrequency(p);
        double y = locationToVolume(p);
        double w = _filter_state->width;
        double c1 = locationToFrequency(QPoint(p.x()+w/2, p.y()));
        double c2 = locationToFrequency(QPoint(p.x()-w/2, p.y()));

        _filter_state->cutoff = x;
        _filter_state->dc_gain = y;
        _filter_state->cutoff1 = c1;
        _filter_state->cutoff2 = c2;

        std::cout << "MOVE: " << x << ", " << y << std::endl;

        unsigned *order = &_filter_state->order;
        double *cutoff = &_filter_state->cutoff;
        double *width = &_filter_state->width;
        double *gain = &_filter_state->dc_gain;

        double *cutoff1 = &_filter_state->cutoff1;
        double *cutoff2 = &_filter_state->cutoff2;

        Filter *filter;
        switch(_filter_state->filter_type)
        {
            case Filter::LOW_PASS:
                filter = new LowPass(1024, 44100, order, cutoff, gain);
                break;
            case Filter::HIGH_PASS:
                filter = new HighPass(1024, 44100, order, cutoff, gain);
                break;
            case Filter::BAND_PASS:
                filter = new BandPass(1024, 44100, order, cutoff1, cutoff2, width, gain);
                break;
            case Filter::BAND_STOP:
                filter = new BandStop(1024, 44100, order, cutoff1, cutoff2, width, gain);
                break;
            default:
                break;
        }
    //    filter->update_kernel();
        _filter_state->filters.push_back(filter);
        update();
    }
}

void RenderArea::mouseReleaseEvent(QMouseEvent *event)
{
    if(_filter_state->filter_enabled)
    {
        double x = locationToFrequency(mapFromGlobal(QCursor::pos()));
        std::cout << "RELEASE: " << x << ", " << _filter_state->cutoff << std::endl;
        update();
    }
}

void RenderArea::mouseMoveEvent(QMouseEvent *event)
{
    if(_filter_state->filter_enabled)
    {
        QPoint p = mapFromGlobal(QCursor::pos());
        _location = p;

        double x = locationToFrequency(p);
        double y = locationToVolume(p);
        double w = _filter_state->width;
        double c1 = locationToFrequency(QPoint(p.x()+w/2, p.y()));
        double c2 = locationToFrequency(QPoint(p.x()-w/2, p.y()));

        _filter_state->cutoff = x;
        _filter_state->dc_gain = y;
        _filter_state->cutoff1 = c1;
        _filter_state->cutoff2 = c2;

        std::cout << "MOVE: " << x << ", " << y << std::endl;

        unsigned *order = &_filter_state->order;
        double *cutoff = &_filter_state->cutoff;
        double *width = &_filter_state->width;
        double *gain = &_filter_state->dc_gain;

        double *cutoff1 = &_filter_state->cutoff1;
        double *cutoff2 = &_filter_state->cutoff2;

        Filter *filter;
        switch(_filter_state->filter_type)
        {
            case Filter::LOW_PASS:
                filter = new LowPass(1024, 44100, order, cutoff, gain);
                break;
            case Filter::HIGH_PASS:
                filter = new HighPass(1024, 44100, order, cutoff, gain);
                break;
            case Filter::BAND_PASS:
                filter = new BandPass(1024, 44100, order, cutoff1, cutoff2, width, gain);
                break;
            case Filter::BAND_STOP:
                filter = new BandStop(1024, 44100, order, cutoff1, cutoff2, width, gain);
                break;
            default:
                break;
        }
    //    filter->update_kernel();
        _filter_state->filters.push_back(filter);
        update();
    }
}

double RenderArea::locationToFrequency(QPoint p) const
{
    return (std::exp2(p.x()/100.0)-1)*(20000.0)/(std::exp2(width()/100.0)-1);
}

double RenderArea::locationToVolume(QPoint p) const
{
    return 3.0 - 3.0*p.y()/height();
}


//void RenderArea::setRotationAngle(int degrees)
//{
//    _rotationAngle = degrees;
//    update();
//}
