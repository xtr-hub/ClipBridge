// ClipBridge.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ClipBridge.h"
#include "core/action_manager.hpp"
#include "core/config_manager.hpp"
#include "core/app_config.hpp"
#include "core/register_manager.hpp"
#include "ui/settings_dialog.hpp"
#include <memory>
#include <string>

#define MAX_LOADSTRING 100
#define WM_TRAYICON (WM_USER + 1)

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HWND hMainWnd;                                 // 主窗口句柄
NOTIFYICONDATAW nid;                            // 托盘图标数据
ConfigManager* g_config_manager;               // 配置管理器指针
AppConfig* g_config;                         // 配置指针
RegisterManager* g_register_manager;             // 热键管理器指针
ActionManager* g_action_manager;                // 动作管理器指针

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                AddTrayIcon(HWND hWnd);
void                RemoveTrayIcon();
void                ShowTrayMenu(HWND hWnd);
void                ShowSettingsDialog(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 配置管理器
    ConfigManager config_manager;
    AppConfig config = config_manager.load();// 加载配置到内存中

    // 注册热键
    RegisterManager register_manager;
    register_manager.register_hotkeys(config.hotkeys);

    // 事件管理器
    ActionManager action_manager(config);

    // 设置全局指针
    g_config_manager = &config_manager;
    g_config = &config;
    g_register_manager = &register_manager;
    g_action_manager = &action_manager;

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLIPBRIDGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIPBRIDGE));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (msg.message == WM_HOTKEY)
        {
            AppConfig::Behavior behavior = register_manager.behavior_for_id(msg.wParam);
            for(std::string action : register_manager.action_for_id(msg.wParam))
                action_manager.run(action, behavior);// 跑起来
        }

        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    RemoveTrayIcon();
    register_manager.unregister_all();
    return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPBRIDGE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CLIPBRIDGE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    hMainWnd = hWnd;

    // 添加托盘图标
    AddTrayIcon(hWnd);

    // 隐藏主窗口
    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    return TRUE;
}

// 添加托盘图标
void AddTrayIcon(HWND hWnd)
{
    ZeroMemory(&nid, sizeof(NOTIFYICONDATAW));
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hWnd;
    nid.uID = IDI_TRAY;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_TRAY));
    if (!nid.hIcon)
    {
        nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CLIPBRIDGE));
    }
    wcscpy_s(nid.szTip, L"ClipBridge");
    Shell_NotifyIconW(NIM_ADD, &nid);
}

// 移除托盘图标
void RemoveTrayIcon()
{
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

// 显示托盘右键菜单
void ShowTrayMenu(HWND hWnd)
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS, L"设置(&S)");
    AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"关于(&A)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, IDM_EXIT, L"退出(&X)");

    SetForegroundWindow(hWnd);
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, nullptr);
    DestroyMenu(hMenu);
}

// 显示设置对话框
void ShowSettingsDialog(HWND hWnd)
{
    MessageBoxW(hWnd, L"托盘单击成功！设置对话框将在后续添加。", L"ClipBridge", MB_OK | MB_ICONINFORMATION);
    // DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGSDIALOG), hWnd, SettingsDlgProc);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_SETTINGS:
                ShowSettingsDialog(hWnd);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TRAYICON:
        {
            if (LOWORD(lParam) == WM_RBUTTONUP || LOWORD(lParam) == WM_CONTEXTMENU)
            {
                ShowTrayMenu(hWnd);
            }
            else if (LOWORD(lParam) == WM_LBUTTONUP || LOWORD(lParam) == WM_LBUTTONDBLCLK)
            {
                ShowSettingsDialog(hWnd);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// "关于"框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
