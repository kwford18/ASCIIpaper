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

        // Create a resizable window
        m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height, SDL_WINDOW_RESIZABLE);
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