#pragma once

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

//过程函数
INT_PTR CALLBACK AddHotkeyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);