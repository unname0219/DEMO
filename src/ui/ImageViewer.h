#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QImage>
#include <QStringList>

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
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void showPrevious();
    void showNext();
    void zoomIn(const QPoint& mousePos = QPoint());
    void zoomOut(const QPoint& mousePos = QPoint());
    void resetZoom();

private:
    void setupUI();
    void updateDisplay();
    void scanFolder();
    void updateNavButtons();
    void adjustOffset(const QPoint& mousePos, double oldScale);
    void centerImage();

    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;

    QImage m_originalImage;
    double m_scaleFactor;
    bool m_smoothScaling;

    QString m_currentPath;
    QStringList m_imageFiles;
    int m_currentIndex;

    bool m_isDragging;
    QPoint m_lastMousePos;
    QPoint m_imageOffset;
};

#endif