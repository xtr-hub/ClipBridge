#pragma once
#include <string>
#include <vector>
#include <memory>

class ClipboardManager
{
public:
    virtual ~ClipboardManager() = default;

    // 获取剪贴板文本 (统一 UTF-8 string)
    virtual std::string get_text() = 0;
    virtual void set_text(const std::string& text) = 0;

    // 图片处理
    virtual bool has_image() = 0;
    virtual std::vector<std::string> get_available_image_formats() = 0;
    virtual bool save_image_to_png(const std::string& file_path) = 0;

    // 模拟粘贴
    virtual void simulate_paste() = 0;

    // 工厂方法
    static std::unique_ptr<ClipboardManager> create();
};
