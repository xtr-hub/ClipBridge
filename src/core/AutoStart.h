#pragma once

#include <QString>

namespace ClipBridge {

class AutoStart
{
public:
    static bool isEnabled();
    static void setEnabled(bool enabled);
    static QString appPath();
    static QString registryKey();
    static QString plistPath();
    static QString desktopFilePath();
};

} // namespace ClipBridge
