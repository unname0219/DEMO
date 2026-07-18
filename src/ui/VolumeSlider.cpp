#include "ui/VolumeSlider.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include "managers/ThemeManager.h"
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
    updateMuteIcon();
}

VolumeSlider::~VolumeSlider()
{
}

void VolumeSlider::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(DPIAdapter::scaledSize(4));

    m_muteBtn = new QPushButton(this);
    m_muteBtn->setFixedSize(DPIAdapter::scaledSize(28), DPIAdapter::scaledSize(28));
    m_muteBtn->setIconSize(QSize(DPIAdapter::scaledSize(18), DPIAdapter::scaledSize(18)));
    m_muteBtn->setCursor(Qt::PointingHandCursor);
    connect(m_muteBtn, &QPushButton::clicked, this, &VolumeSlider::onMuteBtnClicked);
    layout->addWidget(m_muteBtn);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(70);
    m_volumeSlider->setFixedWidth(DPIAdapter::scaledSize(80));
    m_volumeSlider->setTickPosition(QSlider::NoTicks);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VolumeSlider::onSliderValueChanged);
    layout->addWidget(m_volumeSlider);

    setFixedWidth(DPIAdapter::scaledSize(120));

    // 主题变化时刷新图标
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &VolumeSlider::updateMuteIcon);
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
    // 用户只提供了"声音开"和"声音关"两个图标：
    // - 静音或音量为 0 → volume-mute
    // - 否则 → volume
    if (m_isMuted || m_volume == 0) {
        m_muteBtn->setIcon(IconManager::instance()->icon("volume-mute"));
    } else {
        m_muteBtn->setIcon(IconManager::instance()->icon("volume"));
    }
}
