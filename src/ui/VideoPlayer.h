#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>

class QVideoWidget;
class QStackedLayout;
class PlayerController;
class FFmpegVideoRenderer;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent = nullptr);
    ~VideoPlayer();

    void setMediaPlayer(PlayerController* controller);
    void setVideoScalingMode(Qt::AspectRatioMode mode);
    QVideoWidget* videoWidget() const { return m_videoWidget; }

private slots:
    void onDecoderBackendChanged();

private:
    void setupUI();
    void updateActiveRenderer();

    QVideoWidget* m_videoWidget;
    QStackedLayout* m_stackedLayout;
    FFmpegVideoRenderer* m_ffmpegRenderer;
    PlayerController* m_controller;
    Qt::AspectRatioMode m_scalingMode;
};

#endif
