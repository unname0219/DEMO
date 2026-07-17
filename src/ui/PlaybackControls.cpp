#include "ui/PlaybackControls.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QFont>

PlaybackControls::PlaybackControls(QWidget* parent)
    : QWidget(parent)
    , m_playPauseBtn(nullptr)
    , m_prevBtn(nullptr)
    , m_nextBtn(nullptr)
    , m_state(PlaybackState::Stopped)
{
    setupUI();
}

PlaybackControls::~PlaybackControls()
{
}

void PlaybackControls::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(DPIAdapter::scaledSize(4));

    m_prevBtn = new QPushButton("⏮", this);
    m_prevBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    QFont btnFont = m_prevBtn->font();
    btnFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    m_prevBtn->setFont(btnFont);
    connect(m_prevBtn, &QPushButton::clicked, this, &PlaybackControls::previousClicked);
    layout->addWidget(m_prevBtn);

    m_playPauseBtn = new QPushButton("▶", this);
    m_playPauseBtn->setFixedSize(DPIAdapter::scaledSize(44), DPIAdapter::scaledSize(44));
    m_playPauseBtn->setFont(btnFont);
    m_playPauseBtn->setStyleSheet(
        "QPushButton { background-color: #00D4AA; color: white; border-radius: 22px; }"
        "QPushButton:hover { background-color: #00F0C0; }"
        "QPushButton:pressed { background-color: #00B090; }"
    );
    connect(m_playPauseBtn, &QPushButton::clicked, this, &PlaybackControls::playToggled);
    layout->addWidget(m_playPauseBtn);

    m_nextBtn = new QPushButton("⏭", this);
    m_nextBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    m_nextBtn->setFont(btnFont);
    connect(m_nextBtn, &QPushButton::clicked, this, &PlaybackControls::nextClicked);
    layout->addWidget(m_nextBtn);
}

void PlaybackControls::onPlaybackStateChanged(PlaybackState state)
{
    m_state = state;
    if (state == PlaybackState::Playing) {
        m_playPauseBtn->setText("⏸");
    } else {
        m_playPauseBtn->setText("▶");
    }
}
