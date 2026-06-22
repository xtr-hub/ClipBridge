#pragma once

#include <optional>
#include <string>
#include <vector>

struct AppConfig
{
    enum class System
    {
        windows,
        linux,
        macos
    };

    static std::string to_string(System system)
    {
        switch (system)
        {
        case System::windows:
            return "windows";
        case System::linux:
            return "linux";
        case System::macos:
            return "macos";
        }
        return "windows";
    }

    static std::optional<System> system_from_string(const std::string& value)
    {
        if (value == "windows") return System::windows;
        if (value == "linux") return System::linux;
        if (value == "macos") return System::macos;
        return std::nullopt;
    }

    struct Behavior
    {
        bool auto_paste = true;
        bool auto_submit = false;
    };

    struct HotKeyBinding
    {
        std::string action = "clipboard_image_path";
        std::vector<std::string> keys = { "ctrl", "alt", "i" };
    };

    struct Output
    {
        struct Path
        {
            enum class Mode
            {
                custom_path,
                workspace
            };

            static std::string to_string(Mode mode)
            {
                switch (mode)
                {
                case Mode::custom_path:
                    return "custom_path";
                case Mode::workspace:
                    return "workspace";
                }
                return "workspace";
            }

            static std::optional<Mode> mode_from_string(const std::string& value)
            {
                if (value == "custom_path") return Mode::custom_path;
                if (value == "workspace") return Mode::workspace;
                return std::nullopt;
            }

            Mode mode = Mode::workspace;
            std::string dir;
        };

        enum class Mode
        {
            path,
            description,
            both
        };

        static std::string to_string(Mode mode)
        {
            switch (mode)
            {
            case Mode::path:
                return "path";
            case Mode::description:
                return "description";
            case Mode::both:
                return "both";
            }
            return "path";
        }

        static std::optional<Mode> mode_from_string(const std::string& value)
        {
            if (value == "path") return Mode::path;
            if (value == "description") return Mode::description;
            if (value == "both") return Mode::both;
            return std::nullopt;
        }

        Mode mode = Mode::path;
        std::string format = "{path}";
        Path path;
    };

    std::vector<HotKeyBinding> hotkeys = { HotKeyBinding{} };
    Behavior behavior;
    System system = System::windows;
    Output output;
};
