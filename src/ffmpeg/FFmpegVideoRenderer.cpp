#include "ffmpeg/FFmpegVideoRenderer.h"
#include "ffmpeg/FFmpegDecoder.h"
#include <QPainter>
#include <QResizeEvent>
#include <QElapsedTimer>

FFmpegVideoRenderer::FFmpegVideoRenderer(QWidget* parent)
    : QWidget(parent)
    , m_decoder(nullptr)
    , m_updateTimer(nullptr)
    , m_lastPts(0)
    , m_frameCount(0)
    , m_lastFpsTime(0)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setMinimumSize(320, 240);
    setStyleSheet("background-color: black;");

    m_updateTimer = new QTimer(this);
    m_updateTimer->setTimerType(Qt::PreciseTimer);
    m_updateTimer->setInterval(10); // 100FPS上限，由PTS同步控制实际帧率
    connect(m_updateTimer, &QTimer::timeout, this, &FFmpegVideoRenderer::updateFrame);

    // 双缓冲：减少绘制时的等待
    setAttribute(Qt::WA_PaintOnScreen, false);
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

    QSize widgetSize = size();
    // 只在尺寸变化或无缓存时重新缩放
    if (m_lastRenderSize != widgetSize || m_scaledFrame.isNull()) {
        QSize frameSize = m_currentFrame.size();
        frameSize.scale(widgetSize, Qt::KeepAspectRatio);
        if (frameSize.width() > 0 && frameSize.height() > 0) {
            m_scaledFrame = m_currentFrame.scaled(frameSize, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }
        m_lastRenderSize = widgetSize;
    }

    if (m_scaledFrame.isNull()) return;

    QRect targetRect(
        (width() - m_scaledFrame.width()) / 2,
        (height() - m_scaledFrame.height()) / 2,
        m_scaledFrame.width(),
        m_scaledFrame.height()
    );

    painter.drawImage(targetRect, m_scaledFrame);
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
        // 帧变了，缩放缓存失效
        m_scaledFrame = QImage();
        m_lastPts = frame.pts;
        emit frameDisplayed(frame.pts);
        update();
    }
}
