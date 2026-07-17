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
    , m_isFullScreen(false)
{
    m_playerController = new PlayerController(this);

    setAcceptDrops(true);
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

    QWidget* controlBar = new QWidget(this);
    controlBar->setFixedHeight(DPIAdapter::scaledSize(56));
    QHBoxLayout* controlLayout = new QHBoxLayout(controlBar);
    controlLayout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(12), 0);
    controlLayout->setSpacing(DPIAdapter::scaledSize(8));

    QPushButton* settingsBtn = new QPushButton("⚙", controlBar);
    settingsBtn->setFixedSize(DPIAdapter::scaledSize(36), DPIAdapter::scaledSize(36));
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::toggleSettings);
    controlLayout->addWidget(settingsBtn);

    m_playbackControls = new PlaybackControls(controlBar);
    controlLayout->addWidget(m_playbackControls);

    controlLayout->addStretch();

    m_speedSelector = new SpeedSelector(controlBar);
    controlLayout->addWidget(m_speedSelector);

    m_volumeSlider = new VolumeSlider(controlBar);
    controlLayout->addWidget(m_volumeSlider);

    mainLayout->addWidget(controlBar);

    m_settingsPanel = new SettingsPanel(this);
    m_settingsPanel->setVisible(false);
}

void MainWindow::setupConnections()
{
    connect(m_headerBar, &HeaderBar::openFileClicked, this, &MainWindow::openFileDialog);
    connect(m_headerBar, &HeaderBar::minimizeClicked, this, &QMainWindow::showMinimized);
    connect(m_headerBar, &HeaderBar::maximizeClicked, this, [this]() {
        if (isMaximized()) showNormal(); else showMaximized();
    });
    connect(m_headerBar, &HeaderBar::closeClicked, this, &QMainWindow::close);

    connect(m_playbackControls, &PlaybackControls::playToggled,
            m_playerController, &PlayerController::togglePlayPause);
    connect(m_playbackControls, &PlaybackControls::previousClicked,
            m_playerController, [this]() { m_playerController->seekBackward(10); });
    connect(m_playbackControls, &PlaybackControls::nextClicked,
            m_playerController, [this]() { m_playerController->seekForward(10); });

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
}

void MainWindow::toggleSettings()
{
    if (m_settingsPanel->isVisible()) {
        m_settingsPanel->hide();
    } else {
        m_settingsPanel->setParent(this);
        m_settingsPanel->setGeometry(
            DPIAdapter::scaledSize(20),
            m_headerBar->height() + DPIAdapter::scaledSize(10),
            DPIAdapter::scaledSize(400),
            height() - m_headerBar->height() - DPIAdapter::scaledSize(80)
        );
        m_settingsPanel->show();
    }
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
        m_settingsPanel->setFixedHeight(
            height() - m_headerBar->height() - DPIAdapter::scaledSize(80)
        );
    }
}
