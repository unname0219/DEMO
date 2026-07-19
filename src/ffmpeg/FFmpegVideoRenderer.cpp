#include "ffmpeg/FFmpegVideoRenderer.h"
#include "ffmpeg/FFmpegDecoder.h"
#include <QPainter>
#include <QResizeEvent>

FFmpegVideoRenderer::FFmpegVideoRenderer(QWidget* parent)
    : QWidget(parent)
    , m_decoder(nullptr)
    , m_updateTimer(nullptr)
    , m_lastPts(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMinimumSize(320, 240);
    setStyleSheet("background-color: black;");

    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(16);
    connect(m_updateTimer, &QTimer::timeout, this, &FFmpegVideoRenderer::updateFrame);
}

FFmpegVideoRenderer::~FFmpegVideoRenderer()
{
}

void FFmpegVideoRenderer::setDecoder(FFmpegDecoder* decoder)
{
    if (m_decoder) {
        disconnect(m_decoder, &FFmpegDecoder::videoFrameReady,
                   this, &FFmpegVideoRenderer::onVideoFrameReady);
    }

    m_decoder = decoder;

    if (m_decoder) {
        connect(m_decoder, &FFmpegDecoder::videoFrameReady,
                this, &FFmpegVideoRenderer::onVideoFrameReady);
    }

    m_currentFrame = QImage();
    m_lastPts = 0;
    update();
}

void FFmpegVideoRenderer::clearFrame()
{
    m_currentFrame = QImage();
    m_lastPts = 0;
    update();
}

void FFmpegVideoRenderer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    if (m_currentFrame.isNull()) return;

    QSize frameSize = m_currentFrame.size();
    frameSize.scale(size(), Qt::KeepAspectRatio);

    QRect targetRect(
        (width() - frameSize.width()) / 2,
        (height() - frameSize.height()) / 2,
        frameSize.width(),
        frameSize.height()
    );

    painter.drawImage(targetRect, m_currentFrame);
}

void FFmpegVideoRenderer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}

void FFmpegVideoRenderer::onVideoFrameReady()
{
    if (!m_updateTimer->isActive()) {
        m_updateTimer->start();
    }
}

void FFmpegVideoRenderer::updateFrame()
{
    if (!m_decoder) return;

    qint64 currentPos = m_decoder->currentPosition();

    VideoFrame frame = m_decoder->takeVideoFrame(currentPos);
    if (!frame.image.isNull()) {
        m_currentFrame = frame.image;
        m_lastPts = frame.pts;
        emit frameDisplayed(frame.pts);
        update();
    }
}
