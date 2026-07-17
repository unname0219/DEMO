#ifndef MEDIAVIEWER_H
#define MEDIAVIEWER_H

#include <QWidget>
#include <QStackedLayout>
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

private:
    void setupUI();

    QStackedLayout* m_stackedLayout;
    VideoPlayer* m_videoPlayer;
    ImageViewer* m_imageViewer;
    QLabel* m_placeholderLabel;
};

#endif
