#include "ui/VideoPlayer.h"
#include "core/PlayerController.h"
#include <QVBoxLayout>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget* parent)
    : QWidget(parent)
    , m_videoWidget(nullptr)
    , m_controller(nullptr)
    , m_scalingMode(Qt::KeepAspectRatio)
{
    setupUI();
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::setupUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("background-color: #1a1a1a;");
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addWidget(m_videoWidget);
}

void VideoPlayer::setMediaPlayer(PlayerController* controller)
{
    if (!controller) return;
    m_controller = controller;
    m_controller->mediaPlayer()->setVideoOutput(m_videoWidget);
}

void VideoPlayer::setVideoScalingMode(Qt::AspectRatioMode mode)
{
    m_scalingMode = mode;
    if (m_videoWidget) {
        QMediaPlayer* player = m_controller ? m_controller->mediaPlayer() : nullptr;
        if (player && player->videoOutput()) {
            QVideoWidget* vw = qobject_cast<QVideoWidget*>(player->videoOutput());
            if (vw) {
                vw->setAspectRatioMode(mode);
            }
        }
    }
}
