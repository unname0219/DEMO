#include "core/ProcessingAudioOutput.h"

ProcessingAudioOutput::ProcessingAudioOutput(QObject* parent)
    : QObject(parent)
    , m_audioSink(nullptr)
    , m_speed(1.0)
    , m_pitchCompensation(true)
    , m_running(false)
    , m_volume(1.0)
    , m_muted(false)
{
    m_soundTouch.setChannels(2);
    m_soundTouch.setSampleRate(44100);
    m_soundTouch.setRate(1.0);
    m_soundTouch.setPitch(1.0);
    m_soundTouch.setTempo(1.0);

    m_audioBuffer.open(QIODevice::ReadOnly);
}

ProcessingAudioOutput::~ProcessingAudioOutput()
{
    stop();
    if (m_audioSink) {
        delete m_audioSink;
    }
}

void ProcessingAudioOutput::setAudioFormat(const QAudioFormat& format)
{
    m_mutex.lock();
    m_format = format;
    m_soundTouch.setSampleRate(format.sampleRate());
    m_soundTouch.setChannels(format.channelCount());

    if (m_audioSink) {
        delete m_audioSink;
        m_audioSink = nullptr;
    }

    m_audioSink = new QAudioSink(format, this);
    connect(m_audioSink, &QAudioSink::stateChanged, this, [this](QAudio::State state) {
        if (state == QAudio::IdleState && m_running && !m_unprocessedBuffer.isEmpty()) {
            processAudioData();
        }
    });

    m_audioSink->setVolume(m_muted ? 0 : m_volume);
    m_mutex.unlock();
}

QAudioFormat ProcessingAudioOutput::audioFormat() const
{
    return m_format;
}

void ProcessingAudioOutput::setVolume(float volume)
{
    m_mutex.lock();
    m_volume = volume;
    if (m_audioSink && !m_muted) {
        m_audioSink->setVolume(volume);
    }
    m_mutex.unlock();
    emit volumeChanged(volume);
}

float ProcessingAudioOutput::volume() const
{
    return m_volume;
}

void ProcessingAudioOutput::setMuted(bool muted)
{
    m_mutex.lock();
    m_muted = muted;
    if (m_audioSink) {
        m_audioSink->setVolume(muted ? 0 : m_volume);
    }
    m_mutex.unlock();
    emit mutedChanged(muted);
}

bool ProcessingAudioOutput::isMuted() const
{
    return m_muted;
}

void ProcessingAudioOutput::setSpeed(double speed)
{
    m_mutex.lock();
    m_speed = speed;

    if (m_pitchCompensation && speed != 1.0) {
        m_soundTouch.setTempo(speed);
        m_soundTouch.setPitch(1.0 / speed);
    } else {
        m_soundTouch.setTempo(1.0);
        m_soundTouch.setPitch(1.0);
    }
    m_mutex.unlock();
}

double ProcessingAudioOutput::speed() const
{
    return m_speed;
}

void ProcessingAudioOutput::setPitchCompensation(bool enabled)
{
    m_mutex.lock();
    m_pitchCompensation = enabled;

    if (enabled && m_speed != 1.0) {
        m_soundTouch.setTempo(m_speed);
        m_soundTouch.setPitch(1.0 / m_speed);
    } else {
        m_soundTouch.setTempo(1.0);
        m_soundTouch.setPitch(1.0);
    }
    m_mutex.unlock();
}

bool ProcessingAudioOutput::pitchCompensation() const
{
    return m_pitchCompensation;
}

void ProcessingAudioOutput::start()
{
    m_mutex.lock();
    m_running = true;
    if (m_audioSink && m_audioSink->state() != QAudio::ActiveState) {
        m_audioSink->start(&m_audioBuffer);
    }
    m_mutex.unlock();
}

void ProcessingAudioOutput::stop()
{
    m_mutex.lock();
    m_running = false;
    m_soundTouch.clear();
    m_unprocessedBuffer.clear();
    m_processedBuffer.clear();
    if (m_audioSink) {
        m_audioSink->stop();
    }
    m_mutex.unlock();
}

void ProcessingAudioOutput::writeAudioData(const QByteArray& data)
{
    m_mutex.lock();
    if (!m_running || data.isEmpty()) {
        m_mutex.unlock();
        return;
    }

    m_unprocessedBuffer.append(data);

    if (m_audioSink && m_audioSink->bytesFree() > 0) {
        m_mutex.unlock();
        processAudioData();
    } else {
        m_mutex.unlock();
    }
}

bool ProcessingAudioOutput::isRunning() const
{
    return m_running;
}

void ProcessingAudioOutput::processAudioData()
{
    m_mutex.lock();
    if (m_unprocessedBuffer.isEmpty()) {
        m_mutex.unlock();
        return;
    }

    QByteArray processed;

    if (m_pitchCompensation && m_speed != 1.0) {
        int sampleSize = m_format.bytesPerSample();
        int channels = m_format.channelCount();
        int samples = m_unprocessedBuffer.size() / (channels * sampleSize);

        if (samples > 0) {
            QVector<float> floatBuffer(samples * channels);
            const short* inputSamples = reinterpret_cast<const short*>(m_unprocessedBuffer.constData());
            for (int i = 0; i < samples * channels; i++) {
                floatBuffer[i] = inputSamples[i] / 32768.0f;
            }

            m_soundTouch.putSamples(floatBuffer.data(), samples);

            int outputSamples;
            float tempBuffer[8192];

            do {
                outputSamples = m_soundTouch.receiveSamples(tempBuffer, 8192);
                if (outputSamples > 0) {
                    QVector<short> shortBuffer(outputSamples * channels);
                    for (int i = 0; i < outputSamples * channels; i++) {
                        shortBuffer[i] = static_cast<short>(tempBuffer[i] * 32768.0f);
                    }
                    processed.append(reinterpret_cast<const char*>(shortBuffer.data()), outputSamples * channels * sampleSize);
                }
            } while (outputSamples > 0);
        }
    } else {
        processed = m_unprocessedBuffer;
    }

    m_unprocessedBuffer.clear();

    if (!processed.isEmpty()) {
        m_processedBuffer.append(processed);

        int bytesToWrite = qMin(m_processedBuffer.size(), m_audioSink->bytesFree());
        if (bytesToWrite > 0) {
            m_audioBuffer.buffer().replace(0, m_audioBuffer.buffer().size(), QByteArray());
            m_audioBuffer.buffer().append(m_processedBuffer.left(bytesToWrite));
            m_audioBuffer.seek(0);
            m_processedBuffer.remove(0, bytesToWrite);
        }
    }
    m_mutex.unlock();
}
