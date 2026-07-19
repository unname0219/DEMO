#include "ffmpeg/FFmpegPlayer.h"
#include "ffmpeg/FFmpegDecoder.h"
#include "ffmpeg/FFmpegVideoRenderer.h"
#include <QAudioFormat>
#include <QAudioDevice>
#include <QMediaDevices>

FFmpegPlayer::FFmpegPlayer(QObject* parent)
    : QObject(parent)
    , m_decoder(nullptr)
    , m_videoRenderer(nullptr)
    , m_audioSink(nullptr)
    , m_audioDevice(nullptr)
    , m_positionTimer(nullptr)
    , m_state(StoppedState)
    , m_volume(80)
    , m_isMuted(false)
    , m_playbackSpeed(1.0)
    , m_audioOutputStarted(false)
{
    m_decoder = new FFmpegDecoder(this);
    m_videoRenderer = new FFmpegVideoRenderer();

    connect(m_decoder, &FFmpegDecoder::videoFrameReady,
            this, &FFmpegPlayer::onVideoFrameReady);
    connect(m_decoder, &FFmpegDecoder::audioFrameReady,
            this, &FFmpegPlayer::onAudioFrameReady);
    connect(m_decoder, &FFmpegDecoder::finished,
            this, &FFmpegPlayer::onDecoderFinished);
    connect(m_decoder, &FFmpegDecoder::error,
            this, &FFmpegPlayer::onDecoderError);

    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(100);
    connect(m_positionTimer, &QTimer::timeout,
            this, &FFmpegPlayer::updatePosition);
}

FFmpegPlayer::~FFmpegPlayer()
{
    stop();
    close();

    if (m_videoRenderer) {
        m_videoRenderer->deleteLater();
        m_videoRenderer = nullptr;
    }
}

bool FFmpegPlayer::open(const QString& filePath, DecodeMode mode)
{
    close();

    FFmpegDecoder::DecodeMode decMode = (mode == HardwareDecode)
        ? FFmpegDecoder::HardwareDecode
        : FFmpegDecoder::SoftwareDecode;

    if (!m_decoder->open(filePath, decMode)) {
        emit error("Failed to open media file");
        return false;
    }

    m_videoRenderer->setDecoder(m_decoder);

    if (m_decoder->duration() > 0) {
        emit durationChanged(m_decoder->duration());
    }

    initAudioOutput();

    return true;
}

void FFmpegPlayer::close()
{
    stopAudioOutput();

    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_decoder->close();
    m_videoRenderer->clearFrame();
    m_audioBuffer.clear();
    m_audioOutputStarted = false;

    m_state = StoppedState;
    emit stateChanged(m_state);
}

void FFmpegPlayer::play()
{
    if (!m_decoder) return;

    m_decoder->start();

    m_state = PlayingState;
    emit stateChanged(m_state);

    m_positionTimer->start();

    if (hasAudio()) {
        startAudioOutput();
    }
}

void FFmpegPlayer::pause()
{
    if (!m_decoder) return;

    if (m_state == PlayingState) {
        m_decoder->pause();
        m_state = PausedState;
        emit stateChanged(m_state);
    } else if (m_state == PausedState) {
        m_decoder->pause();
        m_state = PlayingState;
        emit stateChanged(m_state);
    }
}

void FFmpegPlayer::stop()
{
    m_decoder->stop();
    m_positionTimer->stop();
    stopAudioOutput();
    m_audioBuffer.clear();
    m_audioOutputStarted = false;
    m_state = StoppedState;
    emit stateChanged(m_state);
}

void FFmpegPlayer::setPosition(qint64 positionMs)
{
    if (!m_decoder) return;
    m_decoder->seek(positionMs);
    emit positionChanged(positionMs);
}

void FFmpegPlayer::setVolume(int volume)
{
    m_volume = volume;
    if (m_audioSink && !m_isMuted) {
        m_audioSink->setVolume(m_volume / 100.0);
    }
}

void FFmpegPlayer::setPlaybackSpeed(double speed)
{
    m_playbackSpeed = speed;
    if (m_decoder) {
        m_decoder->setPlaybackSpeed(speed);
    }
}

void FFmpegPlayer::setMuted(bool muted)
{
    m_isMuted = muted;
    if (m_audioSink) {
        m_audioSink->setVolume(muted ? 0.0 : m_volume / 100.0);
    }
}

qint64 FFmpegPlayer::duration() const
{
    if (m_decoder) return m_decoder->duration();
    return 0;
}

qint64 FFmpegPlayer::position() const
{
    if (m_decoder) return m_decoder->currentPosition();
    return 0;
}

int FFmpegPlayer::volume() const
{
    return m_volume;
}

bool FFmpegPlayer::isPlaying() const
{
    return m_state == PlayingState;
}

bool FFmpegPlayer::isPaused() const
{
    return m_state == PausedState;
}

bool FFmpegPlayer::isMuted() const
{
    return m_isMuted;
}

double FFmpegPlayer::playbackSpeed() const
{
    return m_playbackSpeed;
}

FFmpegVideoRenderer* FFmpegPlayer::videoRenderer() const
{
    return m_videoRenderer;
}

bool FFmpegPlayer::hasVideo() const
{
    if (m_decoder) return m_decoder->videoWidth() > 0;
    return false;
}

bool FFmpegPlayer::hasAudio() const
{
    if (!m_decoder) return false;
    QAudioFormat fmt = m_decoder->audioFormat();
    return fmt.sampleRate() > 0;
}

void FFmpegPlayer::onVideoFrameReady()
{
    emit videoFrameChanged();
}

void FFmpegPlayer::onAudioFrameReady()
{
    if (!hasAudio()) return;

    AudioFrame frame = m_decoder->currentAudioFrame();
    if (frame.data.isEmpty()) return;

    m_audioBuffer.append(frame.data);

    if (m_audioSink && m_audioDevice && m_state == PlayingState) {
        qint64 bytesFree = m_audioSink->bytesFree();
        if (bytesFree > 0 && m_audioBuffer.size() > 0) {
            qint64 toWrite = qMin((qint64)m_audioBuffer.size(), bytesFree);
            qint64 written = m_audioDevice->write(m_audioBuffer.constData(), toWrite);
            if (written > 0) {
                m_audioBuffer.remove(0, written);
            }
        }
    }
}

void FFmpegPlayer::onDecoderFinished()
{
    m_state = StoppedState;
    m_positionTimer->stop();
    emit stateChanged(m_state);
    emit finished();
}

void FFmpegPlayer::onDecoderError(const QString& message)
{
    emit error(message);
}

void FFmpegPlayer::updatePosition()
{
    if (m_decoder) {
        emit positionChanged(m_decoder->currentPosition());
    }
}

void FFmpegPlayer::initAudioOutput()
{
    if (!hasAudio()) return;

    QAudioFormat format = m_decoder->audioFormat();
    QAudioDevice device = QMediaDevices::defaultAudioOutput();

    if (m_audioSink) {
        m_audioSink->stop();
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_audioSink = new QAudioSink(device, format, this);
    m_audioSink->setVolume(m_volume / 100.0);
    m_audioBuffer.clear();
    m_audioOutputStarted = false;
}

void FFmpegPlayer::startAudioOutput()
{
    if (!m_audioSink || m_audioOutputStarted) return;

    m_audioDevice = m_audioSink->start();
    m_audioOutputStarted = true;
}

void FFmpegPlayer::stopAudioOutput()
{
    if (m_audioSink) {
        m_audioSink->stop();
        m_audioDevice = nullptr;
    }
    m_audioOutputStarted = false;
}
