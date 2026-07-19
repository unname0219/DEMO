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
#include "managers/FileAssociation.h"
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
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QDesktopServices>

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
    , m_hideControlsTimer(nullptr)
    , m_controlBarAnimation(nullptr)
    , m_progressBarAnimation(nullptr)
    , m_isFullScreen(false)
{
    m_playerController = new PlayerController(this);
    m_hideControlsTimer = new QTimer(this);
    m_hideControlsTimer->setInterval(1000);
    m_hideControlsTimer->setSingleShot(true);
    connect(m_hideControlsTimer, &QTimer::timeout, this, &MainWindow::hideControlsAfterTimeout);

    m_controlBarAnimation = new QPropertyAnimation(this);
    m_controlBarAnimation->setDuration(300);
    m_controlBarAnimation->setPropertyName("pos");

    m_progressBarAnimation = new QPropertyAnimation(this);
    m_progressBarAnimation->setDuration(300);
    m_progressBarAnimation->setPropertyName("pos");

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

    checkFileAssociations();

    updateWindowTitle(QString());
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget* centralWidget = new QWidget(this);
    centralWidget->setMouseTracking(true);
    centralWidget->setStyleSheet("QWidget { background: transparent; }");
    setCentralWidget(centralWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8), 
                                   DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
    mainLayout->setSpacing(0);

    m_headerBar = new HeaderBar(this);
    mainLayout->addWidget(m_headerBar);

    m_mediaViewer = new MediaViewer(this);
    mainLayout->addWidget(m_mediaViewer, 1);

    m_progressBar = new ProgressBar(this);
    mainLayout->addWidget(m_progressBar);

    m_controlBar = new QWidget(this);
    m_controlBar->setFixedHeight(DPIAdapter::scaledSize(52));
    m_controlBar->setMouseTracking(true);
    QHBoxLayout* controlLayout = new QHBoxLayout(m_controlBar);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(0);

    QWidget* leftPanel = new QWidget(m_controlBar);
    QHBoxLayout* leftLayout = new QHBoxLayout(leftPanel);
    leftLayout->setContentsMargins(DPIAdapter::scaledSize(12), 0, 0, 0);
    leftLayout->setSpacing(DPIAdapter::scaledSize(8));

    QPushButton* settingsBtn = new QPushButton(leftPanel);
    settingsBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(34));
    settingsBtn->setIconSize(QSize(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(20)));
    settingsBtn->setIcon(IconManager::instance()->icon("settings"));
    settingsBtn->setCursor(Qt::PointingHandCursor);
    settingsBtn->setToolTip("设置");
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::toggleSettings);
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [settingsBtn]() {
        settingsBtn->setIcon(IconManager::instance()->icon("settings"));
    });
    leftLayout->addWidget(settingsBtn);

    controlLayout->addWidget(leftPanel);

    controlLayout->addStretch(1);

    m_playbackControls = new PlaybackControls(m_controlBar);
    controlLayout->addWidget(m_playbackControls, 0, Qt::AlignCenter);

    controlLayout->addStretch(1);

    QWidget* rightPanel = new QWidget(m_controlBar);
    QHBoxLayout* rightLayout = new QHBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, DPIAdapter::scaledSize(12), 0);
    rightLayout->setSpacing(DPIAdapter::scaledSize(8));

    m_speedSelector = new SpeedSelector(rightPanel);
    rightLayout->addWidget(m_speedSelector);

    m_volumeSlider = new VolumeSlider(rightPanel);
    rightLayout->addWidget(m_volumeSlider);

    QPushButton* toolboxBtn = new QPushButton(rightPanel);
    toolboxBtn->setFixedSize(DPIAdapter::scaledSize(34), DPIAdapter::scaledSize(34));
    toolboxBtn->setIconSize(QSize(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(20)));
    toolboxBtn->setIcon(IconManager::instance()->icon("settings"));
    toolboxBtn->setCursor(Qt::PointingHandCursor);
    toolboxBtn->setToolTip("工具箱");
    rightLayout->addWidget(toolboxBtn);
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, this, [toolboxBtn]() {
        toolboxBtn->setIcon(IconManager::instance()->icon("settings"));
    });

    controlLayout->addWidget(rightPanel);

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
    m_headerBar->installEventFilter(this);

    connect(m_headerBar, &HeaderBar::openFileClicked, this, &MainWindow::openFileDialog);
    connect(m_headerBar, &HeaderBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_headerBar, &HeaderBar::maximizeClicked, this, [this]() {
        if (isMaximized()) showNormal();
        else showMaximized();
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

    // 视频缩放模式立即生效
    connect(m_settingsPanel, &SettingsPanel::videoScalingModeChanged,
            m_mediaViewer, &MediaViewer::setVideoScalingMode);

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
        m_settingsPanel->move(mapToGlobal(QPoint(
            (width() - m_settingsPanel->width()) / 2,
            (height() - m_settingsPanel->height()) / 2
        )));
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
        setContentsMargins(DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8),
                           DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
        m_headerBar->show();
        m_progressBar->show();
        m_controlBar->show();
        m_controlBar->setStyleSheet("");
        m_progressBar->setStyleSheet("");

        // 恢复控件到布局中
        QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
        if (mainLayout) {
            mainLayout->removeWidget(m_mediaViewer);
            m_mediaViewer->setParent(centralWidget());
            mainLayout->insertWidget(1, m_mediaViewer, 1);
            mainLayout->insertWidget(2, m_progressBar);
            mainLayout->insertWidget(3, m_controlBar);
        }

        m_isFullScreen = false;
        update();
    } else {
        setContentsMargins(0, 0, 0, 0);
        showFullScreen();
        m_headerBar->hide();

        // 从布局中移除控件，改为绝对定位浮动
        QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
        if (mainLayout) {
            mainLayout->removeWidget(m_mediaViewer);
            mainLayout->removeWidget(m_progressBar);
            mainLayout->removeWidget(m_controlBar);
        }

        int w = width();
        int h = height();
        int ctrlH = DPIAdapter::scaledSize(52);
        int progH = DPIAdapter::scaledSize(20);

        // 视频放最底层，充满整个窗口
        m_mediaViewer->setParent(centralWidget());
        m_mediaViewer->setGeometry(0, 0, w, h);
        m_mediaViewer->lower();
        m_mediaViewer->show();

        // 进度条浮在上面
        m_progressBar->setParent(centralWidget());
        m_progressBar->setGeometry(0, h - ctrlH - progH, w, progH);
        m_progressBar->setStyleSheet("QWidget { background-color: rgba(0,0,0,0.5); }");
        m_progressBar->raise();
        m_progressBar->show();

        // 控制栏浮在上面
        m_controlBar->setParent(centralWidget());
        m_controlBar->setGeometry(0, h - ctrlH, w, ctrlH);
        m_controlBar->setStyleSheet(
            "QWidget#controlBar { background-color: rgba(0,0,0,0.5); }"
            "QPushButton { background-color: transparent; }"
        );
        m_controlBar->setObjectName("controlBar");
        m_controlBar->raise();
        m_controlBar->show();

        m_isFullScreen = true;
        m_hideControlsTimer->start();
        update();
    }
}

void MainWindow::showControls()
{
    if (!m_isFullScreen) return;
    m_hideControlsTimer->stop();

    int ctrlH = DPIAdapter::scaledSize(52);
    int progH = DPIAdapter::scaledSize(20);
    int w = width();
    int h = height();

    // 动画：进度条从底部滑入
    QPoint progTarget(0, h - ctrlH - progH);
    if (m_progressBar->pos().y() >= h) {
        m_progressBar->move(0, h);
        m_progressBar->show();
        m_progressBar->raise();
    }
    m_progressBarAnimation->setTargetObject(m_progressBar);
    m_progressBarAnimation->setPropertyName("pos");
    m_progressBarAnimation->setStartValue(m_progressBar->pos());
    m_progressBarAnimation->setEndValue(progTarget);
    m_progressBarAnimation->start();

    // 动画：控制栏从底部滑入
    QPoint ctrlTarget(0, h - ctrlH);
    if (m_controlBar->pos().y() >= h) {
        m_controlBar->move(0, h);
        m_controlBar->show();
        m_controlBar->raise();
    }
    m_controlBarAnimation->setTargetObject(m_controlBar);
    m_controlBarAnimation->setPropertyName("pos");
    m_controlBarAnimation->setStartValue(m_controlBar->pos());
    m_controlBarAnimation->setEndValue(ctrlTarget);
    m_controlBarAnimation->start();

    m_hideControlsTimer->start();
}

void MainWindow::hideControls()
{
    if (!m_isFullScreen) return;

    int h = height();

    // 动画：进度条向下滑出
    QPoint progTarget(0, h);
    m_progressBarAnimation->setTargetObject(m_progressBar);
    m_progressBarAnimation->setPropertyName("pos");
    m_progressBarAnimation->setStartValue(m_progressBar->pos());
    m_progressBarAnimation->setEndValue(progTarget);
    m_progressBarAnimation->start();

    // 动画：控制栏向下滑出
    QPoint ctrlTarget(0, h);
    m_controlBarAnimation->setTargetObject(m_controlBar);
    m_controlBarAnimation->setPropertyName("pos");
    m_controlBarAnimation->setStartValue(m_controlBar->pos());
    m_controlBarAnimation->setEndValue(ctrlTarget);
    m_controlBarAnimation->start();
}

void MainWindow::hideControlsAfterTimeout()
{
    hideControls();
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
    // 全屏下更新浮动控件位置
    if (m_isFullScreen && m_progressBar && m_controlBar && m_mediaViewer) {
        int w = width();
        int h = height();
        int ctrlH = m_controlBar->height();
        int progH = m_progressBar->height();
        m_mediaViewer->setGeometry(0, 0, w, h);
        m_progressBar->setGeometry(0, h - ctrlH - progH, w, progH);
        m_controlBar->setGeometry(0, h - ctrlH, w, ctrlH);
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

    if (m_isFullScreen) {
        painter.setPen(Qt::NoPen);
        painter.drawRect(rect());
    } else {
        QColor borderColor = ThemeManager::instance()->primaryColor();
        QPen borderPen(borderColor);
        borderPen.setWidth(1);
        painter.setPen(borderPen);
        painter.drawRoundedRect(rect().adjusted(0, 0, -1, -1), DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
    }
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
        return true;
    }
    if (event->type() == QEvent::MouseMove && m_isFullScreen) {
        showControls();
        return false;
    }
    if (event->type() == QEvent::MouseButtonPress && !isMaximized() && !m_isFullScreen) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QPoint globalPos = mouseEvent->globalPosition().toPoint();
            QPoint localPos = mapFromGlobal(globalPos);
            int x = localPos.x();
            int y = localPos.y();
            int bw = DPIAdapter::scaledSize(kResizeBorder);
            Qt::Edges edges;
            if (x >= 0 && x < bw) edges |= Qt::LeftEdge;
            if (x <= width() && x > width() - bw) edges |= Qt::RightEdge;
            if (y >= 0 && y < bw) edges |= Qt::TopEdge;
            if (y <= height() && y > height() - bw) edges |= Qt::BottomEdge;
            if (edges) {
                QWindow* w = windowHandle();
                if (w) {
                    w->startSystemResize(edges);
                    return true;
                }
            }
        }
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
    if (m_isFullScreen) {
        showControls();
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton && !m_currentFile.isEmpty()) {
        showFileInfoMenu(event->globalPosition().toPoint());
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::showFileInfoMenu(const QPoint& pos)
{
    QFileInfo fileInfo(m_currentFile);

    QMenu menu(this);
    menu.setTitle("文件信息");

    QAction* nameAction = menu.addAction(tr("文件名: %1").arg(fileInfo.fileName()));
    nameAction->setEnabled(false);

    QAction* pathAction = menu.addAction(tr("路径: %1").arg(fileInfo.absolutePath()));
    pathAction->setEnabled(false);

    qint64 size = fileInfo.size();
    QString sizeStr;
    if (size < 1024) sizeStr = tr("%1 B").arg(size);
    else if (size < 1024 * 1024) sizeStr = tr("%1 KB").arg(qRound(size / 1024.0));
    else if (size < 1024 * 1024 * 1024) sizeStr = tr("%1 MB").arg(qRound(size / (1024.0 * 1024)));
    else sizeStr = tr("%1 GB").arg(qRound(size / (1024.0 * 1024 * 1024)));
    QAction* sizeAction = menu.addAction(tr("大小: %1").arg(sizeStr));
    sizeAction->setEnabled(false);

    QAction* createdAction = menu.addAction(tr("创建时间: %1").arg(
        fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss")));
    createdAction->setEnabled(false);

    QAction* modifiedAction = menu.addAction(tr("修改时间: %1").arg(
        fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss")));
    modifiedAction->setEnabled(false);

    QAction* typeAction = menu.addAction(tr("类型: %1").arg(fileInfo.suffix().toUpper()));
    typeAction->setEnabled(false);

    menu.addSeparator();
    QAction* openFolderAction = menu.addAction(tr("打开所在文件夹"));
    connect(openFolderAction, &QAction::triggered, this, [this]() {
        QFileInfo info(m_currentFile);
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
    });

    menu.exec(pos);
}

void MainWindow::checkFileAssociations()
{
    FileAssociation* fa = FileAssociation::instance();
    QStringList allFormats = fa->videoFormats() + fa->audioFormats() + fa->imageFormats();

    if (!fa->checkAssociationMatches()) {
        fa->syncFromSystem();
        showNotification("文件关联", "检测到文件关联变化，已同步配置");
    }
}

void MainWindow::showNotification(const QString& title, const QString& message)
{
    QWidget* notification = new QWidget(this);
    notification->setFixedWidth(DPIAdapter::scaledSize(320));
    notification->setFixedHeight(DPIAdapter::scaledSize(80));
    notification->setStyleSheet(
        "QWidget { background-color: #2D2D2D; border: 1px solid #444444; border-radius: 8px; }"
    );

    QVBoxLayout* layout = new QVBoxLayout(notification);
    layout->setContentsMargins(DPIAdapter::scaledSize(12), DPIAdapter::scaledSize(10), 
                               DPIAdapter::scaledSize(12), DPIAdapter::scaledSize(10));
    layout->setSpacing(DPIAdapter::scaledSize(4));

    QLabel* titleLabel = new QLabel(title, notification);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #00D4AA;");
    layout->addWidget(titleLabel);

    QLabel* msgLabel = new QLabel(message, notification);
    QFont msgFont = msgLabel->font();
    msgFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    msgLabel->setFont(msgFont);
    msgLabel->setStyleSheet("color: #E0E0E0;");
    msgLabel->setWordWrap(true);
    layout->addWidget(msgLabel);

    notification->setWindowFlags(Qt::ToolTip | Qt::WindowStaysOnTopHint);
    
    QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    int x = screenRect.width() - notification->width() - DPIAdapter::scaledSize(20);
    int y = screenRect.height() - notification->height() - DPIAdapter::scaledSize(20);
    notification->move(x, y);

    notification->show();

    QTimer::singleShot(3000, notification, &QWidget::deleteLater);
}
