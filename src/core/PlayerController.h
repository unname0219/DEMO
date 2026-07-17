#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QUrl>

enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

class PlayerController : public QObject
{
    Q_OBJECT

public:
    explicit PlayerController(QObject* parent = nullptr);
    ~PlayerController();

    QMediaPlayer* mediaPlayer() const;
    QAudioOutput* audioOutput() const;

    PlaybackState playbackState() const;
    qint64 position() const;
    qint64 duration() const;
    int volume() const;
    bool isMuted() const;
    double playbackSpeed() const;
    bool volumeBoostEnabled() const;

public slots:
    void openFile(const QString& filePath);
    void play();
    void pause();
    void togglePlayPause();
    void stop();
    void setPosition(qint64 position);
    void setVolume(int volume);
    void setMuted(bool muted);
    void toggleMute();
    void setPlaybackSpeed(double speed);
    void seekForward(int seconds);
    void seekBackward(int seconds);
    void setVolumeBoostEnabled(bool enabled);

signals:
    void playbackStateChanged(PlaybackState state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void mutedChanged(bool muted);
    void playbackSpeedChanged(double speed);
    void mediaLoaded(const QString& filePath);
    void errorOccurred(const QString& error);
    void volumeBoostRequested();

private slots:
    void onMediaPlayerStateChanged(QMediaPlayer::PlaybackState state);
    void onErrorOccurred(QMediaPlayer::Error error, const QString& errorString);
    void onVolumeBoostCheck(int value);

private:
    QMediaPlayer* m_mediaPlayer;
    QAudioOutput* m_audioOutput;
    PlaybackState m_playbackState;
    double m_playbackSpeed;
    bool m_volumeBoostEnabled;
    int m_volumeBeforeBoost;
};

#endif
