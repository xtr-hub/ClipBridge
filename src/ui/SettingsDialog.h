#pragma once

#include <QDialog>
#include <QListWidget>
#include <QStackedWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QSlider>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QLabel>
#include <QHash>

#include "core/AppConfig.h"

namespace ClipBridge {

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const AppConfig &config, QWidget *parent = nullptr);
    AppConfig config() const { return m_config; }

private slots:
    void saveConfig();
    void onNavChanged(int row);

    void addHotkey();
    void editHotkey();
    void removeHotkey();

    void browsePath();
    void onPathModeChanged(int);
    void onActionFormatChanged(int);
    void onUseSpecificFormatChanged(bool);
    void onFormatTextChanged(const QString &);

private:
    void setupUI();
    void setupNav();
    void setupShortcutsPage();
    void setupOutputPage();
    void setupMonitorPage();
    void setupGeneralPage();
    void applyStyle();

    void loadConfigToUI();
    void refreshHotkeyList();
    void saveCurrentActionFormat();
    void refreshFormatUIForAction(const QString &action);
    void updatePathInputState();
    bool isFormatConfigurableAction(const QString &action) const;
    bool isSavePathConfigurableAction(const QString &action) const;

    AppConfig m_config;
    QString m_currentAction;
    bool m_updatingFormatUI = false;
    QHash<QString, QString> m_actionFormatBackups;

    QListWidget *m_nav;
    QStackedWidget *m_pages;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;

    // Shortcuts
    QListWidget *m_hotkeyList;
    QKeySequenceEdit *m_inputPanelHotkeyEdit;

    // Output
    QComboBox *m_actionFormatCombo;
    QCheckBox *m_useSpecificFormatCheck;
    QLineEdit *m_formatEdit;
    QComboBox *m_pathModeCombo;
    QLineEdit *m_customPathEdit;
    QPushButton *m_browseBtn;
    QComboBox *m_pasteKeyCombo;
    QSlider *m_pasteDelaySlider;
    QLabel *m_pasteDelayValue;

    // Monitor
    QCheckBox *m_monitorEnabledCheck;
    QCheckBox *m_autoImageCheck;
    QCheckBox *m_autoFileCheck;
    QCheckBox *m_autoStripNewlinesCheck;
    QCheckBox *m_autoLongTextCheck;
    QSlider *m_longTextThresholdSlider;
    QLabel *m_longTextThresholdValue;
    QCheckBox *m_monitorNotifyCheck;

    // General
    QCheckBox *m_defaultAutoPasteCheck;
    QCheckBox *m_defaultAutoSubmitCheck;
    QComboBox *m_languageCombo;
    QCheckBox *m_autoStartCheck;
    QLabel *m_versionLabel;
    QSlider *m_opacitySlider;
    QLabel *m_opacityValue;
};

} // namespace ClipBridge
