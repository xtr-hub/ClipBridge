#include "SettingsDialog.h"
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFileDialog>

namespace ClipBridge {

SettingsDialog::SettingsDialog(const AppConfig &config, QWidget *parent)
    : QDialog(parent), m_config(config)
{
    setWindowTitle("ClipBridge 设置");
    setMinimumSize(600, 500);
    resize(650, 550);

    setupUI();
    loadConfigToUI();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 热键组
    QGroupBox *hotkeyGroup = new QGroupBox("快捷键", this);
    QVBoxLayout *hotkeyLayout = new QVBoxLayout(hotkeyGroup);

    m_hotkeyList = new QListWidget(this);
    m_hotkeyList->setMaximumHeight(150);
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

    // 输出设置组
    QGroupBox *outputGroup = new QGroupBox("输出设置", this);
    QFormLayout *outputLayout = new QFormLayout(outputGroup);
    outputLayout->setSpacing(12);
    outputLayout->setContentsMargins(15, 20, 15, 20);

    m_formatEdit = new QLineEdit(this);
    m_formatEdit->setPlaceholderText("例如: 请查看这张图片 {path}");
    outputLayout->addRow("格式:", m_formatEdit);

    m_pathModeCombo = new QComboBox(this);
    m_pathModeCombo->addItem("工作区默认路径", "workspace");
    m_pathModeCombo->addItem("自定义路径", "custom_path");
    outputLayout->addRow("保存模式:", m_pathModeCombo);

    QHBoxLayout *pathLayout = new QHBoxLayout();
    m_customPathEdit = new QLineEdit(this);
    m_customPathEdit->setPlaceholderText("选择自定义图片保存路径");
    m_browseBtn = new QPushButton("浏览...", this);
    m_browseBtn->setMaximumWidth(100);

    pathLayout->addWidget(m_customPathEdit);
    pathLayout->addWidget(m_browseBtn);
    outputLayout->addRow("保存路径:", pathLayout);

    mainLayout->addWidget(outputGroup);

    // 默认行为配置组
    QGroupBox *defaultBehaviorGroup = new QGroupBox("默认行为配置（用于新添加的热键）", this);
    QVBoxLayout *defaultBehaviorLayout = new QVBoxLayout(defaultBehaviorGroup);
    defaultBehaviorLayout->setContentsMargins(15, 20, 15, 20);

    m_defaultAutoPasteCheck = new QCheckBox("自动粘贴", this);
    m_defaultAutoSubmitCheck = new QCheckBox("自动提交", this);
    defaultBehaviorLayout->addWidget(m_defaultAutoPasteCheck);
    defaultBehaviorLayout->addWidget(m_defaultAutoSubmitCheck);

    mainLayout->addWidget(defaultBehaviorGroup);

    mainLayout->addStretch();

    // 底部按钮
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    QPushButton *saveBtn = new QPushButton("保存", this);
    saveBtn->setMinimumWidth(80);
    QPushButton *cancelBtn = new QPushButton("取消", this);
    cancelBtn->setMinimumWidth(80);

    bottomLayout->addStretch();
    bottomLayout->addWidget(saveBtn);
    bottomLayout->addWidget(cancelBtn);

    mainLayout->addLayout(bottomLayout);

    // 连接信号槽
    connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::addHotkey);
    connect(editBtn, &QPushButton::clicked, this, &SettingsDialog::editHotkey);
    connect(removeBtn, &QPushButton::clicked, this, &SettingsDialog::removeHotkey);
    connect(saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveConfig);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_browseBtn, &QPushButton::clicked, this, &SettingsDialog::browsePath);
    connect(m_pathModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onPathModeChanged);
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

    int modeIndex = m_pathModeCombo->findData(m_config.output.mode);
    if (modeIndex >= 0) {
        m_pathModeCombo->setCurrentIndex(modeIndex);
    }

    m_customPathEdit->setText(m_config.output.dir);
    m_defaultAutoPasteCheck->setChecked(m_config.defaultBehavior.autoPaste);
    m_defaultAutoSubmitCheck->setChecked(m_config.defaultBehavior.autoSubmit);

    updatePathInputState();
}

void SettingsDialog::updatePathInputState()
{
    bool isCustom = m_pathModeCombo->currentData().toString() == "custom_path";
    m_customPathEdit->setEnabled(isCustom);
    m_browseBtn->setEnabled(isCustom);
}

void SettingsDialog::onPathModeChanged(int index)
{
    Q_UNUSED(index);
    updatePathInputState();
}

void SettingsDialog::browsePath()
{
    QString initialPath = m_customPathEdit->text();
    if (initialPath.isEmpty()) {
        initialPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "选择图片保存目录",
        initialPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    if (!dirPath.isEmpty()) {
        m_customPathEdit->setText(dirPath);
    }
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
    m_config.output.mode = m_pathModeCombo->currentData().toString();
    m_config.output.dir = m_customPathEdit->text();
    m_config.defaultBehavior.autoPaste = m_defaultAutoPasteCheck->isChecked();
    m_config.defaultBehavior.autoSubmit = m_defaultAutoSubmitCheck->isChecked();

    m_config.save(QDir(QCoreApplication::applicationDirPath()).filePath("config.json"));

    QMessageBox::information(this, "成功", "配置已保存！\n已自动热更新，无需重启程序。");
    accept();
}

} // namespace ClipBridge
