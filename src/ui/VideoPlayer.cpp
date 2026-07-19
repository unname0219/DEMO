#include "ui/VideoPlayer.h"
#include "core/PlayerController.h"
#include "managers/DPIAdapter.h"

#ifdef FFMPEG_ENABLED
#include "ffmpeg/FFmpegVideoRenderer.h"
#include "ffmpeg/FFmpegPlayer.h"
#endif

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(QWidget* parent)
    : QWidget(parent)
    , m_videoWidget(nullptr)
    , m_stackedLayout(nullptr)
    , m_ffmpegRenderer(nullptr)
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
    m_stackedLayout = new QStackedLayout(this);
    m_stackedLayout->setContentsMargins(0, 0, 0, 0);

    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setStyleSheet("background-color: #1a1a1a;");
    m_videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_videoWidget->setMouseTracking(true);
    // 关键：让QVideoWidget不使用原生窗口，避免在全屏下盖住浮动控件
    m_videoWidget->setAttribute(Qt::WA_DontCreateNativeAncestors);
    m_videoWidget->setAttribute(Qt::WA_NativeWindow, false);
    m_stackedLayout->addWidget(m_videoWidget);

#ifdef FFMPEG_ENABLED
    m_ffmpegRenderer = new FFmpegVideoRenderer(this);
    m_ffmpegRenderer->setMouseTracking(true);
    m_stackedLayout->addWidget(m_ffmpegRenderer);
#endif

    m_stackedLayout->setCurrentIndex(0);
    setMouseTracking(true);
}

void VideoPlayer::setMediaPlayer(PlayerController* controller)
{
    if (!controller) return;
    m_controller = controller;
    m_controller->mediaPlayer()->setVideoOutput(m_videoWidget);

    connect(m_controller, &PlayerController::decoderBackendChanged,
            this, &VideoPlayer::onDecoderBackendChanged);

    updateActiveRenderer();
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

void VideoPlayer::onDecoderBackendChanged()
{
    updateActiveRenderer();
}

void VideoPlayer::updateActiveRenderer()
{
    if (!m_controller) return;

#ifdef FFMPEG_ENABLED
    if (m_controller->decoderBackend() != DecoderBackend::QtMultimedia
        && m_controller->ffmpegPlayer()
        && m_ffmpegRenderer) {
        m_controller->mediaPlayer()->setVideoOutput(nullptr);
        m_ffmpegRenderer->setDecoder(m_controller->ffmpegPlayer()->decoder());
        m_stackedLayout->setCurrentWidget(m_ffmpegRenderer);
        return;
    }
#endif

    m_controller->mediaPlayer()->setVideoOutput(m_videoWidget);
    m_stackedLayout->setCurrentWidget(m_videoWidget);
}
