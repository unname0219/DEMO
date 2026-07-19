#ifndef FFMPEGPLAYER_H
#define FFMPEGPLAYER_H

#include <QObject>
#include <QString>
#include <QAudioSink>
#include <QAudioSource>
#include <QTimer>

class FFmpegDecoder;
class FFmpegVideoRenderer;

class FFmpegPlayer : public QObject
{
    Q_OBJECT

public:
    enum PlaybackState {
        StoppedState,
        PlayingState,
        PausedState
    };

    enum DecodeMode {
        SoftwareDecode,
        HardwareDecode
    };

    explicit FFmpegPlayer(QObject* parent = nullptr);
    ~FFmpegPlayer();

    bool open(const QString& filePath, DecodeMode mode = SoftwareDecode);
    void close();

    void play();
    void pause();
    void stop();
    void setPosition(qint64 positionMs);
    void setVolume(int volume);
    void setPlaybackSpeed(double speed);
    void setMuted(bool muted);

    qint64 duration() const;
    qint64 position() const;
    int volume() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isMuted() const;
    double playbackSpeed() const;

    FFmpegVideoRenderer* videoRenderer() const;
    FFmpegDecoder* decoder() const { return m_decoder; }
    bool hasVideo() const;
    bool hasAudio() const;

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void stateChanged(PlaybackState state);
    void videoFrameChanged();
    void finished();
    void error(const QString& message);

private slots:
    void onVideoFrameReady();
    void onAudioFrameReady();
    void onDecoderFinished();
    void onDecoderError(const QString& message);
    void updatePosition();
    void writeAudioData();

private:
    void initAudioOutput();
    void startAudioOutput();
    void stopAudioOutput();

    FFmpegDecoder* m_decoder;
    FFmpegVideoRenderer* m_videoRenderer;
    QAudioSink* m_audioSink;
    QIODevice* m_audioDevice;

    QTimer* m_positionTimer;
    QTimer* m_audioWriteTimer;

    PlaybackState m_state;
    int m_volume;
    bool m_isMuted;
    double m_playbackSpeed;

    QByteArray m_audioBuffer;
    bool m_audioOutputStarted;
};

#endif
