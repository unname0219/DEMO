#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include <QObject>
#include <QString>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QAudioFormat>
#include <thread>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}

struct AudioFrame {
    QByteArray data;
    qint64 pts;
};

struct VideoFrame {
    QImage image;
    qint64 pts;
};

class FFmpegDecoder : public QObject
{
    Q_OBJECT

public:
    enum DecodeMode {
        SoftwareDecode,
        HardwareDecode
    };

    explicit FFmpegDecoder(QObject* parent = nullptr);
    ~FFmpegDecoder();

    bool open(const QString& filePath, DecodeMode mode = SoftwareDecode);
    void close();

    void start();
    void pause();
    void stop();
    void seek(qint64 positionMs);

    qint64 duration() const { return m_durationMs; }
    qint64 currentPosition() const { return m_currentPositionMs; }
    bool isPlaying() const { return m_isPlaying; }
    bool isPaused() const { return m_isPaused; }

    int videoWidth() const { return m_videoWidth; }
    int videoHeight() const { return m_videoHeight; }
    double fps() const { return m_fps; }

    QAudioFormat audioFormat() const { return m_audioFormat; }

    VideoFrame takeVideoFrame(qint64 positionMs);
    AudioFrame currentAudioFrame();

    void setPlaybackSpeed(double speed);
    double playbackSpeed() const { return m_playbackSpeed; }

    void updatePosition();

    void decodeLoop();

    bool isDecodeFinished() const { return m_decodeFinished; }
    bool isQueueEmpty() const;

signals:
    void videoFrameReady();
    void audioFrameReady();
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void stateChanged(bool playing);
    void finished();
    void error(const QString& message);

private:
    bool openCodec(AVCodecParameters* codecpar, AVCodecContext** codecCtx,
                   AVStream* stream, bool isVideo, DecodeMode mode);
    bool initHardwareDecoder(AVCodecContext* codecCtx, AVCodecParameters* codecpar);
    bool decodePacket(AVPacket* pkt, AVCodecContext* codecCtx, AVFrame* frame,
                      bool isVideo, QQueue<VideoFrame>& videoQueue,
                      QQueue<AudioFrame>& audioQueue, qint64& outPts);
    void flushDecoder(AVCodecContext* codecCtx, AVFrame* frame, bool isVideo,
                      QQueue<VideoFrame>& videoQueue, QQueue<AudioFrame>& audioQueue);

    AVFormatContext* m_formatCtx;
    AVCodecContext* m_videoCodecCtx;
    AVCodecContext* m_audioCodecCtx;
    SwsContext* m_swsCtx;
    SwrContext* m_swrCtx;

    int m_videoStreamIndex;
    int m_audioStreamIndex;

    qint64 m_durationMs;
    qint64 m_currentPositionMs;
    qint64 m_startTime;
    qint64 m_pauseTime;

    bool m_isPlaying;
    bool m_isPaused;
    bool m_isStopRequested;
    bool m_isSeekRequested;
    bool m_decodeFinished;
    qint64 m_seekPositionMs;

    int m_videoWidth;
    int m_videoHeight;
    double m_fps;

    double m_playbackSpeed;

    QAudioFormat m_audioFormat;

    QQueue<VideoFrame> m_videoQueue;
    QQueue<AudioFrame> m_audioQueue;

    mutable QMutex m_mutex;
    QWaitCondition m_waitCondition;

    std::thread m_decodeThread;

    DecodeMode m_decodeMode;
};

#endif
