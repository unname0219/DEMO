#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QStackedLayout>
#include <QMediaPlayer>
#include "core/FileSignatureDetector.h"

class VideoPlayer;
class ImageViewer;
class QLabel;

class MediaViewer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaViewer(QWidget* parent = nullptr);
    ~MediaViewer();

    void showMedia(const QString& filePath, MediaType type, class PlayerController* controller);
    void clear();

    MediaType currentMediaType() const;
    void setSmoothScaling(bool smooth);
    void setVideoScalingMode(Qt::AspectRatioMode mode);

private:
    void setupUI();

    QStackedLayout* m_stackedLayout;
    VideoPlayer* m_videoPlayer;
    ImageViewer* m_imageViewer;
    QLabel* m_placeholderLabel;
    MediaType m_currentType;
};

#endif
