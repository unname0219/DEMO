#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <QSlider>
#include <QMouseEvent>

class SeekSlider : public QSlider
{
    Q_OBJECT

public:
    explicit SeekSlider(Qt::Orientation orientation, QWidget* parent = nullptr)
        : QSlider(orientation, parent) {}

    explicit SeekSlider(QWidget* parent = nullptr)
        : QSlider(Qt::Horizontal, parent) {}

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif
