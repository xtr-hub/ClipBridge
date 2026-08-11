#include "HotkeyEditDialog.h"
#include <QMessageBox>
#include <QKeySequence>

namespace ClipBridge {

HotkeyEditDialog::HotkeyEditDialog(const AppConfig &config, Mode mode, int editIndex, QWidget *parent)
    : QDialog(parent), m_config(config), m_mode(mode), m_editIndex(editIndex)
{
    setWindowTitle(mode == AddMode ? tr("添加热键") : tr("编辑热键"));
    setMinimumSize(380, 340);
    resize(400, 360);

    if (m_mode == EditMode && m_editIndex >= 0 && m_editIndex < m_config.hotkeys.size())
        m_binding = m_config.hotkeys[m_editIndex];
    else {
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

    QFormLayout *form = new QFormLayout();
    form->setSpacing(12);

    m_actionCombo = new QComboBox(this);
    m_actionCombo->setMinimumHeight(28);

    // In Add mode, only show actions that don't already have a hotkey
    QSet<QString> assignedActions;
    if (m_mode == AddMode) {
        for (const auto &b : m_config.hotkeys)
            assignedActions.insert(b.action);
    } else if (m_mode == EditMode && m_editIndex >= 0 && m_editIndex < m_config.hotkeys.size()) {
        for (int i = 0; i < m_config.hotkeys.size(); ++i) {
            if (i != m_editIndex)
                assignedActions.insert(m_config.hotkeys[i].action);
        }
    }

    for (const QString &a : AppConfig::availableActions()) {
        if (!assignedActions.contains(a))
            m_actionCombo->addItem(AppConfig::actionDisplayName(a), a);
    }
    form->addRow(tr("动作:"), m_actionCombo);

    m_keySequenceEdit = new QKeySequenceEdit(this);
    m_keySequenceEdit->setMinimumHeight(28);
    QLabel *hint = new QLabel(tr("点击输入框，然后按下快捷键"), this);
    hint->setStyleSheet("color: gray; font-size: 11px;");
    form->addRow("", hint);
    form->addRow(tr("快捷键:"), m_keySequenceEdit);
    mainLayout->addLayout(form);

    QGroupBox *bhvGrp = new QGroupBox(tr("行为设置"), this);
    QVBoxLayout *bhvLayout = new QVBoxLayout(bhvGrp);
    bhvLayout->setSpacing(10);
    bhvLayout->setContentsMargins(12, 18, 12, 15);

    m_autoPasteCheck = new QCheckBox(tr("自动粘贴"), this);
    m_autoPasteCheck->setToolTip(tr("执行后自动粘贴到当前活动窗口"));
    m_autoSubmitCheck = new QCheckBox(tr("自动提交"), this);
    m_autoSubmitCheck->setToolTip(tr("粘贴后模拟回车键提交（慎用）"));
    bhvLayout->addWidget(m_autoPasteCheck);
    bhvLayout->addWidget(m_autoSubmitCheck);
    mainLayout->addWidget(bhvGrp);
    mainLayout->addStretch();

    QHBoxLayout *bottom = new QHBoxLayout();
    bottom->setSpacing(10);
    QPushButton *okBtn = new QPushButton(tr("确定"), this);
    okBtn->setMinimumWidth(90);
    okBtn->setMinimumHeight(30);
    QPushButton *cancelBtn = new QPushButton(tr("取消"), this);
    cancelBtn->setMinimumWidth(90);
    cancelBtn->setMinimumHeight(30);
    bottom->addStretch();
    bottom->addWidget(okBtn);
    bottom->addWidget(cancelBtn);
    mainLayout->addLayout(bottom);

    connect(okBtn, &QPushButton::clicked, this, &HotkeyEditDialog::validateAndAccept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void HotkeyEditDialog::loadHotkeyToUI()
{
    int idx = m_actionCombo->findData(m_binding.action);
    if (idx >= 0) m_actionCombo->setCurrentIndex(idx);
    m_keySequenceEdit->setKeySequence(m_binding.keySequence);
    m_autoPasteCheck->setChecked(m_binding.behavior.autoPaste);
    m_autoSubmitCheck->setChecked(m_binding.behavior.autoSubmit);
}

void HotkeyEditDialog::validateAndAccept()
{
    QKeySequence seq = m_keySequenceEdit->keySequence();
    auto warn = [this](const QString &text) {
        QMessageBox box(QMessageBox::Warning, tr("错误"), text,
                        QMessageBox::NoButton, this);
        box.setStyleSheet("QMessageBox QLabel { qproperty-alignment: 'AlignVCenter | AlignLeft'; }");
        box.exec();
    };

    if (seq.isEmpty()) {
        warn(tr("请输入快捷键！"));
        m_keySequenceEdit->setFocus();
        return;
    }
    for (int i = 0; i < m_config.hotkeys.size(); ++i) {
        if (m_mode == EditMode && i == m_editIndex) continue;
        if (m_config.hotkeys[i].keySequence == seq) {
            warn(tr("该快捷键已被使用！"));
            m_keySequenceEdit->setFocus();
            return;
        }
    }
    m_binding.action = m_actionCombo->currentData().toString();
    m_binding.keySequence = seq;
    m_binding.behavior.autoPaste = m_autoPasteCheck->isChecked();
    m_binding.behavior.autoSubmit = m_autoSubmitCheck->isChecked();
    accept();
}

} // namespace ClipBridge
