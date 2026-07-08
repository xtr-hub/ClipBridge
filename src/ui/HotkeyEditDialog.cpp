#include "HotkeyEditDialog.h"
#include <QMessageBox>
#include <QKeySequence>

namespace ClipBridge {

HotkeyEditDialog::HotkeyEditDialog(const AppConfig &config, Mode mode, int editIndex, QWidget *parent)
    : QDialog(parent), m_config(config), m_mode(mode), m_editIndex(editIndex)
{
    setWindowTitle(mode == AddMode ? tr("添加热键") : tr("编辑热键"));
    setWindowIcon(QIcon(":/resources/icon.png"));
    setMinimumSize(380, 340);
    resize(400, 360);

    if (m_mode == EditMode && m_editIndex >= 0 && m_editIndex < m_config.hotkeys.size()) {
        m_binding = m_config.hotkeys[m_editIndex];
    } else {
        // Default values for new hotkey
        m_binding.action = "clipboard_image_path";
        m_binding.keySequence = QKeySequence();
        m_binding.behavior = m_config.defaultBehavior;
    }

    setupUI();
    loadHotkeyToUI();
}

void HotkeyEditDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);

    m_actionCombo = new QComboBox(this);
    m_actionCombo->setMinimumHeight(28);
    m_actionCombo->addItem(tr("复制图片路径 (clipboard_image_path)"), "clipboard_image_path");
    m_actionCombo->addItem(tr("去除换行符 (strip_newlines)"), "strip_newlines");
    formLayout->addRow(tr("动作:"), m_actionCombo);

    m_keySequenceEdit = new QKeySequenceEdit(this);
    m_keySequenceEdit->setMinimumHeight(28);
    QLabel *keyHint = new QLabel(tr("点击下面的输入框，然后按下快捷键"), this);
    keyHint->setStyleSheet("color: gray; font-size: 11px;");
    formLayout->addRow("", keyHint);
    formLayout->addRow(tr("快捷键:"), m_keySequenceEdit);

    mainLayout->addLayout(formLayout);

    // Behavior group
    QGroupBox *behaviorGroup = new QGroupBox(tr("行为设置"), this);
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);
    behaviorLayout->setSpacing(10);
    behaviorLayout->setContentsMargins(12, 18, 12, 15);

    m_autoPasteCheck = new QCheckBox(tr("自动粘贴"), this);
    m_autoPasteCheck->setToolTip(tr("执行动作后自动将结果粘贴到当前活动窗口"));
    m_autoSubmitCheck = new QCheckBox(tr("自动提交"), this);
    m_autoSubmitCheck->setToolTip(tr("自动粘贴后模拟回车键提交（慎用）"));

    behaviorLayout->addWidget(m_autoPasteCheck);
    behaviorLayout->addWidget(m_autoSubmitCheck);

    mainLayout->addWidget(behaviorGroup);
    mainLayout->addStretch();

    // Bottom buttons
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);
    QPushButton *okBtn = new QPushButton(tr("确定"), this);
    okBtn->setMinimumWidth(90);
    okBtn->setMinimumHeight(30);

    QPushButton *cancelBtn = new QPushButton(tr("取消"), this);
    cancelBtn->setMinimumWidth(90);
    cancelBtn->setMinimumHeight(30);

    bottomLayout->addStretch();
    bottomLayout->addWidget(okBtn);
    bottomLayout->addWidget(cancelBtn);

    mainLayout->addLayout(bottomLayout);

    connect(okBtn, &QPushButton::clicked, this, &HotkeyEditDialog::validateAndAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void HotkeyEditDialog::loadHotkeyToUI()
{
    int actionIndex = m_actionCombo->findData(m_binding.action);
    if (actionIndex >= 0) {
        m_actionCombo->setCurrentIndex(actionIndex);
    }

    m_keySequenceEdit->setKeySequence(m_binding.keySequence);
    m_autoPasteCheck->setChecked(m_binding.behavior.autoPaste);
    m_autoSubmitCheck->setChecked(m_binding.behavior.autoSubmit);
}

void HotkeyEditDialog::validateAndAccept()
{
    QKeySequence keySequence = m_keySequenceEdit->keySequence();
    if (keySequence.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请输入快捷键！"));
        m_keySequenceEdit->setFocus();
        return;
    }

    // Check for duplicate hotkeys
    QString newAction = m_actionCombo->currentData().toString();
    for (int i = 0; i < m_config.hotkeys.size(); ++i) {
        if (m_mode == EditMode && i == m_editIndex) {
            continue;  // Skip ourselves when editing
        }
        if (m_config.hotkeys[i].keySequence == keySequence) {
            QMessageBox::warning(this, tr("错误"), tr("该快捷键已被使用！"));
            m_keySequenceEdit->setFocus();
            return;
        }
    }

    m_binding.action = newAction;
    m_binding.keySequence = keySequence;
    m_binding.behavior.autoPaste = m_autoPasteCheck->isChecked();
    m_binding.behavior.autoSubmit = m_autoSubmitCheck->isChecked();

    accept();
}

} // namespace ClipBridge
