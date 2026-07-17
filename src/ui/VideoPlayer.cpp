#include "ui/VideoPlayer.h"
#include "core/PlayerController.h"
#include <QVBoxLayout>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget* parent)
    : QWidget(parent)
    , m_videoWidget(nullptr)
    , m_controller(nullptr)
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

    m_videoWidget = new QVideoWidget(this);
    layout->addWidget(m_videoWidget);
}

void VideoPlayer::setMediaPlayer(PlayerController* controller)
{
    if (!controller) return;
    m_controller = controller;
    m_controller->mediaPlayer()->setVideoOutput(m_videoWidget);
}
