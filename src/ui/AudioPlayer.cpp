#include "ui/AudioPlayer.h"
#include "core/PlayerController.h"
#include "managers/ThemeManager.h"
#include "managers/DPIAdapter.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QGraphicsRotation>

AudioPlayer::AudioPlayer(QWidget* parent)
    : QWidget(parent)
    , m_discLabel(nullptr)
    , m_rotationTimer(nullptr)
    , m_rotationAnimation(nullptr)
    , m_controller(nullptr)
    , m_rotationAngle(0.0)
    , m_isPlaying(false)
{
    setupUI();
}

AudioPlayer::~AudioPlayer()
{
}

void AudioPlayer::setupUI()
{
    setStyleSheet("background-color: #1a1a1a;");

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    m_discLabel = new QLabel(this);
    m_discLabel->setAlignment(Qt::AlignCenter);
    m_discLabel->setFixedSize(DPIAdapter::scaledSize(280), DPIAdapter::scaledSize(280));
    layout->addWidget(m_discLabel, 0, Qt::AlignCenter);

    m_rotationTimer = new QTimer(this);
    m_rotationTimer->setInterval(50);
    connect(m_rotationTimer, &QTimer::timeout, this, [this]() {
        if (m_isPlaying) {
            m_rotationAngle += 1.5;
            if (m_rotationAngle >= 360.0) {
                m_rotationAngle -= 360.0;
            }
            update();
        }
    });

    showDefaultCover();
}

void AudioPlayer::showDefaultCover()
{
    int size = DPIAdapter::scaledSize(280);
    QPixmap disc(size, size);
    disc.fill(Qt::transparent);
    QPainter painter(&disc);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF discRect(0, 0, size, size);

    // 唱片外圈（黑胶效果）
    QRadialGradient gradient(discRect.center(), size / 2.0);
    gradient.setColorAt(0.0, QColor(40, 40, 40));
    gradient.setColorAt(0.7, QColor(25, 25, 25));
    gradient.setColorAt(0.85, QColor(15, 15, 15));
    gradient.setColorAt(1.0, QColor(5, 5, 5));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(discRect);

    // 唱片纹理
    painter.setPen(QColor(60, 60, 60, 80));
    for (int i = 0; i < 15; i++) {
        qreal ratio = 0.3 + i * 0.04;
        if (ratio > 0.85) break;
        qreal r = size / 2.0 * ratio;
        painter.drawEllipse(discRect.center(), r, r);
    }

    // 中心标签（音符图标位置）
    qreal labelSize = size * 0.35;
    QRectF labelRect((size - labelSize) / 2.0, (size - labelSize) / 2.0, labelSize, labelSize);
    QRadialGradient labelGrad(labelRect.center(), labelSize / 2.0);
    labelGrad.setColorAt(0.0, QColor(0, 212, 170));
    labelGrad.setColorAt(1.0, QColor(0, 160, 130));
    painter.setBrush(labelGrad);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(labelRect);

    // 中心孔
    qreal holeSize = size * 0.06;
    QRectF holeRect((size - holeSize) / 2.0, (size - holeSize) / 2.0, holeSize, holeSize);
    painter.setBrush(QColor(20, 20, 20));
    painter.drawEllipse(holeRect);

    m_defaultCover = disc;
    updateDiscDisplay();
}

void AudioPlayer::setCoverImage(const QPixmap& pixmap)
{
    if (pixmap.isNull()) {
        showDefaultCover();
        return;
    }

    int size = DPIAdapter::scaledSize(280);
    QPixmap disc(size, size);
    disc.fill(Qt::transparent);
    QPainter painter(&disc);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QRectF discRect(0, 0, size, size);

    // 黑胶外圈
    QRadialGradient gradient(discRect.center(), size / 2.0);
    gradient.setColorAt(0.0, QColor(40, 40, 40));
    gradient.setColorAt(0.7, QColor(25, 25, 25));
    gradient.setColorAt(0.85, QColor(15, 15, 15));
    gradient.setColorAt(1.0, QColor(5, 5, 5));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(discRect);

    // 唱片纹理
    painter.setPen(QColor(60, 60, 60, 80));
    for (int i = 0; i < 12; i++) {
        qreal ratio = 0.78 + i * 0.015;
        if (ratio > 0.95) break;
        qreal r = size / 2.0 * ratio;
        painter.drawEllipse(discRect.center(), r, r);
    }

    // 专辑封面（圆形，中间区域）
    qreal coverSize = size * 0.7;
    QRectF coverRect((size - coverSize) / 2.0, (size - coverSize) / 2.0, coverSize, coverSize);

    QPixmap scaledCover = pixmap.scaled(coverSize, coverSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPainterPath coverPath;
    coverPath.addEllipse(coverRect);
    painter.setClipPath(coverPath);
    QRectF scaledCoverRect(coverRect.x(), coverRect.y(), coverSize, coverSize);
    if (scaledCover.width() > scaledCover.height()) {
        int xOffset = (scaledCover.width() - coverSize) / 2;
        painter.drawPixmap(coverRect.topLeft(), scaledCover.copy(xOffset, 0, coverSize, coverSize));
    } else {
        int yOffset = (scaledCover.height() - coverSize) / 2;
        painter.drawPixmap(coverRect.topLeft(), scaledCover.copy(0, yOffset, coverSize, coverSize));
    }
    painter.setClipping(false);

    // 中心孔
    qreal holeSize = size * 0.06;
    QRectF holeRect((size - holeSize) / 2.0, (size - holeSize) / 2.0, holeSize, holeSize);
    painter.setBrush(QColor(20, 20, 20));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(holeRect);

    m_defaultCover = disc;
    updateDiscDisplay();
}

void AudioPlayer::setMediaPlayer(PlayerController* controller)
{
    if (!controller) return;
    m_controller = controller;
    m_rotationTimer->start();
}

void AudioPlayer::onPlaybackStateChanged(bool playing)
{
    m_isPlaying = playing;
}

void AudioPlayer::updateDiscSize()
{
    int size = qMin(width(), height()) * 0.6;
    if (size < DPIAdapter::scaledSize(120)) size = DPIAdapter::scaledSize(120);
    if (size > DPIAdapter::scaledSize(400)) size = DPIAdapter::scaledSize(400);
    m_discLabel->setFixedSize(size, size);
}

void AudioPlayer::updateDiscDisplay()
{
    if (m_defaultCover.isNull()) return;
    m_discLabel->setPixmap(m_defaultCover);
}

void AudioPlayer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), ThemeManager::instance()->backgroundColor());

    if (m_defaultCover.isNull()) return;

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    int discSize = m_discLabel->width();
    QPoint center = rect().center();
    QPoint discTopLeft(center.x() - discSize / 2, center.y() - discSize / 2);

    painter.save();
    painter.translate(center);
    painter.rotate(m_rotationAngle);
    painter.translate(-center);
    painter.drawPixmap(discTopLeft, m_defaultCover.scaled(discSize, discSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    painter.restore();
}

void AudioPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateDiscSize();
}
