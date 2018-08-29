#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QWidget>
#include <QtGui>

class RenderArea : public QWidget
{
    Q_OBJECT
public:
    explicit RenderArea(const QPainterPath &path, QWidget *parent = nullptr);
    explicit RenderArea(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:

public slots:
    void setPenWidth(int width);
    void setPenColor(const QColor &color);
    void setFillRule(Qt::FillRule rule);
    void setFillGradient(const QColor &color1, const QColor &color2);
//    void setRotationAngle(int degrees);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    QPainterPath _path;
    QColor _penColor;
    int _penWidth;
    QColor _fillColor1;
    QColor _fillColor2;
//    int _rotationAngle;
};

#endif // RENDERAREA_H
