#include "ui/AudioPlayer.h"
#include "core/PlayerController.h"
#include "managers/ThemeManager.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>

AudioPlayer::AudioPlayer(QWidget* parent)
    : QWidget(parent)
    , m_rotationTimer(nullptr)
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
    setMinimumSize(DPIAdapter::scaledSize(300), DPIAdapter::scaledSize(300));
    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &AudioPlayer::showDefaultCover);
}

int AudioPlayer::discSize() const
{
    int size = qMin(width(), height()) * 0.6;
    if (size < DPIAdapter::scaledSize(120)) size = DPIAdapter::scaledSize(120);
    if (size > DPIAdapter::scaledSize(400)) size = DPIAdapter::scaledSize(400);
    return size;
}

void AudioPlayer::showDefaultCover()
{
    int size = DPIAdapter::scaledSize(280);
    QPixmap disc(size, size);
    disc.fill(Qt::transparent);
    QPainter painter(&disc);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF discRect(0, 0, size, size);

    // 绘制圆形背景
    QString primary = ThemeManager::instance()->primaryColor();
    QRadialGradient gradient(discRect.center(), size / 2.0);
    gradient.setColorAt(0.0, QColor(primary).lighter(130));
    gradient.setColorAt(0.7, QColor(primary).lighter(110));
    gradient.setColorAt(1.0, QColor(primary).darker(120));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(discRect);

    // 绘制音符图标
    QIcon musicIcon = IconManager::instance()->coloredIcon("music", QColor(255, 255, 255));
    int iconSize = size * 0.45;
    QPixmap iconPixmap = musicIcon.pixmap(iconSize, iconSize);
    QPointF iconPos((size - iconSize) / 2.0, (size - iconSize) / 2.0);
    painter.drawPixmap(iconPos, iconPixmap);

    m_defaultCover = disc;
    update();
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

    QRadialGradient gradient(discRect.center(), size / 2.0);
    gradient.setColorAt(0.0, QColor(40, 40, 40));
    gradient.setColorAt(0.7, QColor(25, 25, 25));
    gradient.setColorAt(0.85, QColor(15, 15, 15));
    gradient.setColorAt(1.0, QColor(5, 5, 5));
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(discRect);

    painter.setPen(QColor(60, 60, 60, 80));
    for (int i = 0; i < 12; i++) {
        qreal ratio = 0.78 + i * 0.015;
        if (ratio > 0.95) break;
        qreal r = size / 2.0 * ratio;
        painter.drawEllipse(discRect.center(), r, r);
    }

    qreal coverSize = size * 0.7;
    QRectF coverRect((size - coverSize) / 2.0, (size - coverSize) / 2.0, coverSize, coverSize);

    QPixmap scaledCover = pixmap.scaled(coverSize, coverSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPainterPath coverPath;
    coverPath.addEllipse(coverRect);
    painter.setClipPath(coverPath);
    if (scaledCover.width() > scaledCover.height()) {
        int xOffset = (scaledCover.width() - coverSize) / 2;
        painter.drawPixmap(coverRect.topLeft(), scaledCover.copy(xOffset, 0, coverSize, coverSize));
    } else {
        int yOffset = (scaledCover.height() - coverSize) / 2;
        painter.drawPixmap(coverRect.topLeft(), scaledCover.copy(0, yOffset, coverSize, coverSize));
    }
    painter.setClipping(false);

    qreal holeSize = size * 0.06;
    QRectF holeRect((size - holeSize) / 2.0, (size - holeSize) / 2.0, holeSize, holeSize);
    painter.setBrush(QColor(20, 20, 20));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(holeRect);

    m_defaultCover = disc;
    update();
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

void AudioPlayer::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), ThemeManager::instance()->backgroundColor());

    if (m_defaultCover.isNull()) return;

    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    int size = discSize();
    QPoint center = rect().center();

    painter.save();
    painter.translate(center);
    painter.rotate(m_rotationAngle);
    painter.translate(-center);

    QPixmap scaled = m_defaultCover.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPoint topLeft(center.x() - size / 2, center.y() - size / 2);
    painter.drawPixmap(topLeft, scaled);

    painter.restore();
}

void AudioPlayer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update();
}
