#include "ui/PlaybackControls.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include "managers/ThemeManager.h"
#include <QHBoxLayout>
#include <QFont>
#include <QPushButton>

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
    refreshIcons();
}

PlaybackControls::~PlaybackControls()
{
}

void PlaybackControls::setupUI()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(DPIAdapter::scaledSize(6));

    int smallBtnSize = DPIAdapter::scaledSize(32);
    int playBtnSize = DPIAdapter::scaledSize(40);
    int iconSize = DPIAdapter::scaledSize(20);

    // 上一个视频
    m_prevBtn = new QPushButton(this);
    m_prevBtn->setFixedSize(smallBtnSize, smallBtnSize);
    m_prevBtn->setIconSize(QSize(iconSize, iconSize));
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    m_prevBtn->setToolTip("上一个");
    connect(m_prevBtn, &QPushButton::clicked, this, &PlaybackControls::previousClicked);
    layout->addWidget(m_prevBtn);

    // 快退
    m_rewindBtn = new QPushButton(this);
    m_rewindBtn->setFixedSize(smallBtnSize, smallBtnSize);
    m_rewindBtn->setIconSize(QSize(iconSize, iconSize));
    m_rewindBtn->setCursor(Qt::PointingHandCursor);
    m_rewindBtn->setToolTip("快退 10 秒");
    connect(m_rewindBtn, &QPushButton::clicked, this, &PlaybackControls::rewindClicked);
    layout->addWidget(m_rewindBtn);

    // 播放/暂停（居中，保留原图渐变色，不染色）
    m_playPauseBtn = new QPushButton(this);
    m_playPauseBtn->setFixedSize(playBtnSize, playBtnSize);
    m_playPauseBtn->setIconSize(QSize(playBtnSize - DPIAdapter::scaledSize(10),
                                      playBtnSize - DPIAdapter::scaledSize(10)));
    m_playPauseBtn->setCursor(Qt::PointingHandCursor);
    m_playPauseBtn->setStyleSheet(
        "QPushButton { background-color: rgba(0,212,170,30); border: none; border-radius: 20px; }"
        "QPushButton:hover { background-color: rgba(0,212,170,60); }"
        "QPushButton:pressed { background-color: rgba(0,212,170,90); }"
    );
    connect(m_playPauseBtn, &QPushButton::clicked, this, &PlaybackControls::playToggled);
    layout->addWidget(m_playPauseBtn);

    // 快进
    m_forwardBtn = new QPushButton(this);
    m_forwardBtn->setFixedSize(smallBtnSize, smallBtnSize);
    m_forwardBtn->setIconSize(QSize(iconSize, iconSize));
    m_forwardBtn->setCursor(Qt::PointingHandCursor);
    m_forwardBtn->setToolTip("快进 10 秒");
    connect(m_forwardBtn, &QPushButton::clicked, this, &PlaybackControls::forwardClicked);
    layout->addWidget(m_forwardBtn);

    // 下一个视频
    m_nextBtn = new QPushButton(this);
    m_nextBtn->setFixedSize(smallBtnSize, smallBtnSize);
    m_nextBtn->setIconSize(QSize(iconSize, iconSize));
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    m_nextBtn->setToolTip("下一个");
    connect(m_nextBtn, &QPushButton::clicked, this, &PlaybackControls::nextClicked);
    layout->addWidget(m_nextBtn);

    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &PlaybackControls::refreshIcons);
}

void PlaybackControls::refreshIcons()
{
    // 普通按钮根据主题染色
    m_prevBtn->setIcon(IconManager::instance()->icon("skip-back"));
    m_nextBtn->setIcon(IconManager::instance()->icon("skip-forward"));
    m_rewindBtn->setIcon(IconManager::instance()->icon("rewind"));
    m_forwardBtn->setIcon(IconManager::instance()->icon("forward"));
    // 播放/暂停保留原色渐变，不染色
    if (m_state == PlaybackState::Playing) {
        m_playPauseBtn->setIcon(IconManager::instance()->icon("pause", false));
    } else {
        m_playPauseBtn->setIcon(IconManager::instance()->icon("play", false));
    }
}

void PlaybackControls::onPlaybackStateChanged(PlaybackState state)
{
    m_state = state;
    refreshIcons();
}
