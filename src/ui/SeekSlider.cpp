#include "ui/SeekSlider.h"

void SeekSlider::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int newVal;
        if (orientation() == Qt::Horizontal) {
            qreal ratio = static_cast<qreal>(event->position().x()) / width();
            newVal = minimum() + static_cast<int>(ratio * (maximum() - minimum()));
        } else {
            qreal ratio = static_cast<qreal>(event->position().y()) / height();
            newVal = minimum() + static_cast<int>(ratio * (maximum() - minimum()));
        }
        newVal = qBound(minimum(), newVal, maximum());
        setValue(newVal);
        emit sliderMoved(newVal);
        event->accept();
        return;
    }
    QSlider::mousePressEvent(event);
}

void SeekSlider::mouseMoveEvent(QMouseEvent* event)
{
    QSlider::mouseMoveEvent(event);
}

void SeekSlider::mouseReleaseEvent(QMouseEvent* event)
{
    QSlider::mouseReleaseEvent(event);
}
