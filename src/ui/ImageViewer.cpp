#include "ui/ImageViewer.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDir>
#include <QFileInfo>
#include <QWheelEvent>
#include <QPainter>
#include <QSettings>
#include <QSlider>

ImageViewer::ImageViewer(QWidget* parent)
    : QWidget(parent)
    , m_prevBtn(nullptr)
    , m_nextBtn(nullptr)
    , m_scaleFactor(1.0)
    , m_smoothScaling(true)
    , m_currentIndex(0)
    , m_isDragging(false)
    , m_imageOffset(QPoint(0, 0))
    , m_rotationAngle(0)
    , m_toolbar(nullptr)
    , m_zoomSlider(nullptr)
    , m_fitBtn(nullptr)
    , m_rotateBtn(nullptr)
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

    m_prevBtn = new QPushButton(this);
    m_prevBtn->setFixedWidth(DPIAdapter::scaledSize(48));
    m_prevBtn->setIconSize(QSize(DPIAdapter::scaledSize(24), DPIAdapter::scaledSize(24)));
    m_prevBtn->setIcon(IconManager::instance()->icon("prev"));
    m_prevBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(0,212,170,15); }"
        "QPushButton:disabled { opacity: 0.3; }"
    );
    m_prevBtn->setCursor(Qt::PointingHandCursor);
    connect(m_prevBtn, &QPushButton::clicked, this, &ImageViewer::showPrevious);
    mainLayout->addWidget(m_prevBtn);

    QWidget* viewport = new QWidget(this);
    viewport->setMouseTracking(true);
    viewport->installEventFilter(this);
    mainLayout->addWidget(viewport, 1);

    m_nextBtn = new QPushButton(this);
    m_nextBtn->setFixedWidth(DPIAdapter::scaledSize(48));
    m_nextBtn->setIconSize(QSize(DPIAdapter::scaledSize(24), DPIAdapter::scaledSize(24)));
    m_nextBtn->setIcon(IconManager::instance()->icon("next"));
    m_nextBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(0,212,170,15); }"
        "QPushButton:disabled { opacity: 0.3; }"
    );
    m_nextBtn->setCursor(Qt::PointingHandCursor);
    connect(m_nextBtn, &QPushButton::clicked, this, &ImageViewer::showNext);
    mainLayout->addWidget(m_nextBtn);

    setMouseTracking(true);
    setupToolbar();
}

void ImageViewer::setupToolbar()
{
    m_toolbar = new QWidget(this);
    m_toolbar->setFixedHeight(DPIAdapter::scaledSize(44));
    m_toolbar->setStyleSheet(
        "QWidget { background-color: rgba(0,0,0,150); border-radius: 8px; }"
    );

    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolbar);
    toolbarLayout->setContentsMargins(DPIAdapter::scaledSize(16), 0, DPIAdapter::scaledSize(16), 0);
    toolbarLayout->setSpacing(DPIAdapter::scaledSize(8));

    m_zoomSlider = new QSlider(Qt::Horizontal, m_toolbar);
    m_zoomSlider->setRange(10, 200);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setFixedWidth(DPIAdapter::scaledSize(150));
    connect(m_zoomSlider, &QSlider::valueChanged, this, &ImageViewer::onZoomSliderChanged);
    toolbarLayout->addWidget(m_zoomSlider);

    m_fitBtn = new QPushButton(m_toolbar);
    m_fitBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_fitBtn->setIconSize(QSize(DPIAdapter::scaledSize(18), DPIAdapter::scaledSize(18)));
    m_fitBtn->setIcon(IconManager::instance()->icon("fit-window"));
    m_fitBtn->setToolTip("适应窗口");
    m_fitBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(255,255,255,20); border-radius: 4px; }"
    );
    m_fitBtn->setCursor(Qt::PointingHandCursor);
    connect(m_fitBtn, &QPushButton::clicked, this, &ImageViewer::fitToWindow);
    toolbarLayout->addWidget(m_fitBtn);

    m_rotateBtn = new QPushButton(m_toolbar);
    m_rotateBtn->setFixedSize(DPIAdapter::scaledSize(32), DPIAdapter::scaledSize(32));
    m_rotateBtn->setIconSize(QSize(DPIAdapter::scaledSize(18), DPIAdapter::scaledSize(18)));
    m_rotateBtn->setIcon(IconManager::instance()->icon("rotate"));
    m_rotateBtn->setToolTip("旋转 90°");
    m_rotateBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(255,255,255,20); border-radius: 4px; }"
    );
    m_rotateBtn->setCursor(Qt::PointingHandCursor);
    connect(m_rotateBtn, &QPushButton::clicked, this, &ImageViewer::rotateImage);
    toolbarLayout->addWidget(m_rotateBtn);

    m_toolbar->hide();
}

void ImageViewer::loadImage(const QString& filePath)
{
    if (!QFile::exists(filePath)) return;

    m_currentPath = filePath;
    m_originalImage.load(filePath);
    m_scaleFactor = 1.0;
    m_imageOffset = QPoint(0, 0);
    m_rotationAngle = 0;
    scanFolder();
    fitToWindow();
    updateZoomSlider();
    update();
    updateNavButtons();
    m_toolbar->show();
    repositionToolbar();
}

void ImageViewer::repositionToolbar()
{
    if (!m_toolbar || m_originalImage.isNull()) return;
    int toolbarW = DPIAdapter::scaledSize(220);
    int toolbarH = DPIAdapter::scaledSize(44);
    int x = (width() - toolbarW) / 2;
    int y = height() - toolbarH - DPIAdapter::scaledSize(16);
    m_toolbar->setGeometry(x, y, toolbarW, toolbarH);
    m_toolbar->raise();
}

void ImageViewer::fitToWindow()
{
    if (m_originalImage.isNull()) return;

    int viewWidth = width() - m_prevBtn->width() - m_nextBtn->width() - DPIAdapter::scaledSize(32);
    int viewHeight = height() - DPIAdapter::scaledSize(60);

    double scaleX = static_cast<double>(viewWidth) / m_originalImage.width();
    double scaleY = static_cast<double>(viewHeight) / m_originalImage.height();
    m_scaleFactor = qMin(scaleX, scaleY);
    m_scaleFactor = qMax(m_scaleFactor, 0.1);

    centerImage();
    updateZoomSlider();
    update();
}

void ImageViewer::rotateImage()
{
    if (m_originalImage.isNull()) return;
    m_rotationAngle = (m_rotationAngle + 90) % 360;

    QTransform transform;
    transform.rotate(m_rotationAngle);
    m_originalImage = m_originalImage.transformed(transform);

    fitToWindow();
}

void ImageViewer::onZoomSliderChanged(int value)
{
    double oldScale = m_scaleFactor;
    m_scaleFactor = value / 100.0;

    if (m_scaleFactor <= 1.0) {
        centerImage();
    } else {
        adjustOffset(rect().center(), oldScale);
    }
    update();
}

void ImageViewer::updateZoomSlider()
{
    if (!m_zoomSlider) return;
    m_zoomSlider->blockSignals(true);
    m_zoomSlider->setValue(static_cast<int>(m_scaleFactor * 100));
    m_zoomSlider->blockSignals(false);
}

void ImageViewer::centerImage()
{
    if (m_originalImage.isNull()) return;

    int viewWidth = width() - m_prevBtn->width() - m_nextBtn->width();
    int viewHeight = height();

    int scaledWidth = qRound(m_originalImage.width() * m_scaleFactor);
    int scaledHeight = qRound(m_originalImage.height() * m_scaleFactor);

    int offsetX = (viewWidth - scaledWidth) / 2 + m_prevBtn->width();
    int offsetY = (viewHeight - scaledHeight) / 2;

    m_imageOffset = QPoint(offsetX, offsetY);
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

void ImageViewer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (m_originalImage.isNull()) {
        painter.setPen(QColor(0x88, 0x88, 0x88));
        painter.drawText(rect(), Qt::AlignCenter, "图片加载失败");
        return;
    }

    Qt::TransformationMode mode = m_smoothScaling
        ? Qt::SmoothTransformation
        : Qt::FastTransformation;

    int scaledWidth = qRound(m_originalImage.width() * m_scaleFactor);
    int scaledHeight = qRound(m_originalImage.height() * m_scaleFactor);

    QImage scaledImage = m_originalImage.scaled(scaledWidth, scaledHeight,
                                                 Qt::KeepAspectRatio, mode);

    painter.drawImage(m_imageOffset, scaledImage);
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

void ImageViewer::zoomIn(const QPoint& mousePos)
{
    double oldScale = m_scaleFactor;
    m_scaleFactor = qMin(m_scaleFactor * 1.2, 2.0);
    adjustOffset(mousePos, oldScale);
    updateZoomSlider();
    update();
}

void ImageViewer::zoomOut(const QPoint& mousePos)
{
    double oldScale = m_scaleFactor;
    m_scaleFactor = qMax(m_scaleFactor / 1.2, 0.1);

    if (m_scaleFactor <= 1.0) {
        m_scaleFactor = 1.0;
        centerImage();
    } else {
        adjustOffset(mousePos, oldScale);
    }
    updateZoomSlider();
    update();
}

void ImageViewer::adjustOffset(const QPoint& mousePos, double oldScale)
{
    if (m_originalImage.isNull()) return;

    double imgX = (mousePos.x() - m_imageOffset.x()) / oldScale;
    double imgY = (mousePos.y() - m_imageOffset.y()) / oldScale;

    int newOffsetX = mousePos.x() - qRound(imgX * m_scaleFactor);
    int newOffsetY = mousePos.y() - qRound(imgY * m_scaleFactor);

    m_imageOffset.setX(newOffsetX);
    m_imageOffset.setY(newOffsetY);
}

void ImageViewer::resetZoom()
{
    m_scaleFactor = 1.0;
    centerImage();
    updateZoomSlider();
    update();
}

void ImageViewer::setSmoothScaling(bool enabled)
{
    m_smoothScaling = enabled;
    QSettings settings;
    settings.setValue("image/smoothScaling", enabled);
    update();
}

bool ImageViewer::smoothScaling() const
{
    return m_smoothScaling;
}

void ImageViewer::wheelEvent(QWheelEvent* event)
{
    if (event->angleDelta().y() > 0) {
        zoomIn(event->position().toPoint());
    } else {
        zoomOut(event->position().toPoint());
    }
}

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    repositionToolbar();
    if (m_scaleFactor <= 1.0 && !m_originalImage.isNull()) {
        fitToWindow();
    }
}

void ImageViewer::keyPressEvent(QKeyEvent* event)
{
    QWidget::keyPressEvent(event);
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
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
        update();
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

bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        QPoint globalPos = wheelEvent->globalPosition().toPoint();
        QPoint localPos = mapFromGlobal(globalPos);
        if (wheelEvent->angleDelta().y() > 0) {
            zoomIn(localPos);
        } else {
            zoomOut(localPos);
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
