#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>

class QVideoWidget;
class PlayerController;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget* parent = nullptr);
    ~VideoPlayer();

    void setMediaPlayer(PlayerController* controller);

private:
    void setupUI();

    QVideoWidget* m_videoWidget;
    PlayerController* m_controller;
};

#endif
