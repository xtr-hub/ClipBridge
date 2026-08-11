#include "InputPanel.h"
#include "core/ActionManager.h"
#include "utils/ClipboardHelper.h"
#include "utils/Simulator.h"
#include <QApplication>
#include <QClipboard>
#include <QHBoxLayout>
#include <QMimeData>
#include <QUrl>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QScrollArea>
#include <QFrame>
#include <QStyle>
#include <QTimer>
#include <QDebug>

namespace ClipBridge {

InputPanel::InputPanel(ActionManager *actionManager, QWidget *parent)
    : QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint)
    , m_actionManager(actionManager)
{
    setWindowTitle(tr("ClipBridge 输入面板"));
    setMinimumSize(520, 380);
    resize(580, 460);

    setupUI();
    applyStyle();

    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &InputPanel::onClipboardChanged);
}

void InputPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Preview area — horizontal row of compact cards, ~5 fit without scrolling
    // Qt 6 natively forwards wheel to horizontal scrollbar when vertical has no range
    m_previewArea = new QScrollArea(this);
    m_previewArea->setObjectName("previewArea");
    m_previewArea->setFixedHeight(28);
    m_previewArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_previewArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_previewArea->setWidgetResizable(false);
    m_previewArea->hide();

    m_previewContent = new QWidget();
    m_previewContent->setObjectName("previewContent");
    m_previewLayout = new QHBoxLayout(m_previewContent);
    m_previewLayout->setSpacing(4);
    m_previewLayout->setContentsMargins(6, 3, 6, 3);
    m_previewLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    m_previewArea->setWidget(m_previewContent);

    // Redirect mouse wheel to horizontal scrollbar
    m_previewArea->viewport()->installEventFilter(this);

    mainLayout->addWidget(m_previewArea);

    // Chat editor
    m_chatEdit = new QTextEdit(this);
    m_chatEdit->setObjectName("chatEdit");
    m_chatEdit->setAcceptRichText(false);
    m_chatEdit->setPlaceholderText(
        tr("在此输入提示词，Ctrl+Enter 发送到剪贴板\n"
           "Ctrl+V 粘贴图片或文件到上方预览区"));
    m_chatEdit->installEventFilter(this);
    mainLayout->addWidget(m_chatEdit, 1);

    // Bottom bar
    QWidget *bottomBar = new QWidget(this);
    bottomBar->setObjectName("bottomBar");
    QHBoxLayout *bl = new QHBoxLayout(bottomBar);
    bl->setContentsMargins(14, 8, 14, 10);
    bl->setSpacing(10);

    m_pinCheck = new QCheckBox(tr("钉住"), this);
    m_pinCheck->setToolTip(tr("发送后不关闭面板"));
    m_saveLongTextCheck = new QCheckBox(tr("长文本存为文件"), this);
    m_saveLongTextCheck->setToolTip(tr("超阈值的文本自动存为 .txt 文件"));

    bl->addWidget(m_pinCheck);
    bl->addWidget(m_saveLongTextCheck);
    bl->addStretch();

    m_cancelBtn = new QPushButton(tr("取消"), this);
    m_cancelBtn->setMinimumWidth(72);
    m_cancelBtn->setMinimumHeight(30);
    bl->addWidget(m_cancelBtn);

    m_sendBtn = new QPushButton(tr("发送"), this);
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setMinimumWidth(80);
    m_sendBtn->setMinimumHeight(30);
    bl->addWidget(m_sendBtn);

    mainLayout->addWidget(bottomBar);

    connect(m_sendBtn, &QPushButton::clicked, this, &InputPanel::onSendClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QWidget::close);
}

void InputPanel::applyStyle()
{
    setStyleSheet(QString(
        "InputPanel { background: #1e1e1e; }"
        "QScrollArea#previewArea { background: #252526; border: none;"
        "  border-bottom: 1px solid #3e3e42; }"
        "QWidget#previewContent { background: #252526; }"
        "QTextEdit#chatEdit { background: #1e1e1e; color: #cccccc; border: none;"
        "  padding: 12px 14px; font-size: 13px; selection-background-color: #264f78; }"
        "QWidget#bottomBar { background: #252526; border-top: 1px solid #3e3e42; }"
        "QCheckBox { color: #8b949e; spacing: 6px; }"
        "QCheckBox::indicator { width: 15px; height: 15px; }"
        "QPushButton { background: #3e3e42; color: #cccccc; border: none;"
        "  border-radius: 5px; padding: 6px 16px; font-size: 13px; }"
        "QPushButton:hover { background: #505050; }"
        "QPushButton:pressed { background: #2d2d2d; }"
        "QPushButton#sendBtn { background: #0078d4; color: white; font-weight: bold; }"
        "QPushButton#sendBtn:hover { background: #1a8ad4; }"
        "QScrollBar:vertical { background: #1e1e1e; width: 8px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #424242; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }"
        "QScrollBar:horizontal { background: #252526; height: 5px; border-radius: 2px; }"
        "QScrollBar::handle:horizontal { background: #424242; border-radius: 2px; min-width: 24px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }"
    ));
}

void InputPanel::updateConfig(const AppConfig &config)
{
    m_config = config;
    m_saveLongTextCheck->setChecked(config.inputPanel.saveLongTextAsFile);
    setWindowOpacity(qBound(0.5, config.inputPanel.windowOpacity / 100.0, 1.0));
}

void InputPanel::showAndRefresh()
{
    m_chatEdit->clear();
    clearPreview();
    m_pendingItems.clear();
    m_pinCheck->setChecked(false);
    m_pinned = false;

    show();
    raise();
    activateWindow();
    m_chatEdit->setFocus();
    emit panelOpened();
}

void InputPanel::closeEvent(QCloseEvent *event)
{
    if (m_pinned) {
        hide();
        event->ignore();
    } else {
        emit panelClosed();
        event->accept();
    }
}

bool InputPanel::eventFilter(QObject *obj, QEvent *event)
{
    // Redirect mouse wheel on preview viewport → horizontal scroll
    if (obj == m_previewArea->viewport() && event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        int delta = we->angleDelta().y();
        if (delta != 0) {
            QScrollBar *hbar = m_previewArea->horizontalScrollBar();
            hbar->setValue(hbar->value() - delta);
        }
        return true;
    }

    if (obj == m_chatEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Return && (ke->modifiers() & Qt::ControlModifier)) {
            onSendClicked();
            return true;
        }

        if (ke->matches(QKeySequence::Paste)) {
            const QMimeData *mime = QApplication::clipboard()->mimeData();
            if (!mime) return false;

            if (mime->hasImage()) {
                QImage image = ClipboardHelper::getImage();
                if (!image.isNull()) {
                    PendingItem item;
                    item.type = PendingItem::Image;
                    item.image = image;
                    m_pendingItems.append(item);
                    addPreviewItem(item);
                }
                return true;
            }

            if (mime->hasUrls()) {
                QStringList paths = ClipboardHelper::getFilePaths();
                if (!paths.isEmpty()) {
                    for (const QString &p : paths) {
                        PendingItem item;
                        item.type = PendingItem::File;
                        item.filePath = p;
                        m_pendingItems.append(item);
                        addPreviewItem(item);
                    }
                }
                return true;
            }

            // Plain text paste — let it through to the editor
            return false;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void InputPanel::onClipboardChanged()
{
    // No-op
}

QString InputPanel::processContent()
{
    QStringList parts;

    // 1. Process pasted images/files — each sets clipboard and calls process
    for (const PendingItem &item : m_pendingItems) {
        if (item.type == PendingItem::Image) {
            QApplication::clipboard()->setImage(item.image);
            QString r = m_actionManager->process("clipboard_image_path");
            if (!r.isEmpty()) parts.append(r);
        } else {
            QMimeData *mime = new QMimeData();
            QList<QUrl> urls;
            urls.append(QUrl::fromLocalFile(item.filePath));
            mime->setUrls(urls);
            QApplication::clipboard()->setMimeData(mime, QClipboard::Clipboard);
            QString r = m_actionManager->process("clipboard_file_path");
            if (!r.isEmpty()) parts.append(r);
        }
    }

    // 2. Append user's typed prompt after the image/file results
    QString text = m_chatEdit->toPlainText().trimmed();
    if (!text.isEmpty()) {
        int threshold = m_config.monitor.longTextThreshold;
        if (m_saveLongTextCheck->isChecked() && threshold > 0 && text.length() > threshold) {
            QApplication::clipboard()->setText(text);
            QString r = m_actionManager->process("clipboard_long_text");
            if (!r.isEmpty()) parts.append(r);
        } else {
            parts.append(text);
        }
    }

    return parts.join("\n");
}

void InputPanel::onSendClicked()
{
    QString result = processContent();
    if (result.isEmpty()) {
        qDebug() << "InputPanel: nothing to send";
        return;
    }

    ClipboardHelper::setText(result);

    m_pinned = m_pinCheck->isChecked();

    if (m_pinned) {
        // Stay open, clear inputs for next use
        m_chatEdit->clear();
        clearPreview();
        m_pendingItems.clear();
        m_chatEdit->setFocus();
    } else {
        close();
    }
}

void InputPanel::clearPreview()
{
    while (m_previewLayout->count() > 0) {
        QLayoutItem *item = m_previewLayout->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_previewArea->hide();
}

void InputPanel::addPreviewItem(const PendingItem &item)
{
    int idx = m_pendingItems.size() - 1; // index of the item we just appended

    // Compact horizontal card: ~110px wide, 18px tall, icon + filename + close
    QFrame *card = new QFrame();
    card->setFixedSize(106, 20);
    card->setStyleSheet(
        "QFrame { background:#2d2d30; border:1px solid #3e3e42;"
        " border-radius:3px; }");
    card->setProperty("pendingIdx", idx);

    QHBoxLayout *cl = new QHBoxLayout(card);
    cl->setContentsMargins(3, 1, 0, 1);
    cl->setSpacing(3);

    // Left icon area (14×14)
    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(14, 14);
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background:#1e1e1e; border:none; border-radius:2px;");

    QString text;
    if (item.type == PendingItem::Image) {
        QPixmap pm = QPixmap::fromImage(
            item.image.scaled(12, 12, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setPixmap(pm);
        text = QString("%1x%2").arg(item.image.width()).arg(item.image.height());
    } else {
        QIcon fileIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);
        iconLabel->setPixmap(fileIcon.pixmap(12, 12));
        QFileInfo fi(item.filePath);
        text = fi.fileName();
    }

    cl->addWidget(iconLabel);

    // Filename / dimensions
    QLabel *nameLabel = new QLabel(text);
    nameLabel->setStyleSheet(
        "color:#cccccc; font-size:10px; background:transparent; border:none;");
    nameLabel->setTextFormat(Qt::PlainText);
    QFontMetrics fm(nameLabel->font());
    nameLabel->setText(fm.elidedText(text, Qt::ElideRight, 60));
    cl->addWidget(nameLabel);

    // Close button
    QPushButton *closeBtn = new QPushButton(QString::fromUtf8("\xc3\x97")); // ×
    closeBtn->setFixedSize(18, 18);
    closeBtn->setCursor(Qt::ArrowCursor);
    closeBtn->setStyleSheet(
        "QPushButton { background:transparent; color:#8b949e; border:none;"
        " font-size:12px; font-weight:bold; padding:0; }"
        "QPushButton:hover { color:#ffffff; background:#c42b1c; border-radius:2px; }");
    cl->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, this, [this, card]() {
        // Read fresh index from property (updated by previous removals)
        bool ok = false;
        int curIdx = card->property("pendingIdx").toInt(&ok);
        if (!ok) return;

        // Remove from data model at current position
        if (curIdx >= 0 && curIdx < m_pendingItems.size())
            m_pendingItems.removeAt(curIdx);
        // Remove card widget
        m_previewLayout->removeWidget(card);
        card->deleteLater();
        // Refresh indices on remaining cards
        for (int i = 0; i < m_previewLayout->count(); ++i) {
            QLayoutItem *li = m_previewLayout->itemAt(i);
            if (li && li->widget())
                li->widget()->setProperty("pendingIdx", i);
        }
        if (m_previewLayout->count() == 0)
            m_previewArea->hide();
        else
            m_previewContent->adjustSize();
    });

    m_previewLayout->addWidget(card);
    m_previewContent->adjustSize();
    m_previewArea->show();
}

} // namespace ClipBridge
