#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 // Windows 7+
#endif

// clang-format off
#include <windows.h>
#include <shellapi.h>
// clang-format on

#include "platform/desktop.h"

#include <SDL3/SDL.h>
#include <bit>
#include <cwchar>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Menu Action IDs
#define ID_TRAY_EXIT 2000
#define ID_SCENE_AQUARIUM 2001
#define ID_SCENE_CITY 2002
#define ID_SYNC_ON 2003
#define ID_SYNC_OFF 2004
#define ID_WEATHER_CLEAR 2005
#define ID_WEATHER_RAIN 2006
#define ID_WEATHER_SNOW 2007
#define ID_WEATHER_STORM 2008

namespace ASCIIpaper::Platform {

namespace {
constexpr UINT WM_TRAYICON = WM_APP + 1;
constexpr UINT TRAY_UID = 1;
constexpr int HOTKEY_ID_QUIT = 1;

NOTIFYICONDATAW g_trayIcon = {};
bool g_trayIconAdded = false;
volatile bool g_shouldQuit = false;
HWND g_trayHwnd = nullptr;
bool g_configChanged = false;

// For CLI hot reloading
std::filesystem::file_time_type g_lastWriteTime;

/*
 * Helper to get the guaranteed absolute path to the config file.
 * This prevents the app from accidentally writing to a ghost config.ini
 * if the Working Directory gets messed up by the OS or build system.
 */
std::string GetConfigPath() {
    std::string path = "config.ini";
    const char* basePath = SDL_GetBasePath();
    if (basePath) {
        path = std::string(basePath) + "config.ini";
    }
    return path;
}

/*
 * A lightweight helper to extract a specific value from the config.
 * Used exclusively by the tray menu to determine where to place checkmarks
 * before the menu is drawn to the screen.
 */
std::string GetActiveSetting(const std::string& searchKey, const std::string& defaultVal) {
    std::ifstream inFile(GetConfigPath());
    if (inFile.is_open()) {
        // NOLINTNEXTLINE(bugprone-unused-local-non-trivial-variable)
        std::string line;

        while (std::getline(inFile, line)) {
            size_t eqPos = line.find('=');

            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                key.erase(key.find_last_not_of(" \t\r\n") + 1);
                key.erase(0, key.find_first_not_of(" \t\r\n"));

                if (key == searchKey) {
                    std::string val = line.substr(eqPos + 1);
                    val.erase(val.find_last_not_of(" \t\r\n") + 1);
                    val.erase(0, val.find_first_not_of(" \t\r\n"));
                    return val;
                }
            }
        }
    }
    return defaultVal;
}

void UpdateConfig(const std::string& key, const std::string& value) {
    std::vector<std::string> lines;
    std::string configPath = GetConfigPath();
    std::ifstream inFile(configPath);
    bool found = false;
    std::string line;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {
            // Ensure match the exact key and not a substring
            if (line.find(key) == 0 && (line[key.length()] == ' ' || line[key.length()] == '=')) {
                lines.push_back(std::format("{} = {}", key, value));
                found = true;
            } else {
                lines.push_back(line);
            }
        }
        inFile.close();
    }

    if (!found) {
        lines.push_back(std::format("{} = {}", key, value));
    }

    std::ofstream outFile(configPath);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
    outFile.close();

    g_configChanged = true;
}

LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAYICON: {
        UINT mouseMsg = LOWORD(lParam);
        if (mouseMsg == WM_RBUTTONUP || mouseMsg == WM_LBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);

            // Read the current settings to apply checkmarks
            std::string currentScene = GetActiveSetting("scene", "aquarium");
            std::string currentSync = GetActiveSetting("system_sync", "false");
            std::string currentWeather = GetActiveSetting("weather", "rain");

            HMENU hMenu = CreatePopupMenu();

            // Scene Menu
            HMENU hSceneMenu = CreatePopupMenu();
            AppendMenuW(hSceneMenu, MF_STRING, ID_SCENE_AQUARIUM, L"Aquarium");
            AppendMenuW(hSceneMenu, MF_STRING, ID_SCENE_CITY, L"City");

            // Apply checkmark to active scene
            if (currentScene == "city") {
                CheckMenuItem(hSceneMenu, ID_SCENE_CITY, MF_CHECKED);
            } else {
                CheckMenuItem(hSceneMenu, ID_SCENE_AQUARIUM, MF_CHECKED);
            }

            // Sync Menu
            HMENU hSyncMenu = CreatePopupMenu();
            AppendMenuW(hSyncMenu, MF_STRING, ID_SYNC_ON, L"On");
            AppendMenuW(hSyncMenu, MF_STRING, ID_SYNC_OFF, L"Off");

            // Apply checkmark to active sync state
            if (currentSync == "true" || currentSync == "1") {
                CheckMenuItem(hSyncMenu, ID_SYNC_ON, MF_CHECKED);
            } else {
                CheckMenuItem(hSyncMenu, ID_SYNC_OFF, MF_CHECKED);
            }

            // Weather Menu
            HMENU hWeatherMenu = CreatePopupMenu();
            AppendMenuW(hWeatherMenu, MF_STRING, ID_WEATHER_CLEAR, L"Clear");
            AppendMenuW(hWeatherMenu, MF_STRING, ID_WEATHER_RAIN, L"Rain");
            AppendMenuW(hWeatherMenu, MF_STRING, ID_WEATHER_SNOW, L"Snow");
            AppendMenuW(hWeatherMenu, MF_STRING, ID_WEATHER_STORM, L"Storm");

            // Apply checkmark to active weather
            if (currentWeather == "clear")
                CheckMenuItem(hWeatherMenu, ID_WEATHER_CLEAR, MF_CHECKED);
            else if (currentWeather == "snow")
                CheckMenuItem(hWeatherMenu, ID_WEATHER_SNOW, MF_CHECKED);
            else if (currentWeather == "storm")
                CheckMenuItem(hWeatherMenu, ID_WEATHER_STORM, MF_CHECKED);
            else
                CheckMenuItem(hWeatherMenu, ID_WEATHER_RAIN, MF_CHECKED);

            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSceneMenu, L"Scene");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hSyncMenu, L"System Sync");
            AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hWeatherMenu, L"Weather");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit ASCIIpaper");

            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        return 0;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_TRAY_EXIT:
            g_shouldQuit = true;
            break;
        case ID_SCENE_AQUARIUM:
            UpdateConfig("scene", "aquarium");
            break;
        case ID_SCENE_CITY:
            UpdateConfig("scene", "city");
            break;
        case ID_SYNC_ON:
            UpdateConfig("system_sync", "true");
            break;
        case ID_SYNC_OFF:
            UpdateConfig("system_sync", "false");
            break;
        case ID_WEATHER_CLEAR:
            UpdateConfig("weather", "clear");
            break;
        case ID_WEATHER_RAIN:
            UpdateConfig("weather", "rain");
            break;
        case ID_WEATHER_SNOW:
            UpdateConfig("weather", "snow");
            break;
        case ID_WEATHER_STORM:
            UpdateConfig("weather", "storm");
            break;
        default:
            break;
        }
        return 0;
    }
    case WM_HOTKEY: {
        if (wParam == HOTKEY_ID_QUIT) {
            g_shouldQuit = true;
            return 0;
        }
        break;
    }
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/*
 * The tray icon needs an owner window that can actually become the
 * foreground window so its popup menu behaves correctly (dismisses on
 * click-away and delivers WM_COMMAND for the clicked item). The
 * wallpaper window itself is WS_CHILD now, and WS_CHILD windows can
 * never become the foreground window - that's what was breaking the
 * menu. So a small, invisible, top-level window i created
 * just to own the tray icon and its menu.
 */
HWND CreateTrayWindow() {
    static const wchar_t* kClassName = L"WallAquariumTrayWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = kClassName;
    RegisterClassW(&wc);

    return CreateWindowExW(WS_EX_TOOLWINDOW, kClassName, L"", WS_POPUP, 0, 0, 0, 0, NULL, NULL,
                           wc.hInstance, NULL);
}
} // namespace

bool HasConfigChanged() {
    // Check if the user clicke the tray UI
    if (g_configChanged)
        return true;

    // Check if the config file has been modified via CLI/external editor
    std::error_code ec;
    auto currentWriteTime = std::filesystem::last_write_time(GetConfigPath(), ec);
    if (!ec && currentWriteTime > g_lastWriteTime) {
        g_lastWriteTime = currentWriteTime;
        g_configChanged = true;
    }

    return g_configChanged;
}

bool ShouldQuit() {
    return g_shouldQuit;
}
void ClearConfigChanged() {
    g_configChanged = false;
}

// A callback used to search through active Windows to find the one rendering desktop icon
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    HWND p = FindWindowExW(hwnd, NULL, L"SHELLDLL_DefView", NULL);
    if (p != NULL) {
        HWND* workerW = std::bit_cast<HWND*>(lParam);
        *workerW = FindWindowExW(NULL, hwnd, L"WorkerW", NULL);
    }
    return TRUE;
}

/*
 * DEBUG: walk WorkerW/Progman's children and print their class names
 * to see what specific Windows builds actually create, instead
 * of guessing blindly
 */
BOOL CALLBACK DumpChildProc(HWND hwnd, LPARAM) {
    wchar_t className[256] = {0};
    GetClassNameW(hwnd, className, 256);
    std::wcerr << L"    child hwnd=" << hwnd << L" class=" << className << '\n';
    return TRUE;
}

BOOL CALLBACK DumpTopLevelProc(HWND hwnd, LPARAM) {
    wchar_t className[256] = {0};
    GetClassNameW(hwnd, className, 256);
    if (wcscmp(className, L"WorkerW") == 0 || wcscmp(className, L"Progman") == 0) {
        std::wcerr << L"top-level hwnd=" << hwnd << L" class=" << className << '\n';
        EnumChildWindows(hwnd, DumpChildProc, 0);
    }
    return TRUE;
}

void DumpDesktopHierarchy() {
    std::cerr << "---- Desktop window hierarchy dump ----" << '\n';
    EnumWindows(DumpTopLevelProc, 0);
    std::cerr << "---- end dump ----" << '\n';
}

/*
 * Some Windows builds put the real background WorkerW as a direct child of
 * Progman alongside SHELLDLL_DefView instead of as a top-level sibling.
 * Check both shapes rather than assuming one.
 */
HWND FindTargetWorkerW(HWND progman) {
    // Shape 1: a WorkerW appears as a top-level window immediately
    // after Progman in z-order, once Progman is asked to spawn one.
    HWND workerW = NULL;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&workerW));
    if (workerW != NULL) {
        return workerW;
    }

    // Shape 2: the background WorkerW is nested
    // directly inside Progman, as a sibling of SHELLDLL_DefView.
    HWND nested = FindWindowExW(progman, NULL, L"WorkerW", NULL);
    if (nested != NULL) {
        return nested;
    }

    return NULL;
}

void AttachToDesktop(SDL_Window* window) {
    // Ask SDL3 to give the OS-level Windows Handle (HWND) for window
    HWND sdlHwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
                                                SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

    if (!sdlHwnd) {
        std::cerr << "Failed to get native HWND from SDL_Window." << '\n';
        return;
    }

    // Find the Program Manager
    HWND progman = FindWindowW(L"Progman", NULL);
    if (!progman) {
        std::cerr << "Could not find Progman. GetLastError=" << GetLastError() << '\n';
        return;
    }

    /*
     * Send an undocumented message (0x052C) to Progman.
     * This forces Windows to spawn a special "WorkerW" window between the wallpaper and desktop
     * icons. On some builds this needs to be sent twice, and on some builds it isn't needed at all
     * because the WorkerW already exists nested under Progman so it's retried a few times and
     * checks both possible shapes each time.
     */
    HWND workerW = NULL;
    for (int attempt = 0; attempt < 5 && workerW == NULL; ++attempt) {
        DWORD_PTR result = 0;
        SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, &result);

        workerW = FindTargetWorkerW(progman);
        if (workerW == NULL)
            Sleep(50);
    }

    /*
     * DEBUG: If a sibling WorkerW is found, DO NOT blindly reparent to
     * Progman. On many current Windows 11 builds Progman itself sits below
     * the wallpaper layer, so a window parented there is completely hidden.
     * This dumps the actual hierarchy so we can see what shape the desktop is
     * in and which window to attach to.
     */
    if (workerW == NULL) {
        std::cerr << "No sibling WorkerW found. Leaving window as a normal "
                     "visible window instead of hiding it behind Progman."
                  << '\n';
        DumpDesktopHierarchy();
        return;
    }

    /*
     * Just SetParent does not make this a proper child window.
     * The SDL window was created as a top-level WS_POPUP window, and
     * Windows keeps treating it as an independent top-level window unless
     * it is explicitly flipped to WS_CHILD style first.
     * Without this, the window doesn't composite into WorkerW's client area.
     */
    LONG_PTR style = GetWindowLongPtrW(sdlHwnd, GWL_STYLE);
    style &= ~WS_POPUP;
    style |= WS_CHILD;
    SetWindowLongPtrW(sdlHwnd, GWL_STYLE, style);

    // Attach the SDL window directly to the desktop background
    if (!SetParent(sdlHwnd, workerW)) {
        std::cerr << "SetParent failed. GetLastError=" << GetLastError() << '\n';
        return;
    }

    /*
     * Once reparented, x/y become relative to the new parent's client
     * area rather than the screen, so explicitly place it at the parent's
     * origin instead of leaving whatever screen coordinates it had before.
     */
    RECT parentRect;
    GetClientRect(workerW, &parentRect);
    SetWindowPos(sdlHwnd, HWND_BOTTOM, 0, 0, parentRect.right - parentRect.left,
                 parentRect.bottom - parentRect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);
    ShowWindow(sdlHwnd, SW_SHOW);

    /*
     * Mark this as a tool window (keeps it out of taskbar/alt-tab) and
     * mouse-transparent (clicks fall through to whatever's below in
     * z-order).
     *
     * NOTE: Deliberately did not add WS_EX_LAYERED here.
     * A layered window that never gets SetLayeredWindowAttributes or
     * UpdateLayeredWindow called on it renders as fully invisible, which
     * caused the window to disappear after reparenting.
     */
    LONG exStyle = GetWindowLongW(sdlHwnd, GWL_EXSTYLE);
    SetWindowLongW(sdlHwnd, GWL_EXSTYLE, exStyle | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
    SetWindowPos(sdlHwnd, NULL, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOACTIVATE);

    /*
     * Set up a way to actually stop the program: a system tray icon with
     * an Exit option, plus a Ctrl+Alt+Q global hotkey as a backup. Since
     * this window has no border, no taskbar entry, and is click-through,
     * there'd otherwise be no way to close it short of Task Manager.
     */
    g_trayHwnd = CreateTrayWindow();
    if (!g_trayHwnd) {
        std::cerr << "Failed to create tray helper window. GetLastError=" << GetLastError() << '\n';
    }

    g_trayIcon.cbSize = sizeof(NOTIFYICONDATAW);
    g_trayIcon.hWnd = g_trayHwnd; // must be the helper window, not the WS_CHILD wallpaper window
    g_trayIcon.uID = TRAY_UID;
    g_trayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_trayIcon.uCallbackMessage = WM_TRAYICON;

    g_trayIcon.hIcon = ExtractIconW(GetModuleHandle(NULL), L"ASCIIpaper.exe", 0);

    /*
     * Use MAKEINTRESOURCEW with the raw resource ID (32512 = IDI_APPLICATION)
     * instead of the IDI_APPLICATION macro directly: that macro expands to
     * the ANSI (LPSTR) form unless the project defines UNICODE/_UNICODE,
     * which doesn't match LoadIconW's LPCWSTR parameter.
     */
    if (g_trayIcon.hIcon == NULL) {
        g_trayIcon.hIcon = LoadIconW(NULL, MAKEINTRESOURCEW(32512));
    }

    wcsncpy_s(g_trayIcon.szTip, L"ASCIIpaper Settings", _TRUNCATE);

    /*
     * Defensively remove any stale icon left behind by a previous run that
     * crashed or got killed before ShutdownDesktopIntegration() could run.
     * A leftover registration can otherwise make the new NIM_ADD misbehave.
     */
    Shell_NotifyIconW(NIM_DELETE, &g_trayIcon);

    g_trayIconAdded = Shell_NotifyIconW(NIM_ADD, &g_trayIcon);
    if (!g_trayIconAdded) {
        std::cerr << "Failed to create tray icon. GetLastError=" << GetLastError() << '\n';
    } else {
        // Opt into modern notification-icon behavior (correct popup menu
        // positioning, etc.) instead of legacy pre-Vista behavior.
        g_trayIcon.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &g_trayIcon);
    }

    if (!RegisterHotKey(g_trayHwnd, HOTKEY_ID_QUIT, MOD_CONTROL | MOD_ALT, 'Q')) {
        std::cerr << "Failed to register Ctrl+Alt+Q hotkey. GetLastError=" << GetLastError()
                  << '\n';
    }

    // For CLI hot reloading, store the last write time of the config file at startup
    std::error_code ec;
    g_lastWriteTime = std::filesystem::last_write_time(GetConfigPath(), ec);
}

void ShutdownDesktopIntegration() {
    if (g_trayIconAdded) {
        Shell_NotifyIconW(NIM_DELETE, &g_trayIcon);
        g_trayIconAdded = false;
    }
    UnregisterHotKey(g_trayHwnd, HOTKEY_ID_QUIT);
    if (g_trayHwnd) {
        DestroyWindow(g_trayHwnd);
        g_trayHwnd = nullptr;
    }
}

} // namespace ASCIIpaper::Platform