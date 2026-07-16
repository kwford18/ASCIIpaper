#include "engine/Window.h"
#include <iostream>

namespace Aquarium::Engine {

    Window::Window(const std::string& title, int width, int height)
        : m_title(title), m_width(width), m_height(height), m_shouldClose(false), m_window(nullptr) {
    }

    Window::~Window() {
        if (m_window) {
            SDL_DestroyWindow(m_window);
        }
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    bool Window::Initialize() {
        // Initialize SDL3 Video Subsystem
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            return false;
        }

        /* 
         * This window is meant to cover the whole desktop background, so size
         * it to the primary display's actual resolution rather than whatever
         * fixed size was passed to the constructor. This keeps it in sync
         * with the WorkerW area AttachToDesktop later reparents it into,
         * instead of relying on a hardcoded guess like 800x600.
        */
        SDL_DisplayID display = SDL_GetPrimaryDisplay();
        SDL_Rect displayBounds;
        if (display != 0 && SDL_GetDisplayBounds(display, &displayBounds)) {
            m_width = displayBounds.w;
            m_height = displayBounds.h;
        } else {
            std::cerr << "SDL_GetDisplayBounds failed: " << SDL_GetError()
                      << ". Falling back to the size passed to the constructor." << std::endl;
        }

        /* 
         * Create a borderless window. It deliberately does NOT use
         * SDL_WINDOW_TRANSPARENT. That puts the window in DWM's per-pixel-alpha
         * composited mode, which is designed for top-level windows and doesn't
         * reliably keep presenting once we reparent this as a WS_CHILD window
         * under the desktop's WorkerW
        */
        m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height, 
                                    SDL_WINDOW_BORDERLESS);
        if (!m_window) {
            std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
            return false;
        }

        return true;
    }

    void Window::PollEvents() {
        SDL_Event event;
        // Process all pending events in the queue
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                m_shouldClose = true;
            }
        }
    }

    bool Window::ShouldClose() const { return m_shouldClose; }
    int Window::GetWidth() const { return m_width; }
    int Window::GetHeight() const { return m_height; }
    SDL_Window* Window::GetNativeWindow() const { return m_window; }

} // namespace Aquarium::Engine