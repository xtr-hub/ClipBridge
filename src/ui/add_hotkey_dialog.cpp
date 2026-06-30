#include <shlobj.h>
#include <objbase.h>
#include <string>
#include <vector>
#include "ui/add_hotkey_dialog.hpp"
#include "Resource.h"
#include "core/string_utils.hpp"
#include "core/hotkey_keys.hpp"
#include "core/action_manager.hpp"
#include "core/config_manager.hpp"
#include "core/register_manager.hpp"

using string_utils::utf8_to_wide;
using string_utils::wide_to_utf8;
using string_utils::split_string;
using string_utils::to_lower;

static const std::vector<std::string> actions = {"clipboard_image_path", "strip_newlines"};

// 寻找这个动作对应的设置
AppConfig::HotKeyBinding* find_hotkey_from_action(const std::string& action)
{
    if(g_config)
    {
        for(auto& hotkeyBinding : g_config->hotkeys)
        {
            if(hotkeyBinding.action == action)
            {
                return &hotkeyBinding;
            }
        }
        return nullptr;
    }
    return nullptr;
}

// 检查键是否符合要求
bool check_keys(const std::vector<std::string> keys)
{
    UINT vk = 0;
    for (std::string key : keys)
    {
        key = to_lower(key);
        if (keys_map.find(key) == keys_map.end()) // 普通键
        {
            if (!vk)
            {
                if (key.size() > 1)
                {
                    return false;
                }
                vk = static_cast<UINT>(std::toupper(static_cast<unsigned char>(key[0])));
            }
            else
            {
                return false;
            }
            continue;
        }
    }

    if (!vk)
    {
        return false;
    }
    return true;
}

INT_PTR CALLBACK AddHotkeyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        {
             if (g_config)
            {
                // 初始化动作选择
                HWND hCombo = GetDlgItem(hDlg, IDC_ADD_HOTKEY_ACTION_COMBO);
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"clipboard_image_path");
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"strip_newlines");

                SendMessageW(hCombo, CB_SETCURSEL, 0, 0);

                AppConfig::HotKeyBinding* hotkeyBinding = find_hotkey_from_action("clipboard_image_path");
                
                if(hotkeyBinding)
                {
                    std::string keys_str = "";
                    for(const auto& key_str : hotkeyBinding->keys)
                    {
                        if(!keys_str.empty())
                        keys_str+="+";
                        keys_str += key_str;
                    }

                    SetDlgItemTextW(hDlg, IDC_ADD_HOTKEY_KEYS_EDIT, utf8_to_wide(keys_str).c_str());
                    CheckDlgButton(hDlg, IDC_ADD_HOTKEY_AUTOPASTE_CHECK, hotkeyBinding->behavior.auto_paste ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_ADD_HOTKEY_AUTOSUBMIT_CHECK, hotkeyBinding->behavior.auto_submit ? BST_CHECKED : BST_UNCHECKED);
                }

            }
            return (INT_PTR)TRUE;
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            if(wmId == IDOK)
            {
                // 获取动作
                HWND hCombo = GetDlgItem(hDlg, IDC_ADD_HOTKEY_ACTION_COMBO);
                int curSel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                std::string action = actions.at(curSel);
                
                // 获取HotKeyBinding并修改
                AppConfig::HotKeyBinding* hotkeyBinding = find_hotkey_from_action(action);
                if(!hotkeyBinding) return (INT_PTR)TRUE;
                hotkeyBinding->behavior.auto_paste = (IsDlgButtonChecked(hDlg, IDC_ADD_HOTKEY_AUTOPASTE_CHECK) == BST_CHECKED);
                hotkeyBinding->behavior.auto_submit = (IsDlgButtonChecked(hDlg, IDC_ADD_HOTKEY_AUTOSUBMIT_CHECK) == BST_CHECKED);
                WCHAR buf[1024];
                GetDlgItemTextW(hDlg, IDC_ADD_HOTKEY_KEYS_EDIT, buf, 1024);
                std::string keys_str = wide_to_utf8(buf);
                
                std::vector<std::string> keys = split_string(keys_str, '+');
                
                if(!check_keys(keys)){
                    MessageBoxW(
                        hDlg,
                        L"请输入正确的键格式！\n"
                        L"例如：ctrl+alt+i 或 shift+f1",
                        L"输入错误",
                        MB_ICONERROR | MB_OK
                    );
                    return (INT_PTR)TRUE;
                }
                hotkeyBinding->keys = keys;

                // 保存配置
                g_config_manager->save(*g_config);

                // 热更新：重新注册热键并更新 ActionManager
                g_register_manager->unregister_all();
                g_register_manager->register_hotkeys(g_config->hotkeys);
                g_action_manager->update_config(*g_config);

                EndDialog(hDlg, IDOK);
                return (INT_PTR)TRUE;
            }

            else if(wmId == IDCANCEL)
            {
                EndDialog(hDlg, IDCANCEL);
                return (INT_PTR)TRUE;
            }

            else if(wmId == IDC_ADD_HOTKEY_ACTION_COMBO)
            {
                // 获取动作
                HWND hCombo = GetDlgItem(hDlg, IDC_ADD_HOTKEY_ACTION_COMBO);
                int curSel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                std::string action = actions.at(curSel);

                AppConfig::HotKeyBinding* hotkeyBinding = find_hotkey_from_action(action);
                
                if(hotkeyBinding)
                {
                    std::string keys_str = "";
                    for(const auto& key_str : hotkeyBinding->keys)
                    {
                        if(!keys_str.empty())
                        keys_str+="+";
                        keys_str += key_str;
                    }

                    SetDlgItemTextW(hDlg, IDC_ADD_HOTKEY_KEYS_EDIT, utf8_to_wide(keys_str).c_str());
                    CheckDlgButton(hDlg, IDC_ADD_HOTKEY_AUTOPASTE_CHECK, hotkeyBinding->behavior.auto_paste ? BST_CHECKED : BST_UNCHECKED);
                    CheckDlgButton(hDlg, IDC_ADD_HOTKEY_AUTOSUBMIT_CHECK, hotkeyBinding->behavior.auto_submit ? BST_CHECKED : BST_UNCHECKED);
                }
                return (INT_PTR)TRUE;
            }
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return (INT_PTR)TRUE;
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE)
        {
            EndDialog(hDlg, IDCANCEL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}