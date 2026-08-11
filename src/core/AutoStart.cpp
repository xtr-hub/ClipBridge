#include "AutoStart.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

namespace ClipBridge {

QString AutoStart::appPath()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
}

#ifdef Q_OS_WIN

QString AutoStart::registryKey()
{
    return "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
}

bool AutoStart::isEnabled()
{
    QSettings settings(registryKey(), QSettings::NativeFormat);
    return settings.contains("ClipBridge");
}

void AutoStart::setEnabled(bool enabled)
{
    QSettings settings(registryKey(), QSettings::NativeFormat);
    if (enabled)
        settings.setValue("ClipBridge", appPath());
    else
        settings.remove("ClipBridge");
}

#elif defined(Q_OS_MACOS)

QString AutoStart::plistPath()
{
    return QDir::homePath() + "/Library/LaunchAgents/com.clipbridge.plist";
}

bool AutoStart::isEnabled()
{
    return QFile::exists(plistPath());
}

void AutoStart::setEnabled(bool enabled)
{
    if (enabled) {
        QString plist =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
            "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n"
            "    <key>Label</key>\n"
            "    <string>com.clipbridge</string>\n"
            "    <key>ProgramArguments</key>\n"
            "    <array>\n"
            "        <string>" + appPath() + "</string>\n"
            "    </array>\n"
            "    <key>RunAtLoad</key>\n"
            "    <true/>\n"
            "</dict>\n"
            "</plist>\n";
        QFile file(plistPath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << plist;
        }
    } else {
        QFile::remove(plistPath());
    }
}

#else

QString AutoStart::desktopFilePath()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return configDir + "/autostart/clipbridge.desktop";
}

bool AutoStart::isEnabled()
{
    return QFile::exists(desktopFilePath());
}

void AutoStart::setEnabled(bool enabled)
{
    if (enabled) {
        QString entry =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=ClipBridge\n"
            "Exec=" + appPath() + "\n"
            "Terminal=false\n"
            "X-GNOME-Autostart-enabled=true\n";
        QFile file(desktopFilePath());
        QDir().mkpath(QFileInfo(desktopFilePath()).absolutePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << entry;
        }
    } else {
        QFile::remove(desktopFilePath());
    }
}

#endif

} // namespace ClipBridge
