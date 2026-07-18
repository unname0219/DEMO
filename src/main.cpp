#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QIcon>
#include "managers/ThemeManager.h"
#include "managers/DPIAdapter.h"
#include "managers/FileAssociation.h"
#include "ui/MainWindow.h"
#include "ui/FileAssocDialog.h"

// 配置文件统一存放在程序所在目录下的 user/ 文件夹（INI 格式），
// 软件产生的数据存放在 data/ 文件夹，不在其他位置生成文件。
static void setupPortableConfigPaths()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString userDir = appDir + "/user";
    QString dataDir = appDir + "/data";

    QDir().mkpath(userDir);
    QDir().mkpath(dataDir);

    // 让所有默认构造的 QSettings 使用 INI 格式，存放在 user/ 目录
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userDir);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, dataDir);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("FireflyPlayer");
    QCoreApplication::setOrganizationName("Firefly");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/logo.svg"));

    // 必须在创建任何使用 QSettings 的对象之前设置路径
    setupPortableConfigPaths();

    DPIAdapter::applyGlobalSettings();

    ThemeManager::instance()->init();

    // 首次启动检测：通过 user/ 目录下的 INI 配置判断
    QSettings settings;
    bool firstRun = settings.value("firstRun", true).toBool();

    if (firstRun) {
        FileAssocDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            // 用户点击"开始使用"，保存勾选的关联格式
            dialog.saveAssociations();
            settings.setValue("associationConfigured", true);
        } else {
            // 用户点击"跳过"，明确清空关联（不保留默认预设）
            FileAssociation::instance()->clearAssociations();
            settings.setValue("associationConfigured", true);
        }
        settings.setValue("firstRun", false);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
