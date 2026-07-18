#include "ui/SettingsPanel.h"
#include "managers/ThemeManager.h"
#include "managers/FileAssociation.h"
#include "managers/PluginManager.h"
#include "managers/DPIAdapter.h"
#include "managers/IconManager.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QComboBox>
#include <QFrame>
#include <QPainter>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QDialog(parent)
    , m_tabWidget(nullptr)
{
    setWindowTitle("Firefly Player - 设置");
    setModal(true);
    setMinimumSize(DPIAdapter::scaledSize(560), DPIAdapter::scaledSize(480));
    resize(DPIAdapter::scaledSize(560), DPIAdapter::scaledSize(520));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setupUI();
}

void SettingsPanel::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor bg = ThemeManager::instance()->backgroundColor();
    painter.setBrush(QBrush(bg));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
}

SettingsPanel::~SettingsPanel()
{
}

void SettingsPanel::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8), 
                                   DPIAdapter::scaledSize(8), DPIAdapter::scaledSize(8));
    mainLayout->setSpacing(0);

    QWidget* titleBar = new QWidget(this);
    titleBar->setFixedHeight(DPIAdapter::scaledSize(36));
    QHBoxLayout* titleLayout = new QHBoxLayout(titleBar);
    titleLayout->setContentsMargins(DPIAdapter::scaledSize(12), 0, DPIAdapter::scaledSize(8), 0);
    titleLayout->setSpacing(DPIAdapter::scaledSize(8));

    QLabel* titleLabel = new QLabel("设置", titleBar);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(11));
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor().name()));
    titleLayout->addWidget(titleLabel);

    titleLayout->addStretch();

    QPushButton* closeBtn = new QPushButton(titleBar);
    closeBtn->setFixedSize(DPIAdapter::scaledSize(28), DPIAdapter::scaledSize(28));
    closeBtn->setIconSize(QSize(DPIAdapter::scaledSize(14), DPIAdapter::scaledSize(14)));
    closeBtn->setIcon(IconManager::instance()->icon("close"));
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setToolTip("关闭");
    closeBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; }"
        "QPushButton:hover { background-color: rgba(255,255,255,10); border-radius: 4px; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    titleLayout->addWidget(closeBtn);

    mainLayout->addWidget(titleBar);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabPosition(QTabWidget::West);
    m_tabWidget->setDocumentMode(true);
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background: transparent; }"
        "QTabWidget::tab-bar { alignment: left; }"
        "QTabBar::tab { width: 100px; height: 40px; }"
    );
    mainLayout->addWidget(m_tabWidget);

    QWidget* appearancePage = new QWidget();
    setupAppearancePage(appearancePage);
    m_tabWidget->addTab(appearancePage, "外观");

    QWidget* fileAssocPage = new QWidget();
    setupFileAssocPage(fileAssocPage);
    m_tabWidget->addTab(fileAssocPage, "文件关联");

    QWidget* pluginsPage = new QWidget();
    setupPluginsPage(pluginsPage);
    m_tabWidget->addTab(pluginsPage, "格式支持包");

    QWidget* playbackPage = new QWidget();
    setupPlaybackPage(playbackPage);
    m_tabWidget->addTab(playbackPage, "播放设置");

    QWidget* shortcutsPage = new QWidget();
    setupShortcutsPage(shortcutsPage);
    m_tabWidget->addTab(shortcutsPage, "快捷键");

    QWidget* aboutPage = new QWidget();
    setupAboutPage(aboutPage);
    m_tabWidget->addTab(aboutPage, "关于");
}

void SettingsPanel::setupAppearancePage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(16));

    QGroupBox* themeGroup = new QGroupBox("主题模式", page);
    QVBoxLayout* themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(DPIAdapter::scaledSize(8));

    ThemeMode currentMode = ThemeManager::instance()->themeMode();

    QRadioButton* systemBtn = new QRadioButton("跟随系统", themeGroup);
    systemBtn->setChecked(currentMode == ThemeMode::System);
    connect(systemBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::System);
    });
    themeLayout->addWidget(systemBtn);

    QRadioButton* lightBtn = new QRadioButton("浅色模式", themeGroup);
    lightBtn->setChecked(currentMode == ThemeMode::Light);
    connect(lightBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Light);
    });
    themeLayout->addWidget(lightBtn);

    QRadioButton* darkBtn = new QRadioButton("深色模式", themeGroup);
    darkBtn->setChecked(currentMode == ThemeMode::Dark);
    connect(darkBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Dark);
    });
    themeLayout->addWidget(darkBtn);

    layout->addWidget(themeGroup);

    QGroupBox* imgGroup = new QGroupBox("图片缩放", page);
    QVBoxLayout* imgLayout = new QVBoxLayout(imgGroup);
    imgLayout->setSpacing(DPIAdapter::scaledSize(8));

    QSettings settings;
    bool smooth = settings.value("image/smoothScaling", true).toBool();

    QRadioButton* smoothBtn = new QRadioButton("平滑模式 (高质量插值)", imgGroup);
    smoothBtn->setChecked(smooth);
    connect(smoothBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", true);
            emit imageScalingChanged(true);
        }
    });
    imgLayout->addWidget(smoothBtn);

    QRadioButton* pixelBtn = new QRadioButton("像素模式 (显示像素点)", imgGroup);
    pixelBtn->setChecked(!smooth);
    connect(pixelBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", false);
            emit imageScalingChanged(false);
        }
    });
    imgLayout->addWidget(pixelBtn);

    layout->addWidget(imgGroup);
    layout->addStretch();
}

void SettingsPanel::setupFileAssocPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* desc = new QLabel("选择需要用 Firefly Player 打开的文件格式", page);
    desc->setWordWrap(true);
    QFont descFont = desc->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    desc->setFont(descFont);
    layout->addWidget(desc);

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(12));

    FileAssociation* fa = FileAssociation::instance();

    QGroupBox* videoGroup = new QGroupBox("视频格式", scrollContent);
    QVBoxLayout* videoLayout = new QVBoxLayout(videoGroup);
    videoLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList videoFormats = {"mp4", "mkv", "avi", "mov", "webm", "flv", "wmv", "ts", "3gp"};
    foreach (const QString& fmt, videoFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", videoGroup);
        cb->setChecked(fa->videoFormats().contains(fmt));
        videoLayout->addWidget(cb);
    }
    scrollLayout->addWidget(videoGroup);

    QGroupBox* audioGroup = new QGroupBox("音频格式", scrollContent);
    QVBoxLayout* audioLayout = new QVBoxLayout(audioGroup);
    audioLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList audioFormats = {"mp3", "wav", "flac", "aac", "ogg", "m4a", "wma", "opus"};
    foreach (const QString& fmt, audioFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", audioGroup);
        cb->setChecked(fa->audioFormats().contains(fmt));
        audioLayout->addWidget(cb);
    }
    scrollLayout->addWidget(audioGroup);

    QGroupBox* imageGroup = new QGroupBox("图片格式", scrollContent);
    QVBoxLayout* imageLayout = new QVBoxLayout(imageGroup);
    imageLayout->setSpacing(DPIAdapter::scaledSize(2));
    QStringList imageFormats = {"jpg", "jpeg", "png", "gif", "bmp", "webp", "tiff", "svg"};
    foreach (const QString& fmt, imageFormats) {
        QCheckBox* cb = new QCheckBox(fmt.toUpper() + " (*." + fmt + ")", imageGroup);
        cb->setChecked(fa->imageFormats().contains(fmt));
        imageLayout->addWidget(cb);
    }
    scrollLayout->addWidget(imageGroup);

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupPluginsPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* desc = new QLabel("管理可选的格式支持包，按需安装以支持更多格式", page);
    desc->setWordWrap(true);
    QFont descFont = desc->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    desc->setFont(descFont);
    layout->addWidget(desc);

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(8));

    PluginManager* pm = PluginManager::instance();
    QList<PluginInfo> plugins = pm->availablePlugins();

    foreach (const PluginInfo& plugin, plugins) {
        QFrame* pluginFrame = new QFrame(scrollContent);
        pluginFrame->setFrameShape(QFrame::StyledPanel);
        pluginFrame->setStyleSheet("QFrame { border: 1px solid #444444; border-radius: 6px; padding: 12px; }");
        QVBoxLayout* pluginLayout = new QVBoxLayout(pluginFrame);
        pluginLayout->setSpacing(DPIAdapter::scaledSize(6));

        QHBoxLayout* headerLayout = new QHBoxLayout();
        QLabel* nameLabel = new QLabel(plugin.name, pluginFrame);
        QFont nameFont = nameLabel->font();
        nameFont.setBold(true);
        nameFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
        nameLabel->setFont(nameFont);
        headerLayout->addWidget(nameLabel);
        headerLayout->addStretch();
        QLabel* sizeLabel = new QLabel(QString("%1 MB").arg(plugin.sizeMB), pluginFrame);
        QFont sizeFont = sizeLabel->font();
        sizeFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
        sizeLabel->setFont(sizeFont);
        headerLayout->addWidget(sizeLabel);
        pluginLayout->addLayout(headerLayout);

        QLabel* descLabel = new QLabel(plugin.description, pluginFrame);
        descLabel->setWordWrap(true);
        QFont pDescFont = descLabel->font();
        pDescFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
        descLabel->setFont(pDescFont);
        pluginLayout->addWidget(descLabel);

        QHBoxLayout* btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        QPushButton* actionBtn = new QPushButton(plugin.isInstalled ? "已安装" : "安装", pluginFrame);
        actionBtn->setEnabled(!plugin.isInstalled);
        actionBtn->setStyleSheet(plugin.isInstalled
            ? "QPushButton { background-color: #00D4AA; color: white; padding: 6px 16px; border-radius: 4px; }"
            : "QPushButton { background-color: transparent; border: 1px solid #00D4AA; color: #00D4AA; padding: 6px 16px; border-radius: 4px; }"
        );
        btnLayout->addWidget(actionBtn);
        pluginLayout->addLayout(btnLayout);

        scrollLayout->addWidget(pluginFrame);
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupPlaybackPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(16));

    QGroupBox* speedGroup = new QGroupBox("默认播放速度", page);
    QVBoxLayout* speedLayout = new QVBoxLayout(speedGroup);
    speedLayout->setSpacing(DPIAdapter::scaledSize(8));

    QComboBox* speedCombo = new QComboBox(speedGroup);
    QStringList speeds = {"0.1x", "0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "1.75x", "2.0x", "5.0x"};
    speedCombo->addItems(speeds);
    speedCombo->setCurrentText("1.0x");
    speedLayout->addWidget(speedCombo);
    layout->addWidget(speedGroup);

    QGroupBox* volGroup = new QGroupBox("音量增强", page);
    QVBoxLayout* volLayout = new QVBoxLayout(volGroup);
    volLayout->setSpacing(DPIAdapter::scaledSize(8));

    QCheckBox* volBoostCb = new QCheckBox("启用音量增强（最高500%）", volGroup);
    volBoostCb->setChecked(false);
    volLayout->addWidget(volBoostCb);

    QLabel* volDesc = new QLabel("注意：过高音量可能损伤听力或设备", volGroup);
    QFont volDescFont = volDesc->font();
    volDescFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    volDesc->setFont(volDescFont);
    volLayout->addWidget(volDesc);

    layout->addWidget(volGroup);
    layout->addStretch();
}

void SettingsPanel::setupShortcutsPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(8));

    struct Shortcut { QString key; QString desc; };
    QList<Shortcut> shortcuts = {
        {"Space", "播放/暂停"},
        {"← / →", "快退/快进 5秒"},
        {"↑ / ↓", "音量增加/减少 10%"},
        {"M", "静音切换"},
        {"F / F11", "全屏切换"},
        {"Esc", "退出全屏"},
        {"Ctrl + + / -", "图片放大/缩小"},
        {"Ctrl + 0", "图片恢复原始大小"},
        {"← / → (图片)", "上一张/下一张"}
    };

    QScrollArea* scrollArea = new QScrollArea(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    QWidget* scrollContent = new QWidget();
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(DPIAdapter::scaledSize(4));

    foreach (const Shortcut& sc, shortcuts) {
        QFrame* row = new QFrame(scrollContent);
        QHBoxLayout* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 6, 0, 6);

        QLabel* keyLabel = new QLabel(sc.key, row);
        keyLabel->setStyleSheet(
            "background: #3A3A3A; padding: 4px 14px; border-radius: 4px; font-family: monospace;"
        );
        keyLabel->setFixedWidth(DPIAdapter::scaledSize(140));
        keyLabel->setAlignment(Qt::AlignCenter);
        rowLayout->addWidget(keyLabel);

        QLabel* descLabel = new QLabel(sc.desc, row);
        rowLayout->addWidget(descLabel, 1);

        scrollLayout->addWidget(row);
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);
}

void SettingsPanel::setupAboutPage(QWidget* page)
{
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(20), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* logoLabel = new QLabel(page);
    logoLabel->setFixedSize(DPIAdapter::scaledSize(64), DPIAdapter::scaledSize(64));
    logoLabel->setPixmap(IconManager::instance()->icon("logo", false).pixmap(logoLabel->size()));
    logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(logoLabel);

    QLabel* nameLabel = new QLabel("Firefly Player", page);
    QFont nameFont = nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSizeF(DPIAdapter::scaledFontSize(14));
    nameLabel->setFont(nameFont);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor().name()));
    layout->addWidget(nameLabel);

    QLabel* versionLabel = new QLabel("版本 1.4.0", page);
    QFont versionFont = versionLabel->font();
    versionFont.setPointSizeF(DPIAdapter::scaledFontSize(10));
    versionLabel->setFont(versionFont);
    versionLabel->setAlignment(Qt::AlignCenter);
    versionLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor().name()));
    layout->addWidget(versionLabel);

    layout->addStretch();

    QLabel* descLabel = new QLabel("一个轻量级的媒体播放器，支持视频、音频和图片播放。", page);
    QFont descFont = descLabel->font();
    descFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    descLabel->setFont(descFont);
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor().name()));
    layout->addWidget(descLabel);

    QLabel* authorLabel = new QLabel("© 2024 Firefly", page);
    QFont authorFont = authorLabel->font();
    authorFont.setPointSizeF(DPIAdapter::scaledFontSize(9));
    authorLabel->setFont(authorFont);
    authorLabel->setAlignment(Qt::AlignCenter);
    authorLabel->setStyleSheet(QString("color: %1;").arg(ThemeManager::instance()->textColor().name()));
    layout->addWidget(authorLabel);
}
