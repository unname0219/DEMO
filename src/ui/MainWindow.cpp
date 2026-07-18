#include "ui/MainWindow.h"
#include "ui/HeaderBar.h"
#include "ui/MediaViewer.h"
#include "ui/PlaybackControls.h"
#include "ui/ProgressBar.h"
#include "ui/VolumeSlider.h"
#include "ui/SpeedSelector.h"
#include "ui/SettingsPanel.h"
#include "core/FileSignatureDetector.h"
#include "managers/ThemeManager.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include "utils/FormatUtils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QKeyEvent>
#include <QMessageBox>
#include <QShortcut>
#include <QDir>
#include <QFileInfo>
#include <QMouseEvent>
#include <QWindow>
#include <QApplication>
#include <QPainter>

static const int kResizeBorder = 6; // 边缘可拉伸区域宽度（像素）

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_headerBar(nullptr)
    , m_mediaViewer(nullptr)
    , m_playbackControls(nullptr)
    , m_progressBar(nullptr)
    , m_volumeSlider(nullptr)
    , m_speedSelector(nullptr)
    , m_settingsPanel(nullptr)
    , m_playerController(nullptr)
    , m_controlBar(nullptr)
    , m_isFullScreen(false)
{
    m_playerController = new PlayerController(this);

    // 去掉 Windows 原生标题栏，使用自定义标题栏
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    setAcceptDrops(true);
    setMouseTracking(true);
    setMinimumSize(DPIAdapter::scaledSize(800), DPIAdapter::scaledSize(600));
    resize(DPIAdapter::scaledSize(1024), DPIAdapter::scaledSize(768));

    setupUI();
    setupConnections();
    setupShortcuts();

    updateWindowTitle(QString());
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setMouseTracking(true);
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_headerBar = new HeaderBar(this);
    mainLayout->addWidget(m_headerBar);

    m_mediaViewer = new MediaViewer(this);
    mainLayout->addWidget(m_mediaViewer, 1);

    m_progressBar = new ProgressBar(this);
    mainLayout->addWidget(m_progressBar);

    // 控制栏布局：设置 | stretch | 上一个 | 快退 | 播放(居中) | 快进 | 下一个 | stretch | 倍速 | 音量
    m_controlBar = new QWidget(this);
    m_controlBar->setFixedHeight(DPIAdapter::scaledSize(52));
    m_controlBar->setMouseTracking(true);
    QHBoxLayout* controlLayout = new QHBoxLayout(m_controlBar);
    controlLayout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(12), 0);
    controlLayout->setSpacing(DPIAdapter::scaledSize(8));

    // 设置按钮（居左，图标化）
    QPushButton* settingsBtn = new QPushButton(m_controlBar);
    settingsBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(34));
    settingsBtn->setIconSize(QSize(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(20)));
    settingsBtn->setIcon(IconManager::instance()->icon("settings"));
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setToolTip("设置");
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::toggleSettings);
    // 主题变化时刷新设置按钮图标
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [settingsBtn]() {
        settingsBtn->setIcon(IconManager::instance()->icon("settings"));
    });
    controlLayout->addWidget(settingsBtn);

    // 左侧弹性，把播放控制组推到中间
    controlLayout->addStretch();

    m_playbackControls = new PlaybackControls(m_controlBar);
    controlLayout->addWidget(m_playbackControls);

    // 右侧弹性，让倍速/音量靠右
    controlLayout->addStretch();

    m_speedSelector = new SpeedSelector(m_controlBar);
    controlLayout->addWidget(m_speedSelector);

    m_volumeSlider = new VolumeSlider(m_controlBar);
    controlLayout->addWidget(m_volumeSlider);

    mainLayout->addWidget(m_controlBar);

    // 设置面板：居中对话框模式，modal 防止与主窗口交互冲突
    m_settingsPanel = new SettingsPanel(this);
    m_settingsPanel->setVisible(false);
}

void MainWindow::setupConnections()
{
    m_mediaViewer->installEventFilter(this);
    m_progressBar->installEventFilter(this);
    m_controlBar->installEventFilter(this);

    connect(m_headerBar, &HeaderBar::openFileClicked, this, &MainWindow::openFileDialog);
    connect(m_headerBar, &HeaderBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_headerBar, &HeaderBar::maximizeClicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    connect(m_headerBar, &HeaderBar::closeClicked, this, &QMainWindow::close);

    // 初始化最大化按钮状态
    m_headerBar->updateMaximizeIcon(isMaximized());

    connect(m_playbackControls, &PlaybackControls::playToggled,
            m_playerController, &PlayerController::togglePlayPause);
    // 快退/快进 → 进度跳转
    connect(m_playbackControls, &PlaybackControls::rewindClicked,
            m_playerController, [this]() { m_playerController->seekBackward(10); });
    connect(m_playbackControls, &PlaybackControls::forwardClicked,
            m_playerController, [this]() { m_playerController->seekForward(10); });
    // 上一个/下一个 → 切换文件
    connect(m_playbackControls, &PlaybackControls::previousClicked,
            this, [this]() { playAdjacentFile(-1); });
    connect(m_playbackControls, &PlaybackControls::nextClicked,
            this, [this]() { playAdjacentFile(1); });

    connect(m_playerController, &PlayerController::playbackStateChanged,
            m_playbackControls, &PlaybackControls::onPlaybackStateChanged);
    connect(m_playerController, &PlayerController::positionChanged,
            m_progressBar, &ProgressBar::onPositionChanged);
    connect(m_playerController, &PlayerController::durationChanged,
            m_progressBar, &ProgressBar::onDurationChanged);
    connect(m_progressBar, &ProgressBar::seekRequested,
            m_playerController, &PlayerController::setPosition);

    connect(m_volumeSlider, &VolumeSlider::volumeChanged,
            m_playerController, &PlayerController::setVolume);
    connect(m_volumeSlider, &VolumeSlider::muteToggled,
            m_playerController, &PlayerController::toggleMute);
    connect(m_playerController, &PlayerController::volumeChanged,
            m_volumeSlider, &VolumeSlider::onVolumeChanged);
    connect(m_playerController, &PlayerController::mutedChanged,
            m_volumeSlider, &VolumeSlider::onMutedChanged);

    connect(m_speedSelector, &SpeedSelector::speedChanged,
            m_playerController, &PlayerController::setPlaybackSpeed);
    connect(m_playerController, &PlayerController::playbackSpeedChanged,
            m_speedSelector, &SpeedSelector::onSpeedChanged);

    connect(m_playerController, &PlayerController::volumeBoostRequested,
            this, &MainWindow::onVolumeBoostRequested);

    // 图片缩放模式立即生效
    connect(m_settingsPanel, &SettingsPanel::imageScalingChanged,
            m_mediaViewer, &MediaViewer::setSmoothScaling);

    connect(ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { update(); });
}

void MainWindow::setupShortcuts()
{
    QShortcut* spaceShortcut = new QShortcut(QKeySequence(Qt::Key_Space), this);
    connect(spaceShortcut, &QShortcut::activated, m_playerController, &PlayerController::togglePlayPause);

    QShortcut* leftShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(leftShortcut, &QShortcut::activated, m_playerController, [this]() {
        m_playerController->seekBackward(5);
    });

    QShortcut* rightShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(rightShortcut, &QShortcut::activated, m_playerController, [this]() {
        m_playerController->seekForward(5);
    });

    QShortcut* upShortcut = new QShortcut(QKeySequence(Qt::Key_Up), this);
    connect(upShortcut, &QShortcut::activated, m_playerController, [this]() {
        m_playerController->setVolume(m_playerController->volume() + 10);
    });

    QShortcut* downShortcut = new QShortcut(QKeySequence(Qt::Key_Down), this);
    connect(downShortcut, &QShortcut::activated, m_playerController, [this]() {
        m_playerController->setVolume(m_playerController->volume() - 10);
    });

    QShortcut* mShortcut = new QShortcut(QKeySequence(Qt::Key_M), this);
    connect(mShortcut, &QShortcut::activated, m_playerController, &PlayerController::toggleMute);

    QShortcut* fShortcut = new QShortcut(QKeySequence(Qt::Key_F), this);
    connect(fShortcut, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    QShortcut* fShortcut11 = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fShortcut11, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    QShortcut* escapeShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escapeShortcut, &QShortcut::activated, this, [this]() {
        if (m_isFullScreen) toggleFullScreen();
    });
}

void MainWindow::openFileDialog()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("打开媒体文件"),
        QString(),
        tr("媒体文件 (*.mp4 *.mkv *.avi *.mov *.webm *.flv "
           "*.mp3 *.wav *.flac *.aac *.ogg *.m4a "
           "*.jpg *.jpeg *.png *.gif *.bmp *.webp);;"
           "视频文件 (*.mp4 *.mkv *.avi *.mov *.webm *.flv);;"
           "音频文件 (*.mp3 *.wav *.flac *.aac *.ogg *.m4a);;"
           "图片文件 (*.jpg *.jpeg *.png *.gif *.bmp *.webp);;"
           "所有文件 (*.*)")
    );

    if (!filePath.isEmpty()) {
        openFile(filePath);
    }
}

void MainWindow::openFile(const QString& filePath)
{
    MediaType type = FileSignatureDetector::detectMediaType(filePath);

    if (type == MediaType::Unknown) {
        QMessageBox::warning(this, tr("未知格式"), tr("无法识别的文件格式。"));
        return;
    }

    m_currentFile = filePath;
    updateWindowTitle(QFileInfo(filePath).fileName());

    m_mediaViewer->showMedia(filePath, type, m_playerController);
    updateLayoutForMediaType(type);
}

void MainWindow::updateLayoutForMediaType(MediaType type)
{
    // 图片模式下隐藏进度条与播放控制（图片不需要进度条和播放控制）
    bool isImage = (type == MediaType::Image);
    m_progressBar->setVisible(!isImage);
    m_playbackControls->setVisible(!isImage);
    m_speedSelector->setVisible(!isImage);
    m_volumeSlider->setVisible(!isImage);
}

void MainWindow::toggleSettings()
{
    if (m_settingsPanel->isVisible()) {
        m_settingsPanel->hide();
    } else {
        m_settingsPanel->move((width() - m_settingsPanel->width()) / 2,
                              (height() - m_settingsPanel->height()) / 2);
        m_settingsPanel->show();
        m_settingsPanel->raise();
        m_settingsPanel->activateWindow();
    }
}

void MainWindow::repositionSettingsPanel()
{
    if (!m_settingsPanel) return;
    int w = DPIAdapter::scaledSize(380);
    int h = height() - m_headerBar->height() - DPIAdapter::scaledSize(70);
    if (h < DPIAdapter::scaledSize(300)) h = DPIAdapter::scaledSize(300);
    // 相对主窗口左上角定位（全局坐标）
    QPoint topLeft = mapToGlobal(QPoint(DPIAdapter::scaledSize(16),
                                        m_headerBar->height() + DPIAdapter::scaledSize(8)));
    m_settingsPanel->setGeometry(topLeft.x(), topLeft.y(), w, h);
}

void MainWindow::toggleFullScreen()
{
    if (m_isFullScreen) {
        showNormal();
        m_headerBar->show();
        m_isFullScreen = false;
    } else {
        showFullScreen();
        m_headerBar->hide();
        m_isFullScreen = true;
    }
}

void MainWindow::onVolumeBoostRequested()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("音量增强"),
        tr("音量已达到100%，是否继续增加至最高500%？\n注意：过高的音量可能会损伤听力或设备。"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        m_playerController->setVolumeBoostEnabled(true);
    }
}

void MainWindow::playAdjacentFile(int direction)
{
    if (m_currentFile.isEmpty()) return;

    // 图片模式交给 ImageViewer 自身处理同目录切换
    if (m_mediaViewer->currentMediaType() == MediaType::Image) {
        if (direction < 0) {
            // 通过快捷键触发；这里复用文件级查找兜底
        }
        return;
    }

    QString next = findAdjacentFile(m_currentFile, direction);
    if (!next.isEmpty()) {
        openFile(next);
    }
}

QString MainWindow::findAdjacentFile(const QString& currentPath, int direction)
{
    QFileInfo currentFi(currentPath);
    QDir dir = currentFi.dir();
    QStringList files = dir.entryList(QDir::Files, QDir::Name);

    // 仅保留通过文件头识别为媒体文件的项
    QStringList mediaFiles;
    foreach (const QString& name, files) {
        QString full = dir.filePath(name);
        MediaType t = FileSignatureDetector::detectMediaType(full);
        if (t == MediaType::Video || t == MediaType::Audio) {
            mediaFiles.append(full);
        }
    }

    int idx = mediaFiles.indexOf(currentFi.absoluteFilePath());
    if (idx < 0) return QString();

    int nextIdx = idx + direction;
    if (nextIdx < 0 || nextIdx >= mediaFiles.size()) return QString();
    return mediaFiles[nextIdx];
}

void MainWindow::updateWindowTitle(const QString& fileName)
{
    if (fileName.isEmpty()) {
        setWindowTitle("Firefly Player");
        m_headerBar->setTitle("Firefly");
    } else {
        setWindowTitle(QString("%1 - Firefly Player").arg(fileName));
        m_headerBar->setTitle(fileName);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString filePath = urls.first().toLocalFile();
        if (!filePath.isEmpty()) {
            openFile(filePath);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (m_settingsPanel && m_settingsPanel->isVisible()) {
        m_settingsPanel->move((width() - m_settingsPanel->width()) / 2,
                              (height() - m_settingsPanel->height()) / 2);
    }
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QMainWindow::moveEvent(event);
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor bg = ThemeManager::instance()->backgroundColor();
    painter.setBrush(QBrush(bg));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        m_headerBar->updateMaximizeIcon(isMaximized());
    }
    QMainWindow::changeEvent(event);
}

// 无边框窗口的边缘拉伸：检测鼠标是否在窗口边缘，触发系统级缩放
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && !isMaximized() && !m_isFullScreen) {
        QWindow* w = windowHandle();
        if (w) {
            int x = event->position().x();
            int y = event->position().y();
            int bw = kResizeBorder;
            Qt::Edges edges;
            if (x < bw) edges |= Qt::LeftEdge;
            if (x > width() - bw) edges |= Qt::RightEdge;
            if (y < bw) edges |= Qt::TopEdge;
            if (y > height() - bw) edges |= Qt::BottomEdge;
            if (edges) {
                w->startSystemResize(edges);
                return;
            }
        }
    }
    QMainWindow::mousePressEvent(event);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::MouseMove && !isMaximized() && !m_isFullScreen) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint globalPos = mouseEvent->globalPosition().toPoint();
        QPoint localPos = mapFromGlobal(globalPos);
        updateResizeCursor(localPos.x(), localPos.y());
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::updateResizeCursor(int x, int y)
{
    int bw = DPIAdapter::scaledSize(kResizeBorder);
    bool left = x >= 0 && x < bw;
    bool right = x <= width() && x > width() - bw;
    bool top = y >= 0 && y < bw;
    bool bottom = y <= height() && y > height() - bw;
    if ((left && top) || (right && bottom)) setCursor(Qt::SizeFDiagCursor);
    else if ((right && top) || (left && bottom)) setCursor(Qt::SizeBDiagCursor);
    else if (left || right) setCursor(Qt::SizeHorCursor);
    else if (top || bottom) setCursor(Qt::SizeVerCursor);
    else setCursor(Qt::ArrowCursor);
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (!isMaximized() && !m_isFullScreen) {
        updateResizeCursor(event->position().x(), event->position().y());
    }
    QMainWindow::mouseMoveEvent(event);
}
