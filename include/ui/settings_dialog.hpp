#pragma once

/**
 * @file settings_dialog.hpp
 * @brief 设置对话框处理
 */

#include "core/app_config.hpp"
#include <Windows.h>

// 全局指针声明
class ActionManager;
class ConfigManager;
class RegisterManager;

extern ConfigManager* g_config_manager;
extern AppConfig* g_config;
extern RegisterManager* g_register_manager;
extern ActionManager* g_action_manager;

// 设置对话框处理函数
INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// 辅助函数
void UpdatePathInputState(HWND hDlg);
void BrowseForFolder(HWND hDlg);
