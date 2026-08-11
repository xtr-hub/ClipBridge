#include "SettingsDialog.h"
#include "HotkeyEditDialog.h"
#include "core/AutoStart.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
namespace ClipBridge {

SettingsDialog::SettingsDialog(const AppConfig &config, QWidget *parent)
    : QDialog(parent), m_config(config)
{
    setWindowTitle(tr("ClipBridge 设置"));
    setMinimumSize(620, 500);
    resize(640, 540);

    setupUI();
    applyStyle();
    loadConfigToUI();
}

void SettingsDialog::setupUI()
{
    QHBoxLayout *root = new QHBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    setupNav();
    root->addWidget(m_nav);

    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(0);
    rightLayout->setContentsMargins(20, 16, 20, 16);

    m_pages = new QStackedWidget(this);
    setupShortcutsPage();
    setupOutputPage();
    setupMonitorPage();
    setupGeneralPage();
    m_pages->setCurrentIndex(0);
    rightLayout->addWidget(m_pages, 1);

    QHBoxLayout *bottom = new QHBoxLayout();
    bottom->setSpacing(10);
    bottom->setContentsMargins(0, 12, 0, 0);
    bottom->addStretch();

    m_saveBtn = new QPushButton(tr("保存"), this);
    m_saveBtn->setObjectName("saveBtn");
    m_saveBtn->setMinimumWidth(100);
    m_saveBtn->setMinimumHeight(32);

    m_cancelBtn = new QPushButton(tr("取消"), this);
    m_cancelBtn->setMinimumWidth(100);
    m_cancelBtn->setMinimumHeight(32);

    bottom->addWidget(m_saveBtn);
    bottom->addWidget(m_cancelBtn);
    rightLayout->addLayout(bottom);
    root->addWidget(rightPanel, 1);

    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::saveConfig);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::setupNav()
{
    m_nav = new QListWidget(this);
    m_nav->setFixedWidth(140);
    m_nav->setSpacing(0);
    m_nav->addItem(tr("快捷键"));
    m_nav->addItem(tr("输出格式"));
    m_nav->addItem(tr("监听模式"));
    m_nav->addItem(tr("通用设置"));
    m_nav->setCurrentRow(0);
    m_nav->setObjectName("nav");
    connect(m_nav, &QListWidget::currentRowChanged, this, &SettingsDialog::onNavChanged);
}

void SettingsDialog::setupShortcutsPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setSpacing(12);
    l->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel(tr("快捷键管理"), this);
    title->setObjectName("pageTitle");
    l->addWidget(title);

    m_hotkeyList = new QListWidget(this);
    m_hotkeyList->setMinimumHeight(140);
    l->addWidget(m_hotkeyList);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *addBtn = new QPushButton(tr("添加"), this);
    QPushButton *editBtn = new QPushButton(tr("编辑"), this);
    QPushButton *removeBtn = new QPushButton(tr("删除"), this);
    btnRow->addWidget(addBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(removeBtn);
    btnRow->addStretch();
    l->addLayout(btnRow);

    QHBoxLayout *ipRow = new QHBoxLayout();
    ipRow->addWidget(new QLabel(tr("输入面板快捷键:"), this));
    m_inputPanelHotkeyEdit = new QKeySequenceEdit(this);
    m_inputPanelHotkeyEdit->setMinimumHeight(28);
    ipRow->addWidget(m_inputPanelHotkeyEdit, 1);
    QPushButton *clearHotkeyBtn = new QPushButton(tr("重置"), this);
    clearHotkeyBtn->setMinimumWidth(60);
    clearHotkeyBtn->setMinimumHeight(28);
    clearHotkeyBtn->setMaximumHeight(28);
    connect(clearHotkeyBtn, &QPushButton::clicked, [this]() {
        m_inputPanelHotkeyEdit->clear();
    });
    ipRow->addWidget(clearHotkeyBtn);
    l->addLayout(ipRow);

    l->addStretch();

    connect(addBtn, &QPushButton::clicked, this, &SettingsDialog::addHotkey);
    connect(editBtn, &QPushButton::clicked, this, &SettingsDialog::editHotkey);
    connect(removeBtn, &QPushButton::clicked, this, &SettingsDialog::removeHotkey);

    m_pages->addWidget(page);
}

void SettingsDialog::setupOutputPage()
{
    QWidget *page = new QWidget(this);
    QFormLayout *form = new QFormLayout(page);
    form->setSpacing(12);
    form->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel(tr("输出格式设置"), this);
    title->setObjectName("pageTitle");
    form->addRow(title);

    m_actionFormatCombo = new QComboBox(this);
    m_actionFormatCombo->setMinimumHeight(28);
    for (const QString &a : AppConfig::availableActions())
        m_actionFormatCombo->addItem(AppConfig::actionDisplayName(a), a);
    form->addRow(tr("动作:"), m_actionFormatCombo);

    m_formatEdit = new QLineEdit(this);
    m_formatEdit->setMinimumHeight(28);
    m_formatEdit->setPlaceholderText(tr("例如: {path}"));
    m_useSpecificFormatCheck = new QCheckBox(tr("专属格式"), this);
    QHBoxLayout *fmtRow = new QHBoxLayout();
    fmtRow->addWidget(m_formatEdit, 1);
    fmtRow->addWidget(m_useSpecificFormatCheck);
    form->addRow(tr("格式:"), fmtRow);

    m_pathModeCombo = new QComboBox(this);
    m_pathModeCombo->setMinimumHeight(28);
    m_pathModeCombo->addItem(tr("工作区默认路径"), "workspace");
    m_pathModeCombo->addItem(tr("自定义路径"), "custom_path");
    form->addRow(tr("保存模式:"), m_pathModeCombo);

    QHBoxLayout *pathRow = new QHBoxLayout();
    m_customPathEdit = new QLineEdit(this);
    m_customPathEdit->setMinimumHeight(28);
    m_customPathEdit->setPlaceholderText(tr("选择自定义保存路径"));
    m_browseBtn = new QPushButton(tr("浏览..."), this);
    m_browseBtn->setFixedWidth(80);
    pathRow->addWidget(m_customPathEdit, 1);
    pathRow->addWidget(m_browseBtn);
    form->addRow(tr("保存路径:"), pathRow);

    m_pasteKeyCombo = new QComboBox(this);
    m_pasteKeyCombo->setMinimumHeight(28);
    m_pasteKeyCombo->addItem("Ctrl+V", "Ctrl+V");
    m_pasteKeyCombo->addItem("Ctrl+Shift+V", "Ctrl+Shift+V");
    m_pasteKeyCombo->addItem("Shift+Insert", "Shift+Insert");
    m_pasteKeyCombo->addItem("Cmd+V", "Cmd+V");
    form->addRow(tr("粘贴快捷键:"), m_pasteKeyCombo);

    QHBoxLayout *delayRow = new QHBoxLayout();
    m_pasteDelaySlider = new QSlider(Qt::Horizontal, this);
    m_pasteDelaySlider->setRange(20, 1000);
    m_pasteDelaySlider->setValue(100);
    m_pasteDelaySlider->setMinimumHeight(28);
    m_pasteDelayValue = new QLabel("100 ms", this);
    m_pasteDelayValue->setMinimumWidth(56);
    m_pasteDelayValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(m_pasteDelaySlider, &QSlider::valueChanged, [this](int v) {
        m_pasteDelayValue->setText(QString("%1 ms").arg(v));
    });
    delayRow->addWidget(m_pasteDelaySlider, 1);
    delayRow->addWidget(m_pasteDelayValue);
    form->addRow(tr("粘贴延迟:"), delayRow);

    form->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(m_browseBtn, &QPushButton::clicked, this, &SettingsDialog::browsePath);
    connect(m_pathModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onPathModeChanged);
    connect(m_actionFormatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SettingsDialog::onActionFormatChanged);
    connect(m_useSpecificFormatCheck, &QCheckBox::clicked,
            this, &SettingsDialog::onUseSpecificFormatChanged);
    connect(m_formatEdit, &QLineEdit::textChanged,
            this, &SettingsDialog::onFormatTextChanged);

    m_pages->addWidget(page);
}

void SettingsDialog::setupMonitorPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setSpacing(12);
    l->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel(tr("监听模式设置"), this);
    title->setObjectName("pageTitle");
    l->addWidget(title);

    m_monitorEnabledCheck = new QCheckBox(tr("启用监听模式"), this);
    m_monitorEnabledCheck->setToolTip(tr("开启后剪贴板变化时自动触发已选动作"));
    l->addWidget(m_monitorEnabledCheck);

    QGroupBox *autoGrp = new QGroupBox(tr("自动触发动作"), this);
    QVBoxLayout *al = new QVBoxLayout(autoGrp);
    m_autoImageCheck = new QCheckBox(tr("图片 → 路径"), this);
    m_autoFileCheck = new QCheckBox(tr("文件 → 路径"), this);
    m_autoStripNewlinesCheck = new QCheckBox(tr("去除换行符"), this);
    m_autoLongTextCheck = new QCheckBox(tr("长文本 → 文件"), this);
    al->addWidget(m_autoImageCheck);
    al->addWidget(m_autoFileCheck);
    al->addWidget(m_autoStripNewlinesCheck);
    al->addWidget(m_autoLongTextCheck);
    l->addWidget(autoGrp);

    QHBoxLayout *threshRow = new QHBoxLayout();
    threshRow->addWidget(new QLabel(tr("长文本阈值:"), this));
    m_longTextThresholdSlider = new QSlider(Qt::Horizontal, this);
    m_longTextThresholdSlider->setRange(0, 10000);
    m_longTextThresholdSlider->setValue(500);
    m_longTextThresholdSlider->setMinimumHeight(28);
    m_longTextThresholdSlider->setToolTip(tr("超出此长度的文本自动存文件，0 表示禁用"));
    m_longTextThresholdValue = new QLabel("500", this);
    m_longTextThresholdValue->setMinimumWidth(48);
    m_longTextThresholdValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(m_longTextThresholdSlider, &QSlider::valueChanged, [this](int v) {
        if (v == 0)
            m_longTextThresholdValue->setText(tr("禁用"));
        else
            m_longTextThresholdValue->setText(QString::number(v));
    });
    threshRow->addWidget(m_longTextThresholdSlider, 1);
    threshRow->addWidget(m_longTextThresholdValue);
    l->addLayout(threshRow);

    m_monitorNotifyCheck = new QCheckBox(tr("执行后弹出通知"), this);
    l->addWidget(m_monitorNotifyCheck);

    l->addStretch();
    m_pages->addWidget(page);
}

void SettingsDialog::setupGeneralPage()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *l = new QVBoxLayout(page);
    l->setSpacing(12);
    l->setContentsMargins(0, 0, 0, 0);

    QLabel *title = new QLabel(tr("通用设置"), this);
    title->setObjectName("pageTitle");
    l->addWidget(title);

    QGroupBox *bhvGrp = new QGroupBox(tr("默认行为（新热键）"), this);
    QVBoxLayout *bl = new QVBoxLayout(bhvGrp);
    m_defaultAutoPasteCheck = new QCheckBox(tr("自动粘贴"), this);
    m_defaultAutoSubmitCheck = new QCheckBox(tr("自动提交"), this);
    bl->addWidget(m_defaultAutoPasteCheck);
    bl->addWidget(m_defaultAutoSubmitCheck);
    l->addWidget(bhvGrp);

    QHBoxLayout *langRow = new QHBoxLayout();
    langRow->addWidget(new QLabel(tr("界面语言:"), this));
    m_languageCombo = new QComboBox(this);
    m_languageCombo->setMinimumHeight(28);
    m_languageCombo->addItem(tr("简体中文"), "zh_CN");
    m_languageCombo->addItem(tr("English"), "en_US");
    langRow->addWidget(m_languageCombo, 1);
    l->addLayout(langRow);

    QHBoxLayout *asRow = new QHBoxLayout();
    m_autoStartCheck = new QCheckBox(tr("开机自启"), this);
    asRow->addWidget(m_autoStartCheck);
    asRow->addStretch();
    l->addLayout(asRow);

    QHBoxLayout *opacityRow = new QHBoxLayout();
    opacityRow->addWidget(new QLabel(tr("窗口透明度:"), this));
    m_opacitySlider = new QSlider(Qt::Horizontal, this);
    m_opacitySlider->setRange(50, 100);
    m_opacitySlider->setValue(95);
    m_opacitySlider->setMinimumHeight(28);
    m_opacityValue = new QLabel("95 %", this);
    m_opacityValue->setMinimumWidth(48);
    m_opacityValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    connect(m_opacitySlider, &QSlider::valueChanged, [this](int v) {
        m_opacityValue->setText(QString::number(v) + " %");
        setWindowOpacity(v / 100.0);
    });
    opacityRow->addWidget(m_opacitySlider, 1);
    opacityRow->addWidget(m_opacityValue);
    l->addLayout(opacityRow);

    l->addStretch();

    m_versionLabel = new QLabel(
        QString("ClipBridge v%1").arg(QCoreApplication::applicationVersion()), this);
    m_versionLabel->setObjectName("versionLabel");
    m_versionLabel->setAlignment(Qt::AlignCenter);
    l->addWidget(m_versionLabel);

    m_pages->addWidget(page);
}

void SettingsDialog::applyStyle()
{
    setStyleSheet(
        "QListWidget#nav { background: #252526; border: none; outline: none; padding: 8px 0; }"
        "QListWidget#nav::item { color: #8b949e; padding: 10px 18px; border: none; }"
        "QListWidget#nav::item:selected { color: #cccccc; background: #094771;"
        "  border-left: 3px solid #0078d4; padding-left: 15px; }"
        "QListWidget#nav::item:hover:!selected { color: #ffffff; background: #094771; }"
        "SettingsDialog { background: #1e1e1e; }"
        "QLabel { color: #cccccc; }"
        "QLabel#pageTitle { font-size: 18px; font-weight: bold; color: #4fc1ff;"
        "  padding-bottom: 8px; border-bottom: 1px solid #3e3e42; margin-bottom: 4px; }"
        "QLabel#versionLabel { color: #505050; font-size: 12px; }"
        "QGroupBox { color: #cccccc; border: 1px solid #3e3e42; border-radius: 8px;"
        "  margin-top: 12px; padding-top: 16px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; }"
        "QLineEdit, QComboBox, QKeySequenceEdit {"
        "  background: #3c3c3c; color: #cccccc; border: 1px solid #3e3e42;"
        "  border-radius: 6px; padding: 4px 10px; }"
        "QLineEdit:hover, QComboBox:hover, QKeySequenceEdit:hover {"
        "  border-color: #0078d4; background: #3a3a40; }"
        "QLineEdit:focus, QComboBox:focus, QKeySequenceEdit:focus {"
        "  border-color: #0078d4; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background: #3c3c3c; color: #cccccc;"
        "  border: 1px solid #3e3e42; outline: none; }"
        "QComboBox QAbstractItemView::item { padding: 5px 10px; }"
        "QComboBox QAbstractItemView::item:hover { background: #094771; }"
        "QComboBox QAbstractItemView::item:selected { background: #094771; }"
        "QCheckBox { color: #8b949e; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; }"
        "QPushButton { background: #3e3e42; color: #cccccc; border: none;"
        "  border-radius: 6px; padding: 6px 18px; }"
        "QPushButton:hover { background: #1a8ad4; }"
        "QPushButton:pressed { background: #0078d4; }"
        "QPushButton#saveBtn { background: #0078d4; color: white; font-weight: bold; }"
        "QPushButton#saveBtn:hover { background: #1a8ad4; }"
        "QSlider::groove:horizontal { background: #3c3c3c; height: 6px; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: #0078d4; width: 16px; height: 16px;"
        "  margin: -5px 0; border-radius: 8px; }"
        "QSlider::handle:horizontal:hover { background: #1a8ad4; }"
        "QSlider::sub-page:horizontal { background: #0078d4; border-radius: 3px; }"
    );
}

void SettingsDialog::onNavChanged(int row)
{
    if (row >= 0 && row < m_pages->count())
        m_pages->setCurrentIndex(row);
}

// ---- load / save ----

void SettingsDialog::loadConfigToUI()
{
    refreshHotkeyList();
    m_inputPanelHotkeyEdit->setKeySequence(m_config.inputPanelHotkey);

    int mi = m_pathModeCombo->findData(m_config.output.mode);
    if (mi >= 0) m_pathModeCombo->setCurrentIndex(mi);
    m_customPathEdit->setText(m_config.output.dir);

    int pk = m_pasteKeyCombo->findData(m_config.output.pasteKey);
    if (pk < 0) pk = 0;
    m_pasteKeyCombo->setCurrentIndex(pk);
    m_pasteDelaySlider->setValue(m_config.output.pasteDelay);

    QString action = m_actionFormatCombo->currentData().toString();
    m_currentAction = action;
    refreshFormatUIForAction(action);

    m_monitorEnabledCheck->setChecked(m_config.monitor.enabled);
    m_autoImageCheck->setChecked(m_config.monitor.autoActions.contains("clipboard_image_path"));
    m_autoFileCheck->setChecked(m_config.monitor.autoActions.contains("clipboard_file_path"));
    m_autoStripNewlinesCheck->setChecked(m_config.monitor.autoActions.contains("strip_newlines"));
    m_autoLongTextCheck->setChecked(m_config.monitor.autoActions.contains("clipboard_long_text"));
    m_longTextThresholdSlider->setValue(m_config.monitor.longTextThreshold);
    m_monitorNotifyCheck->setChecked(m_config.monitor.showNotification);

    m_defaultAutoPasteCheck->setChecked(m_config.defaultBehavior.autoPaste);
    m_defaultAutoSubmitCheck->setChecked(m_config.defaultBehavior.autoSubmit);
    int li = m_languageCombo->findData(m_config.language);
    if (li >= 0) m_languageCombo->setCurrentIndex(li);
    m_autoStartCheck->setChecked(m_config.autoStart);
    m_opacitySlider->setValue(m_config.inputPanel.windowOpacity);
    m_opacityValue->setText(QString::number(m_config.inputPanel.windowOpacity) + " %");

    updatePathInputState();
}

void SettingsDialog::refreshHotkeyList()
{
    m_hotkeyList->clear();
    for (const auto &b : m_config.hotkeys) {
        QString info;
        if (b.behavior.autoPaste) info += tr("自动粘贴");
        if (b.behavior.autoSubmit) info = info.isEmpty() ? tr("自动提交")
                                                        : info + ", " + tr("自动提交");
        if (info.isEmpty()) info = tr("无");
        m_hotkeyList->addItem(
            QString("%1 - %2 [%3]").arg(AppConfig::actionDisplayName(b.action),
                b.keySequence.toString(), info));
    }
}

void SettingsDialog::saveConfig()
{
    saveCurrentActionFormat();

    m_config.inputPanelHotkey = m_inputPanelHotkeyEdit->keySequence();

    m_config.output.mode = m_pathModeCombo->currentData().toString();
    m_config.output.dir = m_customPathEdit->text();
    m_config.output.pasteKey = m_pasteKeyCombo->currentData().toString();
    m_config.output.pasteDelay = m_pasteDelaySlider->value();

    m_config.monitor.enabled = m_monitorEnabledCheck->isChecked();
    m_config.monitor.autoActions.clear();
    if (m_autoImageCheck->isChecked())    m_config.monitor.autoActions.insert("clipboard_image_path");
    if (m_autoFileCheck->isChecked())     m_config.monitor.autoActions.insert("clipboard_file_path");
    if (m_autoStripNewlinesCheck->isChecked()) m_config.monitor.autoActions.insert("strip_newlines");
    if (m_autoLongTextCheck->isChecked()) m_config.monitor.autoActions.insert("clipboard_long_text");
    m_config.monitor.longTextThreshold = m_longTextThresholdSlider->value();
    m_config.monitor.showNotification = m_monitorNotifyCheck->isChecked();

    m_config.defaultBehavior.autoPaste = m_defaultAutoPasteCheck->isChecked();
    m_config.defaultBehavior.autoSubmit = m_defaultAutoSubmitCheck->isChecked();
    m_config.language = m_languageCombo->currentData().toString();
    m_config.autoStart = m_autoStartCheck->isChecked();
    m_config.inputPanel.windowOpacity = m_opacitySlider->value();

    m_config.save(QDir(QCoreApplication::applicationDirPath()).filePath("config.json"));
    accept();
}

// ---- action format logic ----

void SettingsDialog::refreshFormatUIForAction(const QString &action)
{
    m_updatingFormatUI = true;
    bool configurable = isFormatConfigurableAction(action);
    m_useSpecificFormatCheck->setEnabled(configurable);
    m_formatEdit->setEnabled(configurable);

    if (!configurable) {
        m_useSpecificFormatCheck->setChecked(false);
        m_formatEdit->clear();
        m_formatEdit->setPlaceholderText(tr("此动作不使用输出格式"));
    } else {
        bool hasSpecific = m_config.output.formats.contains(action);
        m_useSpecificFormatCheck->setChecked(hasSpecific);
        m_formatEdit->setText(hasSpecific ? m_config.output.formats[action] : m_config.output.format);
        m_formatEdit->setPlaceholderText(hasSpecific ? tr("例如: 请查看 {path}") : tr("例如: {path}"));
    }

    bool needsPath = isSavePathConfigurableAction(action);
    m_pathModeCombo->setEnabled(needsPath);
    m_customPathEdit->setEnabled(needsPath);
    m_browseBtn->setEnabled(needsPath);
    if (!needsPath) {
        m_customPathEdit->setEnabled(false);
        m_browseBtn->setEnabled(false);
    } else {
        updatePathInputState();
    }
    m_updatingFormatUI = false;
}

void SettingsDialog::updatePathInputState()
{
    bool isCustom = m_pathModeCombo->currentData().toString() == "custom_path"
                    && isSavePathConfigurableAction(m_currentAction);
    m_customPathEdit->setEnabled(isCustom);
    m_browseBtn->setEnabled(isCustom);
}

void SettingsDialog::onPathModeChanged(int) { updatePathInputState(); }

void SettingsDialog::browsePath()
{
    QString init = m_customPathEdit->text();
    if (init.isEmpty())
        init = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择保存目录"), init);
    if (!dir.isEmpty()) m_customPathEdit->setText(dir);
}

void SettingsDialog::saveCurrentActionFormat()
{
    if (m_updatingFormatUI || !isFormatConfigurableAction(m_currentAction)) return;
    if (m_useSpecificFormatCheck->isChecked())
        m_config.output.formats[m_currentAction] = m_formatEdit->text();
    else {
        m_config.output.format = m_formatEdit->text();
        if (m_config.output.formats.contains(m_currentAction))
            m_actionFormatBackups[m_currentAction] = m_config.output.formats.take(m_currentAction);
    }
}

void SettingsDialog::onActionFormatChanged(int)
{
    saveCurrentActionFormat();
    QString action = m_actionFormatCombo->currentData().toString();
    m_currentAction = action;
    refreshFormatUIForAction(action);
}

void SettingsDialog::onUseSpecificFormatChanged(bool checked)
{
    if (m_updatingFormatUI || !isFormatConfigurableAction(m_currentAction)) return;
    m_updatingFormatUI = true;
    if (checked) {
        if (m_actionFormatBackups.contains(m_currentAction)) {
            m_config.output.formats[m_currentAction] = m_actionFormatBackups.take(m_currentAction);
            m_formatEdit->setText(m_config.output.formats[m_currentAction]);
        } else if (m_config.output.formats.contains(m_currentAction)) {
            m_formatEdit->setText(m_config.output.formats[m_currentAction]);
        } else {
            m_config.output.formats[m_currentAction] = m_formatEdit->text();
        }
        m_formatEdit->setPlaceholderText(tr("例如: 请查看 {path}"));
    } else {
        if (m_config.output.formats.contains(m_currentAction))
            m_actionFormatBackups[m_currentAction] = m_config.output.formats.take(m_currentAction);
        m_formatEdit->setText(m_config.output.format);
        m_formatEdit->setPlaceholderText(tr("例如: {path}"));
    }
    m_updatingFormatUI = false;
}

void SettingsDialog::onFormatTextChanged(const QString &text)
{
    if (m_updatingFormatUI || !isFormatConfigurableAction(m_currentAction)) return;
    if (m_useSpecificFormatCheck->isChecked())
        m_config.output.formats[m_currentAction] = text;
    else
        m_config.output.format = text;
}

bool SettingsDialog::isFormatConfigurableAction(const QString &action) const
{
    return action != "strip_newlines";
}

bool SettingsDialog::isSavePathConfigurableAction(const QString &action) const
{
    return action == "clipboard_image_path" || action == "clipboard_long_text";
}

// ---- hotkey ops ----

void SettingsDialog::addHotkey()
{
    // Check if any actions are still available for assignment
    QSet<QString> assigned;
    for (const auto &b : m_config.hotkeys)
        assigned.insert(b.action);
    if (assigned.size() >= AppConfig::availableActions().size()) {
        QMessageBox box(QMessageBox::Information, tr("提示"),
            tr("所有动作都已分配了快捷键！请先删除或编辑现有热键。"),
            QMessageBox::NoButton, this);
        box.setStyleSheet("QMessageBox QLabel { qproperty-alignment: 'AlignVCenter | AlignLeft'; }");
        box.exec();
        return;
    }

    HotkeyEditDialog dialog(m_config, HotkeyEditDialog::AddMode, -1, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_config.hotkeys.append(dialog.hotkeyBinding());
        refreshHotkeyList();
    }
}

void SettingsDialog::editHotkey()
{
    int row = m_hotkeyList->currentRow();
    if (row < 0 || row >= m_config.hotkeys.size()) {
        QMessageBox box(QMessageBox::Information, tr("提示"), tr("请先选择要编辑的热键！"),
                        QMessageBox::NoButton, this);
        box.setStyleSheet("QMessageBox QLabel { qproperty-alignment: 'AlignVCenter | AlignLeft'; }");
        box.exec();
        return;
    }
    HotkeyEditDialog dialog(m_config, HotkeyEditDialog::EditMode, row, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_config.hotkeys[row] = dialog.hotkeyBinding();
        refreshHotkeyList();
    }
}

void SettingsDialog::removeHotkey()
{
    int row = m_hotkeyList->currentRow();
    if (row < 0 || row >= m_config.hotkeys.size()) {
        QMessageBox box(QMessageBox::Information, tr("提示"), tr("请先选择要删除的热键！"),
                        QMessageBox::NoButton, this);
        box.setStyleSheet("QMessageBox QLabel { qproperty-alignment: 'AlignVCenter | AlignLeft'; }");
        box.exec();
        return;
    }
    QMessageBox qbox(QMessageBox::Question, tr("确认删除"), tr("确定要删除这个热键吗？"),
                     QMessageBox::Yes | QMessageBox::No, this);
    qbox.setStyleSheet("QMessageBox QLabel { qproperty-alignment: 'AlignVCenter | AlignLeft'; }");
    qbox.setDefaultButton(QMessageBox::No);
    if (qbox.exec() == QMessageBox::Yes) {
        m_config.hotkeys.removeAt(row);
        refreshHotkeyList();
    }
}

} // namespace ClipBridge
