#pragma once

#include "AppConfig.h"
#include <QObject>

namespace ClipBridge {

class ActionManager;

class MonitorService : public QObject
{
    Q_OBJECT
public:
    explicit MonitorService(ActionManager *actionManager, QObject *parent = nullptr);
    ~MonitorService() override;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    // Ref-counted suppression — call when a UI panel opens/closes so the
    // monitor doesn't process its own clipboard writes.  Balanced enter/leave
    // calls are required; nesting is supported.
    void enterIgnoreScope();
    void leaveIgnoreScope();

    // Only takes what it actually needs (MonitorConfig + paste key), not the
    // entire AppConfig.  Returns true if the paste hook needed re-installing.
    void updateConfig(const AppConfig::MonitorConfig &monitorConfig,
                      const QString &pasteKey);

    // Called from low-level keyboard hook when paste key is detected
    static void notifyPasteDetected();

signals:
    void enabledChanged(bool enabled);
    void pasteDetected();

private slots:
    void onPasteDetected();

private:
    void tryAutoActions();
    void installPasteHook();
    void removePasteHook();

    ActionManager *m_actionManager;
    AppConfig::MonitorConfig m_config;
    QString m_pasteKey;
    bool m_enabled = false;
    bool m_processing = false;
    int  m_ignoreCount = 0;  // ref-counted: > 0 → monitor is suppressed
};

} // namespace ClipBridge
