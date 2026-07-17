#include "ui/ProgressBar.h"
#include "managers/DPIAdapter.h"
#include "utils/FormatUtils.h"
#include <QHBoxLayout>
#include <QFont>

ProgressBar::ProgressBar(QWidget* parent)
    : QWidget(parent)
    , m_timeLabel(nullptr)
    , m_durationLabel(nullptr)
    , m_slider(nullptr)
    , m_position(0)
    , m_duration(0)
    , m_isSeeking(false)
{
    setFixedHeight(DPIAdapter::scaledSize(32));
    setupUI();
}

ProgressBar::~ProgressBar()
{
}

void ProgressBar::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(12), 0);
    layout->setSpacing(DPIAdapter::scaledSize(8));

    m_timeLabel = new QLabel("00:00", this);
    QFont timeFont = m_timeLabel->font();
    timeFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setFixedWidth(DPIAdapter::scaledSize(56));
    m_timeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_timeLabel);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setRange(0, 100);
    m_slider->setTracking(false);
    connect(m_slider, &QSlider::sliderMoved, this, &ProgressBar::onSliderMoved);
    connect(m_slider, &QSlider::sliderReleased, this, &ProgressBar::onSliderReleased);
    connect(m_slider, &QSlider::actionTriggered, this, [this](int action) {
        Q_UNUSED(action);
        m_isSeeking = true;
    });
    layout->addWidget(m_slider, 1);

    m_durationLabel = new QLabel("00:00", this);
    m_durationLabel->setFont(timeFont);
    m_durationLabel->setFixedWidth(DPIAdapter::scaledSize(56));
    m_durationLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_durationLabel);
}

void ProgressBar::onPositionChanged(qint64 position)
{
    m_position = position;
    if (!m_isSeeking) {
        if (m_duration > 0) {
            m_slider->blockSignals(true);
            m_slider->setValue(static_cast<int>((position * 1000) / m_duration));
            m_slider->blockSignals(false);
        }
        m_timeLabel->setText(FormatUtils::formatTime(position));
    }
}

void ProgressBar::onDurationChanged(qint64 duration)
{
    m_duration = duration;
    m_durationLabel->setText(FormatUtils::formatTime(duration));
    m_slider->setRange(0, 1000);
}

void ProgressBar::onSliderMoved(int value)
{
    m_isSeeking = true;
    if (m_duration > 0) {
        qint64 newPos = (value * m_duration) / 1000;
        m_timeLabel->setText(FormatUtils::formatTime(newPos));
    }
}

void ProgressBar::onSliderReleased()
{
    m_isSeeking = false;
    if (m_duration > 0) {
        qint64 newPos = (m_slider->value() * m_duration) / 1000;
        emit seekRequested(newPos);
    }
}

void ProgressBar::updatePositionLabel()
{
    m_timeLabel->setText(FormatUtils::formatTime(m_position));
}
