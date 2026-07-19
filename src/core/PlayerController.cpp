#include "core/PlayerController.h"
#include <QSettings>

#ifdef FFMPEG_ENABLED
#include "ffmpeg/FFmpegPlayer.h"
#endif

PlayerController::PlayerController(QObject* parent)
    : QObject(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_playbackState(PlaybackState::Stopped)
    , m_playbackSpeed(1.0)
    , m_volumeBoostEnabled(false)
    , m_volumeBeforeBoost(100)
    , m_pitchCompensation(true)
    , m_decoderBackend(DecoderBackend::QtMultimedia)
    , m_positionTimer(nullptr)
    , m_lastEmittedPosition(-1)
#ifdef FFMPEG_ENABLED
    , m_ffmpegPlayer(nullptr)
#endif
{
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.7);

    QSettings settings;
    m_pitchCompensation = settings.value("playback/preservePitch", true).toBool();

    int backend = settings.value("playback/decoderBackend", 0).toInt();
    m_decoderBackend = static_cast<DecoderBackend>(backend);

    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &PlayerController::onMediaPlayerStateChanged);
    // Qt Multimedia 的 positionChanged 信号更新不规则，用定时器统一刷新
    // connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
    //         this, &PlayerController::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &PlayerController::durationChanged);
    connect(m_audioOutput, &QAudioOutput::volumeChanged, this, [this](float vol) {
        int volInt = static_cast<int>(vol * 100);
        emit volumeChanged(volInt);
        onVolumeBoostCheck(volInt);
    });
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &PlayerController::onErrorOccurred);

    // 统一的位置刷新定时器，确保进度条平滑
    m_positionTimer = new QTimer(this);
    m_positionTimer->setInterval(50); // 20 FPS 进度更新
    m_positionTimer->setTimerType(Qt::PreciseTimer);
    connect(m_positionTimer, &QTimer::timeout, this, [this]() {
        if (m_playbackState != PlaybackState::Playing) return;
        qint64 pos = position();
        if (pos != m_lastEmittedPosition) {
            m_lastEmittedPosition = pos;
            emit positionChanged(pos);
        }
    });

#ifdef FFMPEG_ENABLED
    if (m_decoderBackend != DecoderBackend::QtMultimedia) {
        m_ffmpegPlayer = new FFmpegPlayer(this);
        connect(m_ffmpegPlayer, &FFmpegPlayer::stateChanged,
                this, &PlayerController::onFFmpegStateChanged);
        // FFmpeg 也走统一的定时器刷新，避免双源冲突
        // connect(m_ffmpegPlayer, &FFmpegPlayer::positionChanged,
        //         this, &PlayerController::onFFmpegPositionChanged);
        connect(m_ffmpegPlayer, &FFmpegPlayer::durationChanged,
                this, &PlayerController::onFFmpegDurationChanged);
        connect(m_ffmpegPlayer, &FFmpegPlayer::finished,
                this, &PlayerController::onFFmpegFinished);
        connect(m_ffmpegPlayer, &FFmpegPlayer::error,
                this, &PlayerController::onFFmpegError);
    }
#endif
}

PlayerController::~PlayerController()
{
}

QMediaPlayer* PlayerController::mediaPlayer() const
{
    return m_mediaPlayer;
}

QAudioOutput* PlayerController::audioOutput() const
{
    return m_audioOutput;
}

DecoderBackend PlayerController::decoderBackend() const
{
    return m_decoderBackend;
}

void PlayerController::setDecoderBackend(DecoderBackend backend)
{
    if (m_decoderBackend == backend) return;

    stop();
    m_decoderBackend = backend;

    QSettings settings;
    settings.setValue("playback/decoderBackend", static_cast<int>(backend));

#ifdef FFMPEG_ENABLED
    if (backend != DecoderBackend::QtMultimedia && !m_ffmpegPlayer) {
        m_ffmpegPlayer = new FFmpegPlayer(this);
        connect(m_ffmpegPlayer, &FFmpegPlayer::stateChanged,
                this, &PlayerController::onFFmpegStateChanged);
        connect(m_ffmpegPlayer, &FFmpegPlayer::positionChanged,
                this, &PlayerController::onFFmpegPositionChanged);
        connect(m_ffmpegPlayer, &FFmpegPlayer::durationChanged,
                this, &PlayerController::onFFmpegDurationChanged);
        connect(m_ffmpegPlayer, &FFmpegPlayer::finished,
                this, &PlayerController::onFFmpegFinished);
        connect(m_ffmpegPlayer, &FFmpegPlayer::error,
                this, &PlayerController::onFFmpegError);
    }
#endif

    emit decoderBackendChanged(backend);
}

#ifdef FFMPEG_ENABLED
FFmpegPlayer* PlayerController::ffmpegPlayer() const
{
    return m_ffmpegPlayer;
}

bool PlayerController::useFFmpeg() const
{
    return m_decoderBackend != DecoderBackend::QtMultimedia && m_ffmpegPlayer;
}
#endif

PlaybackState PlayerController::playbackState() const
{
    return m_playbackState;
}

qint64 PlayerController::position() const
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        return m_ffmpegPlayer->position();
    }
#endif
    return m_mediaPlayer->position();
}

qint64 PlayerController::duration() const
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        return m_ffmpegPlayer->duration();
    }
#endif
    return m_mediaPlayer->duration();
}

int PlayerController::volume() const
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        return m_ffmpegPlayer->volume();
    }
#endif
    return static_cast<int>(m_audioOutput->volume() * 100);
}

bool PlayerController::isMuted() const
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        return m_ffmpegPlayer->isMuted();
    }
#endif
    return m_audioOutput->isMuted();
}

double PlayerController::playbackSpeed() const
{
    return m_playbackSpeed;
}

bool PlayerController::volumeBoostEnabled() const
{
    return m_volumeBoostEnabled;
}

void PlayerController::openFile(const QString& filePath)
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        FFmpegPlayer::DecodeMode mode = (m_decoderBackend == DecoderBackend::FFmpegHardware)
            ? FFmpegPlayer::HardwareDecode
            : FFmpegPlayer::SoftwareDecode;
        if (m_ffmpegPlayer->open(filePath, mode)) {
            m_ffmpegPlayer->setVolume(volume());
            m_ffmpegPlayer->setPlaybackSpeed(m_playbackSpeed);
            emit mediaLoaded(filePath);
            return;
        } else {
            qWarning("FFmpeg failed to open file, falling back to QtMultimedia");
        }
    }
#endif
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    emit mediaLoaded(filePath);
}

void PlayerController::play()
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        m_ffmpegPlayer->play();
        return;
    }
#endif
    m_mediaPlayer->play();
}

void PlayerController::pause()
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        m_ffmpegPlayer->pause();
        return;
    }
#endif
    m_mediaPlayer->pause();
}

void PlayerController::togglePlayPause()
{
    if (m_playbackState == PlaybackState::Playing) {
        pause();
    } else {
        play();
    }
}

void PlayerController::stop()
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        m_ffmpegPlayer->stop();
        return;
    }
#endif
    m_mediaPlayer->stop();
}

void PlayerController::setPosition(qint64 position)
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) {
        m_ffmpegPlayer->setPosition(position);
        m_lastEmittedPosition = -1;
        emit positionChanged(position);
        return;
    }
#endif
    m_mediaPlayer->setPosition(position);
    m_lastEmittedPosition = -1;
    emit positionChanged(position);
}

void PlayerController::setVolume(int volume)
{
    float vol = qBound(0, volume, m_volumeBoostEnabled ? 500 : 100) / 100.0f;
    m_audioOutput->setVolume(vol);

#ifdef FFMPEG_ENABLED
    if (m_ffmpegPlayer) {
        m_ffmpegPlayer->setVolume(volume);
    }
#endif
}

void PlayerController::setMuted(bool muted)
{
    m_audioOutput->setMuted(muted);

#ifdef FFMPEG_ENABLED
    if (m_ffmpegPlayer) {
        m_ffmpegPlayer->setMuted(muted);
    }
#endif

    emit mutedChanged(muted);
}

void PlayerController::toggleMute()
{
    setMuted(!isMuted());
}

void PlayerController::setPlaybackSpeed(double speed)
{
    m_playbackSpeed = speed;
    m_mediaPlayer->setPlaybackRate(speed);

#ifdef FFMPEG_ENABLED
    if (m_ffmpegPlayer) {
        m_ffmpegPlayer->setPlaybackSpeed(speed);
    }
#endif

    emit playbackSpeedChanged(speed);
}

void PlayerController::seekForward(int seconds)
{
    setPosition(position() + seconds * 1000);
}

void PlayerController::seekBackward(int seconds)
{
    setPosition(qMax(0LL, position() - seconds * 1000));
}

void PlayerController::setVolumeBoostEnabled(bool enabled)
{
    m_volumeBoostEnabled = enabled;
    if (enabled) {
        m_volumeBeforeBoost = volume();
    } else {
        if (volume() > 100) {
            setVolume(100);
        }
    }
}

void PlayerController::setPitchCompensation(bool enabled)
{
    if (m_pitchCompensation == enabled) return;
    m_pitchCompensation = enabled;
    QSettings settings;
    settings.setValue("playback/preservePitch", enabled);
    emit pitchCompensationChanged(enabled);
}

bool PlayerController::pitchCompensation() const
{
    return m_pitchCompensation;
}

void PlayerController::onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
#ifdef FFMPEG_ENABLED
    if (useFFmpeg()) return;
#endif

    switch (state) {
    case QMediaPlayer::StoppedState:
        m_playbackState = PlaybackState::Stopped;
        m_positionTimer->stop();
        break;
    case QMediaPlayer::PlayingState:
        m_playbackState = PlaybackState::Playing;
        m_positionTimer->start();
        break;
    case QMediaPlayer::PausedState:
        m_playbackState = PlaybackState::Paused;
        m_positionTimer->stop();
        // 暂停时立即更新一次位置
        m_lastEmittedPosition = -1;
        emit positionChanged(position());
        break;
    }
    emit playbackStateChanged(m_playbackState);
}

void PlayerController::onErrorOccurred(QMediaPlayer::Error error, const QString& errorString)
{
    Q_UNUSED(error);
    emit errorOccurred(errorString);
}

void PlayerController::onVolumeBoostCheck(int value)
{
    if (!m_volumeBoostEnabled && value >= 100) {
        emit volumeBoostRequested();
    }
}

#ifdef FFMPEG_ENABLED
void PlayerController::onFFmpegStateChanged(int state)
{
    FFmpegPlayer::PlaybackState ffmpegState = static_cast<FFmpegPlayer::PlaybackState>(state);
    switch (ffmpegState) {
    case FFmpegPlayer::StoppedState:
        m_playbackState = PlaybackState::Stopped;
        m_positionTimer->stop();
        break;
    case FFmpegPlayer::PlayingState:
        m_playbackState = PlaybackState::Playing;
        m_positionTimer->start();
        break;
    case FFmpegPlayer::PausedState:
        m_playbackState = PlaybackState::Paused;
        m_positionTimer->stop();
        // 暂停时立即更新一次位置
        m_lastEmittedPosition = -1;
        emit positionChanged(position());
        break;
    }
    emit playbackStateChanged(m_playbackState);
}

void PlayerController::onFFmpegPositionChanged(qint64 position)
{
    if (useFFmpeg()) {
        emit PlayerController::positionChanged(position);
    }
}

void PlayerController::onFFmpegDurationChanged(qint64 duration)
{
    if (useFFmpeg()) {
        emit PlayerController::durationChanged(duration);
    }
}

void PlayerController::onFFmpegFinished()
{
    m_playbackState = PlaybackState::Stopped;
    emit playbackStateChanged(m_playbackState);
}

void PlayerController::onFFmpegError(const QString& message)
{
    emit errorOccurred(message);
}
#endif
