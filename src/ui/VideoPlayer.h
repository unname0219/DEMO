#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QMediaPlayer>

class QVideoWidget;
class PlayerController;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent = nullptr);
    ~VideoPlayer();

    void setMediaPlayer(PlayerController* controller);
    void setVideoScalingMode(Qt::AspectRatioMode mode);

private:
    void setupUI();

    QVideoWidget* m_videoWidget;
    PlayerController* m_controller;
    Qt::AspectRatioMode m_scalingMode;
};

#endif
