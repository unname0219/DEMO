#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QImage>
#include <QStringList>

class QLabel;
class QPushButton;

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer();

    void loadImage(const QString& filePath);
    void setSmoothScaling(bool enabled);
    bool smoothScaling() const;

protected:
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void showPrevious();
    void showNext();
    void zoomIn();
    void zoomOut();
    void resetZoom();

private:
    void setupUI();
    void updateDisplay();
    void scanFolder();
    void updateNavButtons();

    QLabel* m_imageLabel;
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;

    QImage m_originalImage;
    double m_scaleFactor;
    bool m_smoothScaling;

    QString m_currentPath;
    QStringList m_imageFiles;
    int m_currentIndex;
};

#endif
