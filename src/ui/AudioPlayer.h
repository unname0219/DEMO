#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>

class PlayerController;

class AudioPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit AudioPlayer(QWidget* parent = nullptr);
    ~AudioPlayer();

    void setMediaPlayer(PlayerController* controller);
    void setCoverImage(const QPixmap& pixmap);
    void showDefaultCover();

public slots:
    void onPlaybackStateChanged(bool playing);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    int discSize() const;

    QTimer* m_rotationTimer;
    PlayerController* m_controller;
    qreal m_rotationAngle;
    bool m_isPlaying;
    QPixmap m_defaultCover;
};

#endif
