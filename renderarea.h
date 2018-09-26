#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QtGui>

#include "datastructures.h"

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit RenderArea(const QPainterPath &path, QWidget *parent = nullptr);
    explicit RenderArea(QWidget *parent = nullptr, FilterState *filter_state = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setDrawFilter(bool draw);

signals:

public slots:
    void setPenWidth(int width);
    void setPenColor(const QColor &color);
    void setFillRule(Qt::FillRule rule);
    void setFillGradient(const QColor &color1, const QColor &color2);
//    void setRotationAngle(int degrees);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    double locationToFrequency(QPoint p) const;
    double locationToVolume(QPoint p) const;
    double frequencyToLocation(double f) const;
    QColor intensityToColor(std::complex<double> i) const;

    void drawLowPass(QPainter *painter) const;
    void drawHighPass(QPainter *painter) const;
    void drawBandPass(QPainter *painter) const;
    void drawBandStop(QPainter *painter) const;
    void drawFilter(QPainter *painter) const;
    void drawFrequencies(QPainter *painter) const;

private:
    FilterState *_filter_state;
    QPoint _location;
    bool _draw_filter;

    int _penWidth;
    QColor _penColor;
    QColor _fillColor1;
    QColor _fillColor2;
    QPainterPath _path;
//    int _rotationAngle;
};

#endif // RENDERAREA_H
