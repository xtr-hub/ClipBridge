#pragma once

/**
 * @file HotkeyEditDialog.h
 * @brief 热键编辑对话框
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
#include <QComboBox>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QGroupBox>

#include "core/AppConfig.h"

namespace ClipBridge {

class HotkeyEditDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode { AddMode, EditMode };

    explicit HotkeyEditDialog(const AppConfig &config, Mode mode, int editIndex = -1, QWidget *parent = nullptr);

    AppConfig::HotKeyBinding hotkeyBinding() const { return m_binding; }

private slots:
    void validateAndAccept();

private:
    void setupUI();
    void loadHotkeyToUI();

    AppConfig m_config;
    Mode m_mode;
    int m_editIndex;
    AppConfig::HotKeyBinding m_binding;

    QComboBox *m_actionCombo;
    QKeySequenceEdit *m_keySequenceEdit;
    QCheckBox *m_autoPasteCheck;
    QCheckBox *m_autoSubmitCheck;
};

} // namespace ClipBridge
