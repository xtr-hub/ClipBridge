#pragma once

#include "core/AppConfig.h"
#include <QWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVector>

namespace ClipBridge {

class ActionManager;

struct PendingItem {
    enum Type { Image, File };
    Type type;
    QImage image;
    QString filePath;
};

class InputPanel : public QWidget
{
    Q_OBJECT
public:
    explicit InputPanel(ActionManager *actionManager, QWidget *parent = nullptr);

    void updateConfig(const AppConfig &config);
    void showAndRefresh();

signals:
    void panelOpened();
    void panelClosed();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void onClipboardChanged();
    void onSendClicked();

private:
    void setupUI();
    void applyStyle();
    QString processContent();
    void clearPreview();
    void addPreviewItem(const PendingItem &item);

    ActionManager *m_actionManager;
    AppConfig m_config;

    QTextEdit *m_chatEdit;
    QCheckBox *m_pinCheck;
    QCheckBox *m_saveLongTextCheck;
    QPushButton *m_sendBtn;
    QPushButton *m_cancelBtn;

    // Preview area at top — vertical list of thin bars
    QScrollArea *m_previewArea;
    QWidget *m_previewContent;
    QHBoxLayout *m_previewLayout;

    // Pending items — accumulated across multiple pastes, supports mixed images+files
    QVector<PendingItem> m_pendingItems;

    bool m_pinned = false;
};

} // namespace ClipBridge
