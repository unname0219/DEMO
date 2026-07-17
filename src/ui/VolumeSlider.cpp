#include "ui/VolumeSlider.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QFont>

VolumeSlider::VolumeSlider(QWidget* parent)
    : QWidget(parent)
    , m_muteBtn(nullptr)
    , m_volumeSlider(nullptr)
    , m_volume(70)
    , m_isMuted(false)
{
    setFixedHeight(DPIAdapter::scaledSize(40));
    setupUI();
}

VolumeSlider::~VolumeSlider()
{
}

void VolumeSlider::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(DPIAdapter::scaledSize(4));

    m_muteBtn = new QPushButton("🔊", this);
    m_muteBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    QFont btnFont = m_muteBtn->font();
    btnFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    m_muteBtn->setFont(btnFont);
    connect(m_muteBtn, &QPushButton::clicked, this, &VolumeSlider::onMuteBtnClicked);
    layout->addWidget(m_muteBtn);

    m_volumeSlider = new QSlider(Qt::Vertical, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedHeight(DPIAdapter::scaledSize(100));
    m_volumeSlider->setTickPosition(QSlider::NoTicks);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VolumeSlider::onSliderValueChanged);
    layout->addWidget(m_volumeSlider);

    setFixedWidth(DPIAdapter::scaledSize(48));
}

void VolumeSlider::onSliderValueChanged(int value)
{
    m_volume = value;
    updateMuteIcon();
    emit volumeChanged(value);
}

void VolumeSlider::onMuteBtnClicked()
{
    emit muteToggled();
}

void VolumeSlider::onVolumeChanged(int volume)
{
    m_volume = volume;
    m_volumeSlider->blockSignals(true);
    m_volumeSlider->setValue(volume);
    m_volumeSlider->blockSignals(false);
    updateMuteIcon();
}

void VolumeSlider::onMutedChanged(bool muted)
{
    m_isMuted = muted;
    updateMuteIcon();
}

void VolumeSlider::updateMuteIcon()
{
    if (m_isMuted || m_volume == 0) {
        m_muteBtn->setText("🔇");
    } else if (m_volume < 50) {
        m_muteBtn->setText("🔉");
    } else {
        m_muteBtn->setText("🔊");
    }
}
