#pragma once

/**
 * @file SettingsDialog.h
 * @brief 设置对话框
 * @author Your Name
 * @date 2026
 */

#include <QDialog>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
#include <QComboBox>
#include <QHBoxLayout>

#include "core/AppConfig.h"
#include "ui/HotkeyEditDialog.h"

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
    void browsePath();
    void onPathModeChanged(int index);
    void onActionFormatChanged(int index);
    void onUseSpecificFormatChanged(bool checked);
    void onFormatTextChanged(const QString &text);

private:
    void setupUI();
    void loadConfigToUI();
    void updatePathInputState();
    void updateFormatEditState();
    void saveCurrentActionFormat();

    AppConfig m_config;

    QListWidget *m_hotkeyList;
    QComboBox *m_actionFormatCombo;
    QCheckBox *m_useSpecificFormatCheck;
    QLineEdit *m_formatEdit;
    QComboBox *m_pathModeCombo;
    QLineEdit *m_customPathEdit;
    QPushButton *m_browseBtn;
    QCheckBox *m_defaultAutoPasteCheck;
    QCheckBox *m_defaultAutoSubmitCheck;
    QComboBox *m_languageCombo;
    QHash<QString, QString> m_actionFormatBackups;
};

} // namespace ClipBridge
