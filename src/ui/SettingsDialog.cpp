#include "SettingsDialog.h"
#include "HotkeyEditDialog.h"
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
    setMinimumSize(680, 620);
    resize(720, 650);

    setupUI();
    loadConfigToUI();
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(25, 25, 25, 25);

    // 热键组
    QGroupBox *hotkeyGroup = new QGroupBox("快捷键", this);
    QVBoxLayout *hotkeyLayout = new QVBoxLayout(hotkeyGroup);
    hotkeyLayout->setSpacing(12);
    hotkeyLayout->setContentsMargins(18, 22, 18, 18);

    m_hotkeyList = new QListWidget(this);
    m_hotkeyList->setMaximumHeight(150);
    hotkeyLayout->addWidget(m_hotkeyList);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
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
    outputLayout->setSpacing(15);
    outputLayout->setContentsMargins(18, 25, 18, 22);
    outputLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    m_formatEdit = new QLineEdit(this);
    m_formatEdit->setMinimumHeight(28);
    m_formatEdit->setPlaceholderText("例如: 请查看这张图片 {path}");
    outputLayout->addRow("格式:", m_formatEdit);

    m_pathModeCombo = new QComboBox(this);
    m_pathModeCombo->setMinimumHeight(28);
    m_pathModeCombo->addItem("工作区默认路径", "workspace");
    m_pathModeCombo->addItem("自定义路径", "custom_path");
    outputLayout->addRow("保存模式:", m_pathModeCombo);

    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->setSpacing(10);
    m_customPathEdit = new QLineEdit(this);
    m_customPathEdit->setMinimumHeight(28);
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
    defaultBehaviorLayout->setSpacing(12);
    defaultBehaviorLayout->setContentsMargins(18, 25, 18, 22);

    m_defaultAutoPasteCheck = new QCheckBox("自动粘贴", this);
    m_defaultAutoSubmitCheck = new QCheckBox("自动提交", this);
    defaultBehaviorLayout->addWidget(m_defaultAutoPasteCheck);
    defaultBehaviorLayout->addWidget(m_defaultAutoSubmitCheck);

    mainLayout->addWidget(defaultBehaviorGroup);

    mainLayout->addStretch();

    // 底部按钮
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);
    QPushButton *saveBtn = new QPushButton("保存", this);
    saveBtn->setMinimumWidth(90);
    saveBtn->setMinimumHeight(30);
    QPushButton *cancelBtn = new QPushButton("取消", this);
    cancelBtn->setMinimumWidth(90);
    cancelBtn->setMinimumHeight(30);

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
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dirPath.isEmpty()) {
        m_customPathEdit->setText(dirPath);
    }
}

void SettingsDialog::addHotkey()
{
    HotkeyEditDialog dialog(m_config, HotkeyEditDialog::AddMode, -1, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_config.hotkeys.append(dialog.hotkeyBinding());
        loadConfigToUI();
    }
}

void SettingsDialog::editHotkey()
{
    int currentRow = m_hotkeyList->currentRow();
    if (currentRow < 0 || currentRow >= m_config.hotkeys.size()) {
        QMessageBox::information(this, "提示", "请先选择要编辑的热键！");
        return;
    }

    HotkeyEditDialog dialog(m_config, HotkeyEditDialog::EditMode, currentRow, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_config.hotkeys[currentRow] = dialog.hotkeyBinding();
        loadConfigToUI();
    }
}

void SettingsDialog::removeHotkey()
{
    int currentRow = m_hotkeyList->currentRow();
    if (currentRow < 0 || currentRow >= m_config.hotkeys.size()) {
        QMessageBox::information(this, "提示", "请先选择要删除的热键！");
        return;
    }

    auto reply = QMessageBox::question(
        this,
        "确认删除",
        "确定要删除这个热键吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_config.hotkeys.removeAt(currentRow);
        loadConfigToUI();
    }
}

void SettingsDialog::saveConfig()
{
    m_config.output.format = m_formatEdit->text();
    m_config.output.mode = m_pathModeCombo->currentData().toString();
    m_config.output.dir = m_customPathEdit->text();
    m_config.defaultBehavior.autoPaste = m_defaultAutoPasteCheck->isChecked();
    m_config.defaultBehavior.autoSubmit = m_defaultAutoSubmitCheck->isChecked();

    m_config.save(QDir(QCoreApplication::applicationDirPath()).filePath("config.json"));

    accept();
}

} // namespace ClipBridge
