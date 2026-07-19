#include "ffmpeg/FFmpegDecoder.h"
#include <QDebug>
#include <QCoreApplication>

FFmpegDecoder::FFmpegDecoder(QObject* parent)
    : QObject(parent)
    , m_formatCtx(nullptr)
    , m_videoCodecCtx(nullptr)
    , m_audioCodecCtx(nullptr)
    , m_swsCtx(nullptr)
    , m_swrCtx(nullptr)
    , m_videoStreamIndex(-1)
    , m_audioStreamIndex(-1)
    , m_durationMs(0)
    , m_currentPositionMs(0)
    , m_startTime(0)
    , m_pauseTime(0)
    , m_isPlaying(false)
    , m_isPaused(false)
    , m_isStopRequested(false)
    , m_isSeekRequested(false)
    , m_decodeFinished(false)
    , m_seekPositionMs(0)
    , m_videoWidth(0)
    , m_videoHeight(0)
    , m_fps(0.0)
    , m_playbackSpeed(1.0)
    , m_decodeMode(SoftwareDecode)
{
}

FFmpegDecoder::~FFmpegDecoder()
{
    stop();
    close();
}

bool FFmpegDecoder::open(const QString& filePath, DecodeMode mode)
{
    close();
    m_decodeMode = mode;

    int ret = avformat_open_input(&m_formatCtx, filePath.toUtf8().constData(), nullptr, nullptr);
    if (ret < 0) {
        char errBuf[256];
        av_strerror(ret, errBuf, sizeof(errBuf));
        emit error(QString("Failed to open file: %1").arg(errBuf));
        return false;
    }

    ret = avformat_find_stream_info(m_formatCtx, nullptr);
    if (ret < 0) {
        char errBuf[256];
        av_strerror(ret, errBuf, sizeof(errBuf));
        emit error(QString("Failed to find stream info: %1").arg(errBuf));
        return false;
    }

    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;

    for (unsigned int i = 0; i < m_formatCtx->nb_streams; i++) {
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && m_videoStreamIndex < 0) {
            m_videoStreamIndex = i;
        }
        if (m_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && m_audioStreamIndex < 0) {
            m_audioStreamIndex = i;
        }
    }

    if (m_videoStreamIndex < 0 && m_audioStreamIndex < 0) {
        emit error("No video or audio stream found");
        return false;
    }

    if (m_videoStreamIndex >= 0) {
        AVStream* videoStream = m_formatCtx->streams[m_videoStreamIndex];
        if (!openCodec(videoStream->codecpar, &m_videoCodecCtx, videoStream, true, mode)) {
            qWarning() << "Failed to open video codec, trying without video";
            m_videoStreamIndex = -1;
        } else {
            m_videoWidth = m_videoCodecCtx->width;
            m_videoHeight = m_videoCodecCtx->height;
            if (videoStream->avg_frame_rate.den > 0 && videoStream->avg_frame_rate.num > 0) {
                m_fps = av_q2d(videoStream->avg_frame_rate);
            } else {
                m_fps = 25.0;
            }

            m_swsCtx = sws_getContext(
                m_videoWidth, m_videoHeight, m_videoCodecCtx->pix_fmt,
                m_videoWidth, m_videoHeight, AV_PIX_FMT_RGB24,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
        }
    }

    if (m_audioStreamIndex >= 0) {
        AVStream* audioStream = m_formatCtx->streams[m_audioStreamIndex];
        if (!openCodec(audioStream->codecpar, &m_audioCodecCtx, audioStream, false, SoftwareDecode)) {
            qWarning() << "Failed to open audio codec, trying without audio";
            m_audioStreamIndex = -1;
        } else {
            m_swrCtx = swr_alloc();
            if (m_swrCtx) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 24, 100)
                AVChannelLayout outChLayout = AV_CHANNEL_LAYOUT_STEREO;
                swr_alloc_set_opts2(&m_swrCtx,
                    &outChLayout, AV_SAMPLE_FMT_S16, 48000,
                    &m_audioCodecCtx->ch_layout, m_audioCodecCtx->sample_fmt, m_audioCodecCtx->sample_rate,
                    0, nullptr);
#else
                av_opt_set_int(m_swrCtx, "in_channel_layout", m_audioCodecCtx->channel_layout ? m_audioCodecCtx->channel_layout : AV_CH_LAYOUT_STEREO, 0);
                av_opt_set_int(m_swrCtx, "in_sample_rate", m_audioCodecCtx->sample_rate, 0);
                av_opt_set_sample_fmt(m_swrCtx, "in_sample_fmt", m_audioCodecCtx->sample_fmt, 0);
                av_opt_set_int(m_swrCtx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
                av_opt_set_int(m_swrCtx, "out_sample_rate", 48000, 0);
                av_opt_set_sample_fmt(m_swrCtx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
#endif
                swr_init(m_swrCtx);

                m_audioFormat.setSampleRate(48000);
                m_audioFormat.setChannelCount(2);
                m_audioFormat.setSampleFormat(QAudioFormat::Int16);
            }
        }
    }

    if (m_formatCtx->duration != AV_NOPTS_VALUE) {
        m_durationMs = m_formatCtx->duration / 1000;
    }

    return true;
}

bool FFmpegDecoder::openCodec(AVCodecParameters* codecpar, AVCodecContext** codecCtx,
                              AVStream* stream, bool isVideo, DecodeMode mode)
{
    const AVCodec* codec = nullptr;

    if (isVideo && mode == HardwareDecode) {
        static const char* hwDecoders[] = {
            "h264_qsv", "hevc_qsv", "av1_qsv",
            "h264_nvdec", "hevc_nvdec", "av1_nvdec",
            "h264_d3d11va", "hevc_d3d11va", "av1_d3d11va",
            "h264_vaapi", "hevc_vaapi", "av1_vaapi",
            "h264_vdpau", "hevc_vdpau",
            nullptr
        };

        for (int i = 0; hwDecoders[i]; i++) {
            const AVCodec* hwCodec = avcodec_find_decoder_by_name(hwDecoders[i]);
            if (hwCodec && hwCodec->id == codecpar->codec_id) {
                codec = hwCodec;
                break;
            }
        }
    }

    if (!codec) {
        codec = avcodec_find_decoder(codecpar->codec_id);
    }

    if (!codec) {
        return false;
    }

    *codecCtx = avcodec_alloc_context3(codec);
    if (!*codecCtx) {
        return false;
    }

    int ret = avcodec_parameters_to_context(*codecCtx, codecpar);
    if (ret < 0) {
        avcodec_free_context(codecCtx);
        return false;
    }

    (*codecCtx)->pkt_timebase = stream->time_base;

    if (isVideo && mode == HardwareDecode) {
        initHardwareDecoder(*codecCtx, codecpar);
    }

    ret = avcodec_open2(*codecCtx, codec, nullptr);
    if (ret < 0) {
        avcodec_free_context(codecCtx);
        return false;
    }

    return true;
}

bool FFmpegDecoder::initHardwareDecoder(AVCodecContext* codecCtx, AVCodecParameters* codecpar)
{
    Q_UNUSED(codecCtx);
    Q_UNUSED(codecpar);
    return true;
}

void FFmpegDecoder::close()
{
    stop();

    if (m_decodeThread.joinable()) {
        m_decodeThread.join();
    }

    if (m_swsCtx) {
        sws_freeContext(m_swsCtx);
        m_swsCtx = nullptr;
    }

    if (m_swrCtx) {
        swr_free(&m_swrCtx);
        m_swrCtx = nullptr;
    }

    if (m_videoCodecCtx) {
        avcodec_free_context(&m_videoCodecCtx);
        m_videoCodecCtx = nullptr;
    }

    if (m_audioCodecCtx) {
        avcodec_free_context(&m_audioCodecCtx);
        m_audioCodecCtx = nullptr;
    }

    if (m_formatCtx) {
        avformat_close_input(&m_formatCtx);
        m_formatCtx = nullptr;
    }

    m_videoQueue.clear();
    m_audioQueue.clear();

    m_durationMs = 0;
    m_currentPositionMs = 0;
    m_videoStreamIndex = -1;
    m_audioStreamIndex = -1;
    m_videoWidth = 0;
    m_videoHeight = 0;
    m_fps = 0.0;
    m_isPlaying = false;
    m_isPaused = false;
    m_isStopRequested = false;
    m_decodeFinished = false;
}

void FFmpegDecoder::start()
{
    if (!m_formatCtx) return;

    m_isPlaying = true;
    m_isPaused = false;
    m_isStopRequested = false;
    m_decodeFinished = false;

    m_mutex.lock();
    m_startTime = av_gettime() - (qint64)(m_currentPositionMs * 1000 * m_playbackSpeed);
    m_mutex.unlock();

    m_waitCondition.wakeAll();
    emit stateChanged(true);
}

void FFmpegDecoder::pause()
{
    if (!m_isPlaying) return;

    m_isPaused = !m_isPaused;
    if (m_isPaused) {
        m_pauseTime = av_gettime();
        emit stateChanged(false);
    } else {
        m_mutex.lock();
        // 暂停期间补偿时间
        qint64 pausedDuration = av_gettime() - m_pauseTime;
        m_startTime += pausedDuration;
        m_mutex.unlock();
        m_waitCondition.wakeAll();
        emit stateChanged(true);
    }
}

void FFmpegDecoder::stop()
{
    m_isStopRequested = true;
    m_isPlaying = false;
    m_isPaused = false;
    m_waitCondition.wakeAll();

    if (m_decodeThread.joinable()) {
        m_decodeThread.join();
    }
}

void FFmpegDecoder::seek(qint64 positionMs)
{
    QMutexLocker locker(&m_mutex);
    m_isSeekRequested = true;
    m_seekPositionMs = positionMs;
    m_decodeFinished = false;
    m_waitCondition.wakeAll();
}

VideoFrame FFmpegDecoder::takeVideoFrame(qint64 positionMs)
{
    QMutexLocker locker(&m_mutex);
    if (m_videoQueue.isEmpty()) {
        return VideoFrame();
    }

    // 取出所有 PTS <= positionMs 的帧，返回最后一帧（跳帧策略）
    VideoFrame result;
    while (!m_videoQueue.isEmpty()) {
        const VideoFrame& head = m_videoQueue.head();
        if (head.pts <= positionMs + 30) {
            result = m_videoQueue.dequeue();
        } else {
            break;
        }
    }
    return result;
}

AudioFrame FFmpegDecoder::currentAudioFrame()
{
    QMutexLocker locker(&m_mutex);
    if (m_audioQueue.isEmpty()) {
        return AudioFrame();
    }
    AudioFrame frame = m_audioQueue.dequeue();
    return frame;
}

bool FFmpegDecoder::isQueueEmpty() const
{
    QMutexLocker locker(&m_mutex);
    return m_videoQueue.isEmpty() && m_audioQueue.isEmpty();
}

void FFmpegDecoder::setPlaybackSpeed(double speed)
{
    QMutexLocker locker(&m_mutex);
    qint64 currentPos = m_currentPositionMs;
    m_playbackSpeed = speed;
    m_startTime = av_gettime() - (qint64)(currentPos * 1000 * speed);
}

void FFmpegDecoder::updatePosition()
{
    QMutexLocker locker(&m_mutex);
    if (m_isPaused || !m_isPlaying) return;

    qint64 elapsed = (av_gettime() - m_startTime) / 1000;
    qint64 newPos = (qint64)(elapsed / m_playbackSpeed);
    if (newPos > m_currentPositionMs && newPos <= m_durationMs) {
        m_currentPositionMs = newPos;
    } else if (newPos > m_durationMs) {
        m_currentPositionMs = m_durationMs;
    }
}

void FFmpegDecoder::decodeLoop()
{
    if (!m_formatCtx) return;

    AVPacket* pkt = av_packet_alloc();
    AVFrame* videoFrame = av_frame_alloc();
    AVFrame* audioFrame = av_frame_alloc();

    QQueue<VideoFrame> videoQueue;
    QQueue<AudioFrame> audioQueue;

    m_isStopRequested = false;
    m_decodeFinished = false;

    while (!m_isStopRequested) {
        m_mutex.lock();
        if (m_isPaused) {
            m_mutex.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }
        m_mutex.unlock();

        m_mutex.lock();
        if (m_isSeekRequested) {
            qint64 seekPos = m_seekPositionMs;
            m_mutex.unlock();

            int64_t target = seekPos * 1000;
            int ret = avformat_seek_file(m_formatCtx, -1, INT64_MIN, target, INT64_MAX, 0);
            if (ret >= 0) {
                if (m_videoCodecCtx) avcodec_flush_buffers(m_videoCodecCtx);
                if (m_audioCodecCtx) avcodec_flush_buffers(m_audioCodecCtx);
                videoQueue.clear();
                audioQueue.clear();
                m_decodeFinished = false;
            }

            m_mutex.lock();
            m_isSeekRequested = false;
            m_currentPositionMs = seekPos;
            m_startTime = av_gettime() - (qint64)(seekPos * 1000 * m_playbackSpeed);
            m_mutex.unlock();
            continue;
        }
        m_mutex.unlock();

        // 检查队列水位，满则等待（控制解码速度）
        m_mutex.lock();
        int vqSize = m_videoQueue.size();
        int aqSize = m_audioQueue.size();
        m_mutex.unlock();

        bool queueFull = false;
        if (m_videoStreamIndex >= 0 && vqSize >= 5) queueFull = true;
        if (m_audioStreamIndex >= 0 && aqSize >= 20) queueFull = true;

        if (queueFull) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // 如果解码已完成，等待队列消费
        if (m_decodeFinished) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            continue;
        }

        int ret = av_read_frame(m_formatCtx, pkt);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                if (m_videoCodecCtx) flushDecoder(m_videoCodecCtx, videoFrame, true, videoQueue, audioQueue);
                if (m_audioCodecCtx) flushDecoder(m_audioCodecCtx, audioFrame, false, videoQueue, audioQueue);

                // 转移剩余帧到主队列
                m_mutex.lock();
                while (!videoQueue.isEmpty() && m_videoQueue.size() < 30) {
                    m_videoQueue.enqueue(videoQueue.dequeue());
                    emit videoFrameReady();
                }
                while (!audioQueue.isEmpty() && m_audioQueue.size() < 50) {
                    m_audioQueue.enqueue(audioQueue.dequeue());
                    emit audioFrameReady();
                }
                m_decodeFinished = true;
                m_mutex.unlock();
                // 不立即emit finished()，由FFmpegPlayer根据position判断
                // 等待队列消费完
                while (!m_isStopRequested) {
                    m_mutex.lock();
                    if (m_videoQueue.isEmpty() && m_audioQueue.isEmpty()) {
                        m_mutex.unlock();
                        break;
                    }
                    m_mutex.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
                break;
            }
            break;
        }

        qint64 currentPts = 0;

        if (pkt->stream_index == m_videoStreamIndex && m_videoCodecCtx) {
            decodePacket(pkt, m_videoCodecCtx, videoFrame, true, videoQueue, audioQueue, currentPts);
        } else if (pkt->stream_index == m_audioStreamIndex && m_audioCodecCtx) {
            decodePacket(pkt, m_audioCodecCtx, audioFrame, false, videoQueue, audioQueue, currentPts);
        }

        av_packet_unref(pkt);

        m_mutex.lock();
        while (!videoQueue.isEmpty() && m_videoQueue.size() < 30) {
            m_videoQueue.enqueue(videoQueue.dequeue());
            emit videoFrameReady();
        }
        while (!audioQueue.isEmpty() && m_audioQueue.size() < 50) {
            m_audioQueue.enqueue(audioQueue.dequeue());
            emit audioFrameReady();
        }
        m_mutex.unlock();
    }

    av_frame_free(&videoFrame);
    av_frame_free(&audioFrame);
    av_packet_free(&pkt);
}

bool FFmpegDecoder::decodePacket(AVPacket* pkt, AVCodecContext* codecCtx, AVFrame* frame,
                                 bool isVideo, QQueue<VideoFrame>& videoQueue,
                                 QQueue<AudioFrame>& audioQueue, qint64& outPts)
{
    int ret = avcodec_send_packet(codecCtx, pkt);
    if (ret < 0) return false;

    outPts = 0;

    while (ret >= 0) {
        ret = avcodec_receive_frame(codecCtx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }
        if (ret < 0) {
            return false;
        }

        qint64 framePts = 0;
        if (frame->pts != AV_NOPTS_VALUE) {
            AVRational tb = codecCtx->pkt_timebase;
            framePts = (qint64)(frame->pts * av_q2d(tb) * 1000);
            outPts = framePts;
        }

        if (isVideo) {
            AVFrame* rgbFrame = av_frame_alloc();
            rgbFrame->format = AV_PIX_FMT_RGB24;
            rgbFrame->width = m_videoWidth;
            rgbFrame->height = m_videoHeight;
            av_frame_get_buffer(rgbFrame, 32);

            if (m_swsCtx) {
                sws_scale(m_swsCtx, frame->data, frame->linesize, 0,
                          m_videoHeight, rgbFrame->data, rgbFrame->linesize);
            }

            QImage img(rgbFrame->data[0], m_videoWidth, m_videoHeight,
                       rgbFrame->linesize[0], QImage::Format_RGB888);

            VideoFrame vf;
            vf.image = img.copy();
            vf.pts = framePts;

            videoQueue.enqueue(vf);
            av_frame_free(&rgbFrame);
        } else {
            if (m_swrCtx) {
                uint8_t* outBuffer = nullptr;
                int outSamples = swr_get_out_samples(m_swrCtx, frame->nb_samples);
                av_samples_alloc(&outBuffer, nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 0);

                int converted = swr_convert(m_swrCtx, &outBuffer, outSamples,
                                            (const uint8_t**)frame->data, frame->nb_samples);

                if (converted > 0) {
                    AudioFrame af;
                    af.data = QByteArray((const char*)outBuffer, converted * 2 * 2);
                    af.pts = framePts;
                    audioQueue.enqueue(af);
                }
                av_freep(&outBuffer);
            }
        }

        av_frame_unref(frame);
    }

    return true;
}

void FFmpegDecoder::flushDecoder(AVCodecContext* codecCtx, AVFrame* frame, bool isVideo,
                                 QQueue<VideoFrame>& videoQueue, QQueue<AudioFrame>& audioQueue)
{
    int ret = avcodec_send_packet(codecCtx, nullptr);
    if (ret < 0) return;

    while (ret >= 0) {
        ret = avcodec_receive_frame(codecCtx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) break;

        if (isVideo) {
            AVFrame* rgbFrame = av_frame_alloc();
            rgbFrame->format = AV_PIX_FMT_RGB24;
            rgbFrame->width = m_videoWidth;
            rgbFrame->height = m_videoHeight;
            av_frame_get_buffer(rgbFrame, 32);

            if (m_swsCtx) {
                sws_scale(m_swsCtx, frame->data, frame->linesize, 0,
                          m_videoHeight, rgbFrame->data, rgbFrame->linesize);
            }

            QImage img(rgbFrame->data[0], m_videoWidth, m_videoHeight,
                       rgbFrame->linesize[0], QImage::Format_RGB888);

            VideoFrame vf;
            vf.image = img.copy();
            vf.pts = m_durationMs;
            videoQueue.enqueue(vf);
            av_frame_free(&rgbFrame);
        }

        av_frame_unref(frame);
    }
}
