#ifndef PITCHSHIFTER_H
#define PITCHSHIFTER_H

#include <QObject>
#include <QByteArray>
#include <QMutex>
#include <soundtouch/SoundTouch.h>

class PitchShifter : public QObject
{
    Q_OBJECT

public:
    explicit PitchShifter(QObject* parent = nullptr);
    ~PitchShifter();

    void setSpeed(double speed);
    double speed() const;

    void setEnabled(bool enabled);
    bool enabled() const;

    QByteArray process(const QByteArray& input, int sampleRate, int channels);

    void clear();

private:
    soundtouch::SoundTouch m_soundTouch;
    double m_speed;
    bool m_enabled;
    QMutex m_mutex;
};

#endif
