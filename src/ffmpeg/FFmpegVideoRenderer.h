#ifndef FFMPEGVIDEORENDERER_H
#define FFMPEGVIDEORENDERER_H

#include <QWidget>
#include <QImage>
#include <QTimer>

class FFmpegDecoder;

class FFmpegVideoRenderer : public QWidget
{
    Q_OBJECT

public:
    explicit FFmpegVideoRenderer(QWidget* parent = nullptr);
    ~FFmpegVideoRenderer();

    void setDecoder(FFmpegDecoder* decoder);
    void clearFrame();

signals:
    void frameDisplayed(qint64 pts);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onVideoFrameReady();
    void updateFrame();

private:
    FFmpegDecoder* m_decoder;
    QImage m_currentFrame;
    QTimer* m_updateTimer;
    qint64 m_lastPts;
};

#endif
