#include <QApplication>
#include <QSettings>
#include "managers/ThemeManager.h"
#include "managers/DPIAdapter.h"
#include "ui/MainWindow.h"
#include "ui/FileAssocDialog.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName("FireflyPlayer");
    QCoreApplication::setOrganizationName("Firefly");
    QCoreApplication::setApplicationVersion("1.0.0");

    QApplication app(argc, argv);

    DPIAdapter::applyGlobalSettings();

    ThemeManager::instance()->init();

    QSettings settings;
    bool firstRun = settings.value("firstRun", true).toBool();

    if (firstRun) {
        FileAssocDialog dialog;
        if (dialog.exec() == QDialog::Accepted) {
            dialog.saveAssociations();
        }
        settings.setValue("firstRun", false);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
