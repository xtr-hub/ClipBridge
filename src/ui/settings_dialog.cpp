/**
 * @file settings_dialog.cpp
 * @brief 设置对话框实现
 */

#include "ui/settings_dialog.hpp"
#include "Resource.h"
#include "core/action_manager.hpp"
#include "core/config_manager.hpp"
#include "core/register_manager.hpp"
#include <shlobj.h>
#include <objbase.h>
#include <string>

// UTF-16 转 UTF-8
static std::string WideToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};
    std::string result(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
}

// 更新路径输入框状态（根据模式选择启用/禁用）
void UpdatePathInputState(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_PATH_MODE_COMBO);
    int curSel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
    bool isCustom = (curSel == 1); // 0 = workspace, 1 = custom_path

    EnableWindow(GetDlgItem(hDlg, IDC_PATH_EDIT), isCustom);
    EnableWindow(GetDlgItem(hDlg, IDC_BROWSE_BUTTON), isCustom);
}

// 浏览文件夹
void BrowseForFolder(HWND hDlg)
{
    WCHAR path[MAX_PATH] = { 0 };

    // 先获取当前路径
    GetDlgItemTextW(hDlg, IDC_PATH_EDIT, path, MAX_PATH);

    BROWSEINFOW bi = { 0 };
    bi.hwndOwner = hDlg;
    bi.lpszTitle = L"选择图片保存目录";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl != NULL)
    {
        if (SHGetPathFromIDListW(pidl, path))
        {
            SetDlgItemTextW(hDlg, IDC_PATH_EDIT, path);
        }
        CoTaskMemFree(pidl);
    }
}

// "设置"框的消息处理程序。
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        {
            if (g_config)
            {
                // 初始化路径模式选择
                HWND hCombo = GetDlgItem(hDlg, IDC_PATH_MODE_COMBO);
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"工作区默认路径");
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"自定义路径");

                int modeIndex = (g_config->output.path.mode == AppConfig::Output::Path::Mode::workspace) ? 0 : 1;
                SendMessageW(hCombo, CB_SETCURSEL, modeIndex, 0);

                // 设置其他控件
                std::wstring formatWstr(g_config->output.format.begin(), g_config->output.format.end());
                SetDlgItemTextW(hDlg, IDC_FORMAT_EDIT, formatWstr.c_str());
                std::wstring pathWstr(g_config->output.path.dir.begin(), g_config->output.path.dir.end());
                SetDlgItemTextW(hDlg, IDC_PATH_EDIT, pathWstr.c_str());
                CheckDlgButton(hDlg, IDC_AUTOPASTE_CHECK, g_config->default_behavior.auto_paste ? BST_CHECKED : BST_UNCHECKED);
                CheckDlgButton(hDlg, IDC_AUTOSUBMIT_CHECK, g_config->default_behavior.auto_submit ? BST_CHECKED : BST_UNCHECKED);

                UpdatePathInputState(hDlg);
            }
            return (INT_PTR)TRUE;
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            if (wmId == IDC_SAVE_BUTTON)
            {
                if (g_config && g_config_manager && g_register_manager && g_action_manager)
                {
                    WCHAR buf[1024];
                    GetDlgItemTextW(hDlg, IDC_FORMAT_EDIT, buf, 1024);
                    g_config->output.format = WideToUTF8(buf);

                    // 获取路径模式
                    HWND hCombo = GetDlgItem(hDlg, IDC_PATH_MODE_COMBO);
                    int curSel = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
                    g_config->output.path.mode = (curSel == 0) ?
                        AppConfig::Output::Path::Mode::workspace :
                        AppConfig::Output::Path::Mode::custom_path;

                    GetDlgItemTextW(hDlg, IDC_PATH_EDIT, buf, 1024);
                    g_config->output.path.dir = WideToUTF8(buf);

                    g_config->default_behavior.auto_paste = (IsDlgButtonChecked(hDlg, IDC_AUTOPASTE_CHECK) == BST_CHECKED);
                    g_config->default_behavior.auto_submit = (IsDlgButtonChecked(hDlg, IDC_AUTOSUBMIT_CHECK) == BST_CHECKED);

                    // 保存配置
                    g_config_manager->save(*g_config);

                    // 热更新：重新注册热键并更新 ActionManager
                    g_register_manager->unregister_all();
                    g_register_manager->register_hotkeys(g_config->hotkeys);
                    g_action_manager->update_config(*g_config);

                    MessageBoxW(hDlg, L"设置已保存，已自动热更新！", L"提示", MB_OK | MB_ICONINFORMATION);
                }
                EndDialog(hDlg, IDOK);
                return (INT_PTR)TRUE;
            }
            else if (wmId == IDC_CANCEL_BUTTON)
            {
                EndDialog(hDlg, IDCANCEL);
                return (INT_PTR)TRUE;
            }
            else if (wmId == IDC_BROWSE_BUTTON)
            {
                BrowseForFolder(hDlg);
                return (INT_PTR)TRUE;
            }
            else if (wmId == IDC_PATH_MODE_COMBO && HIWORD(wParam) == CBN_SELCHANGE)
            {
                UpdatePathInputState(hDlg);
                return (INT_PTR)TRUE;
            }
            break;
        }
    }
    return (INT_PTR)FALSE;
}
