#include "ui/PlaybackControls.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QFont>

PlaybackControls::PlaybackControls(QWidget* parent)
    : QWidget(parent)
    , m_playPauseBtn(nullptr)
    , m_prevBtn(nullptr)
    , m_nextBtn(nullptr)
    , m_rewindBtn(nullptr)
    , m_forwardBtn(nullptr)
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
    layout->setSpacing(DPIAdapter::scaledSize(6));

    QFont btnFont;
    btnFont.setPointSizeF(DPIAdapter::scaledFontSize(11));

    // 上一个视频
    m_prevBtn = new QPushButton("⏮", this);
    m_prevBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_prevBtn->setFont(btnFont);
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setToolTip("上一个");
    connect(m_prevBtn, &QPushButton::clicked, this, &PlaybackControls::previousClicked);
    layout->addWidget(m_prevBtn);

    // 快退
    m_rewindBtn = new QPushButton("«", this);
    m_rewindBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_rewindBtn->setFont(btnFont);
    m_rewindBtn->setCursor(Qt::PointingHandCursor);
    m_rewindBtn->setToolTip("快退 10 秒");
    connect(m_rewindBtn, &QPushButton::clicked, this, &PlaybackControls::rewindClicked);
    layout->addWidget(m_rewindBtn);

    // 播放/暂停（居中）
    m_playPauseBtn = new QPushButton("▶", this);
    m_playPauseBtn->setFixedSize(DPIAdapter::scaledSize(40), DPIAdapter::scaledSize(40));
    QFont playFont = btnFont;
    playFont.setPointSizeF(DPIAdapter::scaledFontSize(13));
    m_playPauseBtn->setFont(playFont);
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);
    m_playPauseBtn->setStyleSheet(
        "QPushButton { background-color: #00D4AA; color: white; border-radius: 20px; }"
        "QPushButton:hover { background-color: #00F0C0; }"
        "QPushButton:pressed { background-color: #00B090; }"
    );
    connect(m_playPauseBtn, &QPushButton::clicked, this, &PlaybackControls::playToggled);
    layout->addWidget(m_playPauseBtn);

    // 快进
    m_forwardBtn = new QPushButton("»", this);
    m_forwardBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_forwardBtn->setFont(btnFont);
    m_forwardBtn->setCursor(Qt::PointingHandCursor);
    m_forwardBtn->setToolTip("快进 10 秒");
    connect(m_forwardBtn, &QPushButton::clicked, this, &PlaybackControls::forwardClicked);
    layout->addWidget(m_forwardBtn);

    // 下一个视频
    m_nextBtn = new QPushButton("⏭", this);
    m_nextBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_nextBtn->setFont(btnFont);
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setToolTip("下一个");
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
