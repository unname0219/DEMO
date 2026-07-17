#include "core/PlayerController.h"

PlayerController::PlayerController(QObject* parent)
    : QObject(parent)
    , m_mediaPlayer(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
    , m_playbackState(PlaybackState::Stopped)
    , m_playbackSpeed(1.0)
    , m_volumeBoostEnabled(false)
    , m_volumeBeforeBoost(100)
{
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.7);

    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &PlayerController::onMediaPlayerStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
            this, &PlayerController::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &PlayerController::durationChanged);
    connect(m_audioOutput, &QAudioOutput::volumeChanged, this, [this](float vol) {
        int volInt = static_cast<int>(vol * 100);
        emit volumeChanged(volInt);
        onVolumeBoostCheck(volInt);
    });
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &PlayerController::onErrorOccurred);
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

PlaybackState PlayerController::playbackState() const
{
    return m_playbackState;
}

qint64 PlayerController::position() const
{
    return m_mediaPlayer->position();
}

qint64 PlayerController::duration() const
{
    return m_mediaPlayer->duration();
}

int PlayerController::volume() const
{
    return static_cast<int>(m_audioOutput->volume() * 100);
}

bool PlayerController::isMuted() const
{
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
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    emit mediaLoaded(filePath);
}

void PlayerController::play()
{
    m_mediaPlayer->play();
}

void PlayerController::pause()
{
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
    m_mediaPlayer->stop();
}

void PlayerController::setPosition(qint64 position)
{
    m_mediaPlayer->setPosition(position);
}

void PlayerController::setVolume(int volume)
{
    float vol = qBound(0, volume, m_volumeBoostEnabled ? 500 : 100) / 100.0f;
    m_audioOutput->setVolume(vol);
}

void PlayerController::setMuted(bool muted)
{
    m_audioOutput->setMuted(muted);
    emit mutedChanged(muted);
}

void PlayerController::toggleMute()
{
    setMuted(!m_audioOutput->isMuted());
}

void PlayerController::setPlaybackSpeed(double speed)
{
    m_playbackSpeed = speed;
    m_mediaPlayer->setPlaybackRate(speed);
    emit playbackSpeedChanged(speed);
}

void PlayerController::seekForward(int seconds)
{
    m_mediaPlayer->setPosition(m_mediaPlayer->position() + seconds * 1000);
}

void PlayerController::seekBackward(int seconds)
{
    m_mediaPlayer->setPosition(qMax(0LL, m_mediaPlayer->position() - seconds * 1000));
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

void PlayerController::onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::StoppedState:
        m_playbackState = PlaybackState::Stopped;
        break;
    case QMediaPlayer::PlayingState:
        m_playbackState = PlaybackState::Playing;
        break;
    case QMediaPlayer::PausedState:
        m_playbackState = PlaybackState::Paused;
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
