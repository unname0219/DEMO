#include "core/PitchShifter.h"

PitchShifter::PitchShifter(QObject* parent)
    : QObject(parent)
    , m_speed(1.0)
    , m_enabled(false)
{
    m_soundTouch.setChannels(2);
    m_soundTouch.setSampleRate(44100);
    m_soundTouch.setRate(1.0);
    m_soundTouch.setPitch(1.0);
    m_soundTouch.setTempo(1.0);
}

PitchShifter::~PitchShifter()
{
}

void PitchShifter::setSpeed(double speed)
{
    QMutexLocker locker(&m_mutex);
    m_speed = speed;
    if (m_enabled) {
        m_soundTouch.setTempo(speed);
        m_soundTouch.setPitch(1.0 / speed);
    } else {
        m_soundTouch.setTempo(1.0);
        m_soundTouch.setPitch(1.0);
    }
}

double PitchShifter::speed() const
{
    return m_speed;
}

void PitchShifter::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enabled = enabled;
    if (m_enabled) {
        m_soundTouch.setTempo(m_speed);
        m_soundTouch.setPitch(1.0 / m_speed);
    } else {
        m_soundTouch.setTempo(1.0);
        m_soundTouch.setPitch(1.0);
    }
}

bool PitchShifter::enabled() const
{
    return m_enabled;
}

QByteArray PitchShifter::process(const QByteArray& input, int sampleRate, int channels)
{
    QMutexLocker locker(&m_mutex);

    if (!m_enabled || m_speed == 1.0) {
        return input;
    }

    m_soundTouch.setSampleRate(sampleRate);
    m_soundTouch.setChannels(channels);

    int sampleSize = 2;
    int samples = input.size() / (channels * sampleSize);

    if (samples == 0) {
        return QByteArray();
    }

    QVector<float> floatBuffer(samples * channels);
    const short* inputSamples = reinterpret_cast<const short*>(input.constData());
    for (int i = 0; i < samples * channels; i++) {
        floatBuffer[i] = inputSamples[i] / 32768.0f;
    }

    m_soundTouch.putSamples(floatBuffer.data(), samples);

    QByteArray output;
    int outputSamples;

    do {
        float tempBuffer[8192];
        outputSamples = m_soundTouch.receiveSamples(tempBuffer, 8192);
        if (outputSamples > 0) {
            QVector<short> shortBuffer(outputSamples * channels);
            for (int i = 0; i < outputSamples * channels; i++) {
                shortBuffer[i] = static_cast<short>(tempBuffer[i] * 32768.0f);
            }
            output.append(reinterpret_cast<const char*>(shortBuffer.data()), outputSamples * channels * sampleSize);
        }
    } while (outputSamples > 0);

    return output;
}

void PitchShifter::clear()
{
    QMutexLocker locker(&m_mutex);
    m_soundTouch.clear();
}
