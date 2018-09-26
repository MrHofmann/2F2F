#include "renderarea.h"

#include <iostream>


RenderArea::RenderArea(QWidget *parent, FilterState *filter_state)
    : QWidget(parent), _filter_state(filter_state)
{
    _penColor = qvariant_cast<QColor>("black");
    _penWidth = 3;
//    rotationAngle = 0;
    setBackgroundRole(QPalette::Base);
    _draw_filter = false;
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

void RenderArea::setDrawFilter(bool draw)
{
    _draw_filter = draw;
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

    _penColor = qvariant_cast<QColor>("cornsilk");
    painter.setPen(QPen(_penColor, _penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QLinearGradient gradient(0, 0, 0, 100);
    gradient.setColorAt(0.0, _fillColor1);
    gradient.setColorAt(1.0, _fillColor2);
    painter.setBrush(gradient);

    drawFrequencies(&painter);
    if(_draw_filter == true)
    {
        switch(_filter_state->filter_type){
        case Filter::EQUALIZER:
            break;
        case Filter::LOW_PASS:
            drawLowPass(&painter);
            break;
        case Filter::HIGH_PASS:
            drawHighPass(&painter);
            break;
        case Filter::BAND_PASS:
//            drawBandPass(&painter);
//            break;
        case Filter::BAND_STOP:
//            drawBandStop(&painter);
//            break;
            drawFilter(&painter);
            break;
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
        _filter_state->cutoff1 = std::min(c1, 20000.0);
        _filter_state->cutoff2 = std::max(c2, 20.0);

        std::cout << "PRESS: " << x << ", " << y << std::endl;

        unsigned order = _filter_state->order;
        double cutoff = _filter_state->cutoff;
        double gain = _filter_state->dc_gain;

        double cutoff1 = _filter_state->cutoff1;
        double cutoff2 = _filter_state->cutoff2;

        Filter *filter = _filter_state->filters[0];
        filter->update_params(order, cutoff, cutoff1, cutoff2, gain);
        filter->update_kernel();
        _filter_state->filters.push_back(filter);
        _filter_state->filter_method = Filter::OA_FFT;
        _filter_state->filter_initialized = true;

        setDrawFilter(true);
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
        _filter_state->cutoff1 = std::min(c1, 20000.0);
        _filter_state->cutoff2 = std::max(c2, 20.0);

        unsigned order = _filter_state->order;
        double cutoff = _filter_state->cutoff;
        double gain = _filter_state->dc_gain;

        double cutoff1 = _filter_state->cutoff1;
        double cutoff2 = _filter_state->cutoff2;


        std::cout << "MOVE1: " << x << ", " << y << std::endl;

        Filter *filter = _filter_state->filters[0];
        filter->update_params(order, cutoff, cutoff1, cutoff2, gain);
        filter->update_kernel();
        _filter_state->filter_initialized = true;
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


double RenderArea::locationToFrequency(QPoint p) const
{
    if(p.x() < 0)
        return 0.0;
    else
        return (std::exp2(p.x()/100.0)-1)*(20000.0)/(std::exp2(width()/100.0)-1);
}

double RenderArea::locationToVolume(QPoint p) const
{
    if(p.y() < 0)
        return 3.0;
    if (p.y() > height()+1)
        return 0.0;
    else
        return 3.0 - 3.0*std::log2(p.y()+1)/std::log2(height()+1);
    //        return std::pow(10, (-48+(54.0*(height()-p.y()))/height())/20);
}

double RenderArea::frequencyToLocation(double f) const
{
    //[A, B] --> [a, b]: (val-A)*(b-a)/(B-A) + a
    //[0,20000] --> [0, width()]
    double sample_rate = _filter_state->filters[0]->sample_rate();
    double freq_max = sample_rate*1.0;

    return f*width()/freq_max;
}

QColor RenderArea::intensityToColor(std::complex<double> i) const
{
    return QColor(qvariant_cast<QColor>("red"));
}


void RenderArea::drawLowPass(QPainter *painter) const
{
    Filter *filter = _filter_state->filters[0];
    std::vector<double> kernel = filter->kernel();
    double ly = _location.y();

    double order = _filter_state->order;
    double cutoff = _filter_state->cutoff;

    unsigned num_bins = width();
    double bin_width = filter->sample_rate()/(2.0*num_bins);

    std::vector<QPoint> responsePoints(num_bins);
    for(unsigned i=0; i<num_bins; i++)
    {
//        double freq_max = filter->sample_rate()/2;
//        double freq = i*(freq_max*2/num_bins);
//        double x = freq*width()/freq_max;
//        double y = height() - (kernel[i]*height())/(3.0);

        double freq = i*bin_width;
        double gain = (height()-ly)/std::sqrt(1 + std::pow(freq/cutoff, 2.0*order));

        QPoint p(i, height()-gain);
        responsePoints[i] = p;
//        responsePoints[i] = QPoint(freq, gain);
    }

    std::vector<QPainterPath> responsePath;
    for(unsigned i=0; i<responsePoints.size()-2; i+=2)
    {
        QPoint start = responsePoints[i];
        QPoint mid = responsePoints[i+1];
        QPoint end = responsePoints[i+2];
        QPoint c1 = (start+mid)/2;
        QPoint c2 = (mid+end)/2;

        QPainterPath bezierPath;
        bezierPath.moveTo(start);
        bezierPath.cubicTo(c1, c2, end);
        painter->drawPath(bezierPath);
    }
}

void RenderArea::drawHighPass(QPainter *painter) const
{
    Filter *filter = _filter_state->filters[0];
    double ly = _location.y();

    double order = _filter_state->order;
    double cutoff = _filter_state->cutoff;

    unsigned num_bins = width();
    double bin_width = filter->sample_rate()/(2.0*num_bins);

    std::vector<QPoint> responsePoints(num_bins);
    for(unsigned i=0; i<num_bins; i++)
    {
        double freq = i*bin_width;
        double gain = (height()-ly)/std::sqrt(1 + std::pow(cutoff/freq, 2.0*order));

        QPoint p(i, height()-gain);
        responsePoints[i] = p;
    }

    std::vector<QPainterPath> responsePath;
    for(unsigned i=0; i<responsePoints.size()-2; i+=2)
    {
        QPoint start = responsePoints[i];
        QPoint mid = responsePoints[i+1];
        QPoint end = responsePoints[i+2];
        QPoint c1 = (start+mid)/2;
        QPoint c2 = (mid+end)/2;

        QPainterPath bezierPath;
        bezierPath.moveTo(start);
        bezierPath.cubicTo(c1, c2, end);
        painter->drawPath(bezierPath);
    }
}

void RenderArea::drawBandPass(QPainter *painter) const
{
    Filter *filter = _filter_state->filters[0];
    double w = _filter_state->width;
    double lx = _location.x();
    double ly = _location.y();
    double c1 = lx + w/2;
    double c2 = lx - w/2;

    std::cout << w << " " << c1 << " " << c2 << std::endl;

    _filter_state->cutoff1 = locationToFrequency(QPoint(c1, _location.y()));
    _filter_state->cutoff2 = locationToFrequency(QPoint(c2, _location.y()));
    filter->update_kernel();

    double order = _filter_state->order;
    double cutoff1 = _filter_state->cutoff1;
    double cutoff2 = _filter_state->cutoff2;

    unsigned num_bins = width();
    double bin_width = filter->sample_rate()/(2.0*num_bins);

    std::vector<QPoint> responsePoints(num_bins);
    for(unsigned i=0; i<num_bins; i++)
    {
        double gain = (1/(std::sqrt(2*3.1415926*(w/150.0))))*(1/std::exp(0.5*(i-lx)*(i-lx)/(w/150.0)));
//        std::cout << gain << std::endl;
        QPoint p(i, height()-gain*height());
        responsePoints[i] = p;
    }

    std::vector<QPainterPath> responsePath;
    for(unsigned i=0; i<responsePoints.size()-2; i+=2)
    {
        QPoint start = responsePoints[i];
        QPoint mid = responsePoints[i+1];
        QPoint end = responsePoints[i+2];
        QPoint c1 = (start+mid)/2;
        QPoint c2 = (mid+end)/2;

        QPainterPath bezierPath;
        bezierPath.moveTo(start);
        bezierPath.cubicTo(c1, c2, end);
        painter->drawPath(bezierPath);
    }
}

void RenderArea::drawBandStop(QPainter *painter) const
{

}

void RenderArea::drawFilter(QPainter *painter) const
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
    for(unsigned i=0; i<responsePoints.size()-2; i+=2)
    {
        QPoint start = responsePoints[i];
        QPoint mid = responsePoints[i+1];
        QPoint end = responsePoints[i+2];
        QPoint c1 = (start+mid)/2;
        QPoint c2 = (mid+end)/2;

        QPainterPath bezierPath;
        bezierPath.moveTo(start);
        bezierPath.cubicTo(c1, c2, end);
        painter->drawPath(bezierPath);
    }
}

void RenderArea::drawFrequencies(QPainter *painter) const
{
    std::vector<std::complex<double> > freqs = _filter_state->frequencies;
    std::vector<double> new_freqs(width(), 0);
    std::vector<unsigned> counts(width(), 1);
    for(unsigned i=0; i<freqs.size(); i++)
    {
        double sample_rate = _filter_state->filters[0]->sample_rate();
        double freq_max = sample_rate*1.0;
        double freq = i*(sample_rate/(freqs.size()*1.0));

        unsigned l = frequencyToLocation(freq);
        new_freqs[l] += std::abs(freqs[i]);
        counts[l]++;
    }

    double max_power = new_freqs[0]/counts[0];
    double min_power = 0;
    for(unsigned i=0; i<new_freqs.size(); i++)
    {
        new_freqs[i] /= counts[i];
        if(new_freqs[i] > max_power)
            max_power = new_freqs[i];
//        painter->setPen(intensityToColor(new_freqs[i]));
//        painter->drawLine(i, 0, i, height());
    }
//    std::cout << max_power << " " << std::endl;
}


//void RenderArea::setRotationAngle(int degrees)
//{
//    _rotationAngle = degrees;
//    update();
//}
