#include "platform/desktop.h"
#include <iostream>
#include <cwchar>

// Include the Windows API
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Aquarium::Platform {

    // A callback used to search through active Windows to find the one rendering desktop icons
    BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        HWND p = FindWindowExW(hwnd, NULL, L"SHELLDLL_DefView", NULL);
        if (p != NULL) {
            HWND* workerW = reinterpret_cast<HWND*>(lParam);
            // Change FindWindowEx to FindWindowExW
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
        std::wcerr << L"    child hwnd=" << hwnd << L" class=" << className << std::endl;
        return TRUE;
    }

    BOOL CALLBACK DumpTopLevelProc(HWND hwnd, LPARAM) {
        wchar_t className[256] = {0};
        GetClassNameW(hwnd, className, 256);
        if (wcscmp(className, L"WorkerW") == 0 || wcscmp(className, L"Progman") == 0) {
            std::wcerr << L"top-level hwnd=" << hwnd << L" class=" << className << std::endl;
            EnumChildWindows(hwnd, DumpChildProc, 0);
        }
        return TRUE;
    }

    void DumpDesktopHierarchy() {
        std::cerr << "---- Desktop window hierarchy dump ----" << std::endl;
        EnumWindows(DumpTopLevelProc, 0);
        std::cerr << "---- end dump ----" << std::endl;
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
        HWND sdlHwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

        if (!sdlHwnd) {
            std::cerr << "Failed to get native HWND from SDL_Window." << std::endl;
            return;
        }

        // Find the Program Manager
        HWND progman = FindWindowW(L"Progman", NULL);
        if (!progman) {
            std::cerr << "Could not find Progman. GetLastError=" << GetLastError() << std::endl;
            return;
        }

        /* 
         * Send an undocumented message (0x052C) to Progman.
         * This forces Windows to spawn a special "WorkerW" window between the wallpaper and desktop icons.
         * On some builds this needs to be sent twice, and on some builds it isn't
         * needed at all because the WorkerW already exists nested under Progman
         * so it's retried a few times and checks both possible shapes each time.
        */
        HWND workerW = NULL;
        for (int attempt = 0; attempt < 5 && workerW == NULL; ++attempt) {
            DWORD_PTR result = 0;
            SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, &result);

            workerW = FindTargetWorkerW(progman);

            if (workerW == NULL) {
                Sleep(50); // give the shell a moment to actually create/update it
            }
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
                         "visible window instead of hiding it behind Progman." << std::endl;
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
            std::cerr << "SetParent failed. GetLastError=" << GetLastError() << std::endl;
            return;
        }

        /* 
         * Once reparented, x/y become relative to the new parent's client
         * area rather than the screen, so explicitly place it at the parent's
         * origin instead of leaving whatever screen coordinates it had before.
        */
        RECT parentRect;
        GetClientRect(workerW, &parentRect);
        SetWindowPos(sdlHwnd, HWND_BOTTOM, 0, 0,
                     parentRect.right - parentRect.left,
                     parentRect.bottom - parentRect.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
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
    }

} // namespace Aquarium::Platform