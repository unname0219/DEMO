#include "ui/MediaViewer.h"
#include "ui/VideoPlayer.h"
#include "ui/ImageViewer.h"
#include "ui/AudioPlayer.h"
#include "core/PlayerController.h"
#include "managers/DPIAdapter.h"
#include <QLabel>
#include <QVideoWidget>
#include <QVBoxLayout>

MediaViewer::MediaViewer(QWidget* parent)
    : QWidget(parent)
    , m_stackedLayout(nullptr)
    , m_videoPlayer(nullptr)
    , m_imageViewer(nullptr)
    , m_audioPlayer(nullptr)
    , m_placeholderLabel(nullptr)
    , m_currentType(MediaType::Unknown)
{
    setupUI();
}

MediaViewer::~MediaViewer()
{
}

void MediaViewer::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 1, 0);

    m_stackedLayout = new QStackedLayout();
    mainLayout->addLayout(m_stackedLayout);

    m_placeholderLabel = new QLabel(this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setText("拖拽文件到此处或点击打开文件");
    QFont font = m_placeholderLabel->font();
    font.setPointSizeF(DPIAdapter::scaledFontSize(11));
    m_placeholderLabel->setFont(font);
    m_placeholderLabel->setStyleSheet("color: #888888;");
    m_stackedLayout->addWidget(m_placeholderLabel);

    m_videoPlayer = new VideoPlayer(this);
    m_stackedLayout->addWidget(m_videoPlayer);

    m_imageViewer = new ImageViewer(this);
    m_stackedLayout->addWidget(m_imageViewer);

    m_audioPlayer = new AudioPlayer(this);
    m_stackedLayout->addWidget(m_audioPlayer);

    m_stackedLayout->setCurrentWidget(m_placeholderLabel);
}

void MediaViewer::showMedia(const QString& filePath, MediaType type, PlayerController* controller)
{
    m_currentType = type;
    switch (type) {
    case MediaType::Video:
        m_videoPlayer->setMediaPlayer(controller);
        controller->openFile(filePath);
        controller->play();
        m_stackedLayout->setCurrentWidget(m_videoPlayer);
        break;
    case MediaType::Audio:
        m_audioPlayer->setMediaPlayer(controller);
        connect(controller, &PlayerController::playbackStateChanged,
                m_audioPlayer, [this](PlaybackState state) {
                    m_audioPlayer->onPlaybackStateChanged(state == PlaybackState::Playing);
                }, Qt::UniqueConnection);
        controller->openFile(filePath);
        controller->play();
        m_stackedLayout->setCurrentWidget(m_audioPlayer);
        break;
    case MediaType::Image:
        m_imageViewer->loadImage(filePath);
        m_stackedLayout->setCurrentWidget(m_imageViewer);
        break;
    default:
        m_stackedLayout->setCurrentWidget(m_placeholderLabel);
        break;
    }
}

MediaType MediaViewer::currentMediaType() const
{
    return m_currentType;
}

void MediaViewer::setSmoothScaling(bool smooth)
{
    if (m_imageViewer) {
        m_imageViewer->setSmoothScaling(smooth);
    }
}

void MediaViewer::setVideoScalingMode(Qt::AspectRatioMode mode)
{
    if (m_videoPlayer) {
        m_videoPlayer->setVideoScalingMode(mode);
    }
}

QWidget* MediaViewer::videoWidget() const
{
    if (m_videoPlayer && m_videoPlayer->videoWidget()) {
        return m_videoPlayer->videoWidget();
    }
    return nullptr;
}

ImageViewer* MediaViewer::imageViewer() const
{
    return m_imageViewer;
}

AudioPlayer* MediaViewer::audioPlayer() const
{
    return m_audioPlayer;
}

void MediaViewer::clear()
{
    m_currentType = MediaType::Unknown;
    m_stackedLayout->setCurrentWidget(m_placeholderLabel);
}
