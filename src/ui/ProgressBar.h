#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QLabel>
#include "ui/SeekSlider.h"

class ProgressBar : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressBar(QWidget* parent = nullptr);
    ~ProgressBar();

signals:
    void seekRequested(qint64 position);

public slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);

private slots:
    void onSliderMoved(int value);
    void onSliderReleased();

private:
    void setupUI();
    void updatePositionLabel();

    QLabel* m_timeLabel;
    QLabel* m_durationLabel;
    SeekSlider* m_slider;

    qint64 m_position;
    qint64 m_duration;
    bool m_isSeeking;
};

#endif
