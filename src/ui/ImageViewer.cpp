#include "ui/ImageViewer.h"
#include "managers/DPIAdapter.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDir>
#include <QFileInfo>
#include <QWheelEvent>
#include <QPainter>
#include <QSettings>

ImageViewer::ImageViewer(QWidget* parent)
    : QWidget(parent)
    , m_imageLabel(nullptr)
    , m_prevBtn(nullptr)
    , m_nextBtn(nullptr)
    , m_scaleFactor(1.0)
    , m_smoothScaling(true)
    , m_currentIndex(0)
    , m_isDragging(false)
    , m_imageOffset(QPoint(0, 0))
{
    QSettings settings;
    m_smoothScaling = settings.value("image/smoothScaling", true).toBool();
    setupUI();
}

ImageViewer::~ImageViewer()
{
}

void ImageViewer::setupUI()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_prevBtn = new QPushButton("◀", this);
    m_prevBtn->setFixedWidth(DPIAdapter::scaledSize(48));
    m_prevBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 24pt; color: rgba(255,255,255,0.3); }"
        "QPushButton:hover { color: rgba(0,212,170,1); }"
    );
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    connect(m_prevBtn, &QPushButton::clicked, this, &ImageViewer::showPrevious);
    mainLayout->addWidget(m_prevBtn);

    m_imageLabel = new QLabel(this);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setStyleSheet("background: transparent;");
    mainLayout->addWidget(m_imageLabel, 1);

    m_nextBtn = new QPushButton("▶", this);
    m_nextBtn->setFixedWidth(DPIAdapter::scaledSize(48));
    m_nextBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; font-size: 24pt; color: rgba(255,255,255,0.3); }"
        "QPushButton:hover { color: rgba(0,212,170,1); }"
    );
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    connect(m_nextBtn, &QPushButton::clicked, this, &ImageViewer::showNext);
    mainLayout->addWidget(m_nextBtn);

    setMouseTracking(true);
}

void ImageViewer::loadImage(const QString& filePath)
{
    if (!QFile::exists(filePath)) return;

    m_currentPath = filePath;
    m_originalImage.load(filePath);
    m_scaleFactor = 1.0;
    scanFolder();
    updateDisplay();
    updateNavButtons();
}

void ImageViewer::scanFolder()
{
    QFileInfo fi(m_currentPath);
    QDir dir = fi.dir();

    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp" << "*.webp"
            << "*.JPG" << "*.JPEG" << "*.PNG" << "*.GIF" << "*.BMP" << "*.WEBP";

    m_imageFiles = dir.entryList(filters, QDir::Files, QDir::Name);
    m_currentIndex = m_imageFiles.indexOf(fi.fileName());
    if (m_currentIndex < 0) m_currentIndex = 0;
}

void ImageViewer::updateDisplay()
{
    if (m_originalImage.isNull()) return;

    int newWidth = qRound(m_originalImage.width() * m_scaleFactor);
    int newHeight = qRound(m_originalImage.height() * m_scaleFactor);

    Qt::TransformationMode mode = m_smoothScaling
        ? Qt::SmoothTransformation
        : Qt::FastTransformation;

    QImage scaledImage = m_originalImage.scaled(newWidth, newHeight, Qt::KeepAspectRatio, mode);
    QPixmap scaled = QPixmap::fromImage(scaledImage);
    m_imageLabel->setPixmap(scaled);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setGeometry(m_imageOffset.x(), m_imageOffset.y(), width(), height());
}

void ImageViewer::updateNavButtons()
{
    m_prevBtn->setEnabled(m_currentIndex > 0);
    m_nextBtn->setEnabled(m_currentIndex < m_imageFiles.size() - 1);
}

void ImageViewer::showPrevious()
{
    if (m_currentIndex <= 0) return;
    m_currentIndex--;
    QFileInfo fi(m_currentPath);
    QString newPath = fi.dir().filePath(m_imageFiles[m_currentIndex]);
    loadImage(newPath);
}

void ImageViewer::showNext()
{
    if (m_currentIndex >= m_imageFiles.size() - 1) return;
    m_currentIndex++;
    QFileInfo fi(m_currentPath);
    QString newPath = fi.dir().filePath(m_imageFiles[m_currentIndex]);
    loadImage(newPath);
}

void ImageViewer::zoomIn()
{
    m_scaleFactor = qMin(m_scaleFactor * 1.2, 10.0);
    updateDisplay();
}

void ImageViewer::zoomOut()
{
    m_scaleFactor = qMax(m_scaleFactor / 1.2, 0.1);
    updateDisplay();
}

void ImageViewer::resetZoom()
{
    m_scaleFactor = 1.0;
    updateDisplay();
}

void ImageViewer::setSmoothScaling(bool enabled)
{
    m_smoothScaling = enabled;
    QSettings settings;
    settings.setValue("image/smoothScaling", enabled);
    updateDisplay();
}

bool ImageViewer::smoothScaling() const
{
    return m_smoothScaling;
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else {
        zoomOut();
    }
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
}

void ImageViewer::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_scaleFactor > 1.0) {
        m_isDragging = true;
        m_lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isDragging) {
        QPoint delta = event->pos() - m_lastMousePos;
        m_imageOffset += delta;
        m_lastMousePos = event->pos();
        updateDisplay();
    }
    QWidget::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QWidget::mouseReleaseEvent(event);
}
