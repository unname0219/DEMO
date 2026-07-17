#include "ui/SettingsPanel.h"
#include "managers/ThemeManager.h"
#include "managers/FileAssociation.h"
#include "managers/PluginManager.h"
#include "managers/DPIAdapter.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QPushButton>
#include <QSettings>
#include <QComboBox>
#include <QFrame>
#include <QSize>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QWidget(parent)
    , m_categoryList(nullptr)
    , m_contentStack(nullptr)
{
    setupUI();
}

SettingsPanel::~SettingsPanel()
{
}

void SettingsPanel::setupUI()
{
    // 设置面板使用固定合理的字号，不依赖过大 DPI 缩放
    setMinimumWidth(DPIAdapter::scaledSize(360));

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_categoryList = new QListWidget(this);
    m_categoryList->setFixedWidth(DPIAdapter::scaledSize(120));
    m_categoryList->setIconSize(QSize(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16)));
    m_categoryList->addItem("外观");
    m_categoryList->addItem("文件关联");
    m_categoryList->addItem("格式支持包");
    m_categoryList->addItem("播放设置");
    m_categoryList->addItem("快捷键");
    m_categoryList->setCurrentRow(0);
    connect(m_categoryList, &QListWidget::currentRowChanged,
            this, &SettingsPanel::onCategoryChanged);
    mainLayout->addWidget(m_categoryList);

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFixedWidth(1);
    separator->setProperty("role", "separator");
    mainLayout->addWidget(separator);

    m_contentStack = new QStackedWidget(this);
    mainLayout->addWidget(m_contentStack, 1);

    setupAppearancePage();
    setupFileAssocPage();
    setupPluginsPage();
    setupPlaybackPage();
    setupShortcutsPage();
}

void SettingsPanel::onCategoryChanged(int row)
{
    m_contentStack->setCurrentIndex(row);
}

void SettingsPanel::setupAppearancePage()
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* title = new QLabel("外观设置", page);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    QGroupBox* themeGroup = new QGroupBox("主题模式", page);
    QVBoxLayout* themeLayout = new QVBoxLayout(themeGroup);
    themeLayout->setSpacing(DPIAdapter::scaledSize(6));

    QRadioButton* systemBtn = new QRadioButton("跟随系统", themeGroup);
    QRadioButton* lightBtn = new QRadioButton("浅色模式", themeGroup);
    QRadioButton* darkBtn = new QRadioButton("深色模式", themeGroup);

    ThemeMode currentMode = ThemeManager::instance()->themeMode();
    switch (currentMode) {
    case ThemeMode::System: systemBtn->setChecked(true); break;
    case ThemeMode::Light: lightBtn->setChecked(true); break;
    case ThemeMode::Dark: darkBtn->setChecked(true); break;
    }

    connect(systemBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::System);
    });
    connect(lightBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Light);
    });
    connect(darkBtn, &QRadioButton::toggled, this, [](bool checked) {
        if (checked) ThemeManager::instance()->setThemeMode(ThemeMode::Dark);
    });

    themeLayout->addWidget(systemBtn);
    themeLayout->addWidget(lightBtn);
    themeLayout->addWidget(darkBtn);
    layout->addWidget(themeGroup);

    QGroupBox* imgGroup = new QGroupBox("图片缩放", page);
    QVBoxLayout* imgLayout = new QVBoxLayout(imgGroup);
    imgLayout->setSpacing(DPIAdapter::scaledSize(6));

    QRadioButton* smoothBtn = new QRadioButton("平滑模式 (高质量插值)", imgGroup);
    QRadioButton* pixelBtn = new QRadioButton("像素模式 (显示像素点)", imgGroup);

    QSettings settings;
    bool smooth = settings.value("image/smoothScaling", true).toBool();
    smoothBtn->setChecked(smooth);
    pixelBtn->setChecked(!smooth);

    // 改动立即生效：保存配置并发出信号，由 ImageViewer 实时响应
    connect(smoothBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", true);
            emit imageScalingChanged(true);
        }
    });
    connect(pixelBtn, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            QSettings s;
            s.setValue("image/smoothScaling", false);
            emit imageScalingChanged(false);
        }
    });

    imgLayout->addWidget(smoothBtn);
    imgLayout->addWidget(pixelBtn);
    layout->addWidget(imgGroup);

    layout->addStretch();
    scroll->setWidget(page);
    m_contentStack->addWidget(scroll);
}

void SettingsPanel::setupFileAssocPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(8));

    QLabel* title = new QLabel("文件关联", page);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

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
    scrollLayout->setSpacing(DPIAdapter::scaledSize(10));

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

    m_contentStack->addWidget(page);
}

void SettingsPanel::setupPluginsPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(8));

    QLabel* title = new QLabel("格式支持包", page);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

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
        pluginFrame->setStyleSheet("QFrame { border: 1px solid #444444; border-radius: 6px; padding: 8px; }");
        QVBoxLayout* pluginLayout = new QVBoxLayout(pluginFrame);
        pluginLayout->setSpacing(DPIAdapter::scaledSize(4));

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

    m_contentStack->addWidget(page);
}

void SettingsPanel::setupPlaybackPage()
{
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(12));

    QLabel* title = new QLabel("播放设置", page);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    QGroupBox* speedGroup = new QGroupBox("默认播放速度", page);
    QVBoxLayout* speedLayout = new QVBoxLayout(speedGroup);
    speedLayout->setSpacing(DPIAdapter::scaledSize(6));

    QComboBox* speedCombo = new QComboBox(speedGroup);
    QStringList speeds = {"0.1x", "0.5x", "0.75x", "1.0x", "1.25x", "1.5x", "1.75x", "2.0x", "5.0x"};
    speedCombo->addItems(speeds);
    speedCombo->setCurrentText("1.0x");
    speedLayout->addWidget(speedCombo);
    layout->addWidget(speedGroup);

    QGroupBox* volGroup = new QGroupBox("音量增强", page);
    QVBoxLayout* volLayout = new QVBoxLayout(volGroup);
    volLayout->setSpacing(DPIAdapter::scaledSize(6));
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
    scroll->setWidget(page);
    m_contentStack->addWidget(scroll);
}

void SettingsPanel::setupShortcutsPage()
{
    QWidget* page = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setContentsMargins(DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16),
                                DPIAdapter::scaledSize(16), DPIAdapter::scaledSize(16));
    layout->setSpacing(DPIAdapter::scaledSize(8));

    QLabel* title = new QLabel("快捷键", page);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(DPIAdapter::scaledFontSize(12));
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

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
        rowLayout->setContentsMargins(0, 4, 0, 4);

        QLabel* keyLabel = new QLabel(sc.key, row);
        keyLabel->setStyleSheet(
            "background: #3A3A3A; padding: 4px 12px; border-radius: 4px; font-family: monospace;"
        );
        keyLabel->setFixedWidth(DPIAdapter::scaledSize(120));
        keyLabel->setAlignment(Qt::AlignCenter);
        rowLayout->addWidget(keyLabel);

        QLabel* descLabel = new QLabel(sc.desc, row);
        rowLayout->addWidget(descLabel, 1);

        scrollLayout->addWidget(row);
    }

    scrollLayout->addStretch();
    scrollArea->setWidget(scrollContent);
    layout->addWidget(scrollArea, 1);

    m_contentStack->addWidget(page);
}
