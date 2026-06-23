#include "SettingsDialog.h"
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>

namespace ClipBridge {

SettingsDialog::SettingsDialog(const AppConfig &config, QWidget *parent)
    : QDialog(parent), m_config(config)
{
    setWindowTitle("ClipBridge 设置");
    setMinimumSize(550, 450);

    setupUI();
    loadConfigToUI();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *hotkeyGroup = new QGroupBox("快捷键", this);
    QVBoxLayout *hotkeyLayout = new QVBoxLayout(hotkeyGroup);

    m_hotkeyList = new QListWidget(this);
    hotkeyLayout->addWidget(m_hotkeyList);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton("添加", this);
    QPushButton *editBtn = new QPushButton("编辑", this);
    QPushButton *removeBtn = new QPushButton("删除", this);

    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(removeBtn);
    hotkeyLayout->addLayout(buttonLayout);

    mainLayout->addWidget(hotkeyGroup);

    QGroupBox *outputGroup = new QGroupBox("输出设置", this);
    QFormLayout *outputLayout = new QFormLayout(outputGroup);

    m_formatEdit = new QLineEdit(this);
    m_formatEdit->setPlaceholderText("例如: 请查看这张图片 {path}");
    outputLayout->addRow("格式:", m_formatEdit);

    m_customPathEdit = new QLineEdit(this);
    m_customPathEdit->setPlaceholderText("自定义图片保存路径");
    outputLayout->addRow("保存路径:", m_customPathEdit);

    mainLayout->addWidget(outputGroup);

    QGroupBox *defaultBehaviorGroup = new QGroupBox("默认行为配置（用于新添加的热键）", this);
    QVBoxLayout *defaultBehaviorLayout = new QVBoxLayout(defaultBehaviorGroup);

    m_defaultAutoPasteCheck = new QCheckBox("自动粘贴", this);
    m_defaultAutoSubmitCheck = new QCheckBox("自动提交", this);
    defaultBehaviorLayout->addWidget(m_defaultAutoPasteCheck);
    defaultBehaviorLayout->addWidget(m_defaultAutoSubmitCheck);

    mainLayout->addWidget(defaultBehaviorGroup);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("保存", this);
    QPushButton *cancelBtn = new QPushButton("取消", this);

    bottomLayout->addStretch();
    bottomLayout->addWidget(saveBtn);
    bottomLayout->addWidget(cancelBtn);

    mainLayout->addLayout(bottomLayout);

    connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::addHotkey);
    connect(editBtn, &QPushButton::clicked, this, &SettingsDialog::editHotkey);
    connect(removeBtn, &QPushButton::clicked, this, &SettingsDialog::removeHotkey);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveConfig);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::loadConfigToUI()
{
    m_hotkeyList->clear();
    for (const auto &binding : m_config.hotkeys) {
        QString behaviorInfo;
        if (binding.behavior.autoPaste) {
            behaviorInfo += "自动粘贴";
        }
        if (binding.behavior.autoSubmit) {
            if (!behaviorInfo.isEmpty()) {
                behaviorInfo += ", ";
            }
            behaviorInfo += "自动提交";
        }
        if (behaviorInfo.isEmpty()) {
            behaviorInfo = "无";
        }
        QString itemText = QString("%1 - %2 [%3]")
            .arg(binding.action)
            .arg(binding.keySequence.toString())
            .arg(behaviorInfo);
        m_hotkeyList->addItem(itemText);
    }

    m_formatEdit->setText(m_config.output.format);
    m_customPathEdit->setText(m_config.output.dir);
    m_defaultAutoPasteCheck->setChecked(m_config.defaultBehavior.autoPaste);
    m_defaultAutoSubmitCheck->setChecked(m_config.defaultBehavior.autoSubmit);
}

void SettingsDialog::addHotkey()
{
    QMessageBox::information(this, "提示", "编辑功能请在 config.json 中手动添加。\n完整 GUI 编辑功能正在开发中。");
}

void SettingsDialog::editHotkey()
{
    QMessageBox::information(this, "提示", "编辑功能请在 config.json 中手动修改。\n完整 GUI 编辑功能正在开发中。");
}

void SettingsDialog::removeHotkey()
{
    QMessageBox::information(this, "提示", "删除功能请在 config.json 中手动修改。\n完整 GUI 编辑功能正在开发中。");
}

void SettingsDialog::saveConfig()
{
    m_config.output.format = m_formatEdit->text();
    m_config.output.dir = m_customPathEdit->text();
    m_config.defaultBehavior.autoPaste = m_defaultAutoPasteCheck->isChecked();
    m_config.defaultBehavior.autoSubmit = m_defaultAutoSubmitCheck->isChecked();

    m_config.save(QFileInfo(QCoreApplication::applicationDirPath()).filePath("config.json"));

    QMessageBox::information(this, "成功", "配置已保存！\n请重启程序生效。");
    accept();
}

} // namespace ClipBridge
