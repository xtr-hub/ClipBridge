#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>

#include "core/appconfig.h"

namespace ClipBridge {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const AppConfig &config, QWidget *parent = nullptr);

    AppConfig config() const { return m_config; }

private slots:
    void saveConfig();
    void addHotkey();
    void editHotkey();
    void removeHotkey();

private:
    void setupUI();
    void loadConfigToUI();

    AppConfig m_config;

    QListWidget *m_hotkeyList;
    QLineEdit *m_formatEdit;
    QLineEdit *m_customPathEdit;
    QCheckBox *m_autoPasteCheck;
    QCheckBox *m_autoSubmitCheck;
};

} // namespace ClipBridge
