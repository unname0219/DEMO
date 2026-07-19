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
        setSliderDown(true);
        emit sliderPressed();
        emit sliderMoved(newVal);
        event->accept();
        return;
    }
    QSlider::mousePressEvent(event);
}

void SeekSlider::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::LeftButton) {
        int newVal;
        if (orientation() == Qt::Horizontal) {
            qreal ratio = static_cast<qreal>(event->position().x()) / width();
            newVal = minimum() + static_cast<int>(ratio * (maximum() - minimum()));
        } else {
            qreal ratio = static_cast<qreal>(event->position().y()) / height();
            newVal = minimum() + static_cast<int>(ratio * (maximum() - minimum()));
        }
        newVal = qBound(minimum(), newVal, maximum());
        if (newVal != value()) {
            setValue(newVal);
            emit sliderMoved(newVal);
        }
        event->accept();
        return;
    }
    QSlider::mouseMoveEvent(event);
}

void SeekSlider::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        setSliderDown(false);
        emit sliderReleased();
        event->accept();
        return;
    }
    QSlider::mouseReleaseEvent(event);
}
