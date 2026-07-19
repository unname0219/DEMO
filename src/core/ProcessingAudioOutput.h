#ifndef PROCESSINGAUDIOOUTPUT_H
#define PROCESSINGAUDIOOUTPUT_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QBuffer>
#include <soundtouch/SoundTouch.h>
#include <QMutex>

class ProcessingAudioOutput : public QObject
{
    Q_OBJECT

public:
    explicit ProcessingAudioOutput(QObject* parent = nullptr);
    ~ProcessingAudioOutput();

    void setAudioFormat(const QAudioFormat& format);
    QAudioFormat audioFormat() const;

    void setVolume(float volume);
    float volume() const;

    void setMuted(bool muted);
    bool isMuted() const;

    void setSpeed(double speed);
    double speed() const;

    void setPitchCompensation(bool enabled);
    bool pitchCompensation() const;

    void start();
    void stop();

    void writeAudioData(const QByteArray& data);

    bool isRunning() const;

signals:
    void volumeChanged(float volume);
    void mutedChanged(bool muted);

private:
    void processAudioData();

    QAudioSink* m_audioSink;
    QAudioFormat m_format;
    QBuffer m_audioBuffer;
    QByteArray m_processedBuffer;
    QByteArray m_unprocessedBuffer;

    soundtouch::SoundTouch m_soundTouch;
    double m_speed;
    bool m_pitchCompensation;
    bool m_running;

    float m_volume;
    bool m_muted;

    QMutex m_mutex;
};

#endif
