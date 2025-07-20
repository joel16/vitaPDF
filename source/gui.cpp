#include "config.h"
#include "fs.h"
#include "gui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "log.h"
#include "utils.h"
#include "windows.h"

namespace Renderer {
    static void Start(void) {
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }
    
    static void End(ImGuiIO &io, ImVec4 clear_color, SDL_Renderer *renderer) {
        ImGui::Render();
        SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }
}

namespace GUI {
    static SDL_Window *window;
    static SDL_Renderer *renderer;
    static const ImVec4 accent = ImVec4(0.44f, 0.76f, 0.94f, 1.0f);

    SDL_Renderer *GetRenderer(void) {
        return renderer;
    }

    SDL_Window *GetWindow(void) {
        return window;
    }

    void DarkTheme(void) {
        ImVec4 *colors = ImGui::GetStyle().Colors;

        // style->FrameRounding = 4.0f;
        // style->GrabRounding = 4.0f;

        colors[ImGuiCol_Text]                   = ImVec4(1, 1, 1, 1);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.15f, 0.15f, 0.15f, 0.95f);
        colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(accent.x, accent.y, accent.z, 0.30f);
        colors[ImGuiCol_FrameBgActive]          = accent;
        colors[ImGuiCol_TitleBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.10f, 0.10f, 0.10f, 0.75f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
        colors[ImGuiCol_CheckMark]              = accent;
        colors[ImGuiCol_SliderGrab]             = accent;
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(accent.x * 0.9f, accent.y * 0.9f, accent.z * 0.9f, 1.0f);
        colors[ImGuiCol_Button]                 = ImVec4(accent.x, accent.y, accent.z, 0.40f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_ButtonActive]           = accent;
        colors[ImGuiCol_Header]                 = ImVec4(accent.x, accent.y, accent.z, 0.30f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_HeaderActive]           = accent;
        colors[ImGuiCol_Separator]              = ImVec4(accent.x, accent.y, accent.z, 0.40f);
        colors[ImGuiCol_Tab]                    = ImVec4(accent.x, accent.y, accent.z, 0.30f);
        colors[ImGuiCol_TabHovered]             = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_TabSelected]              = accent;
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(accent.x, accent.y, accent.z, 0.2f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(accent.x, accent.y, accent.z, 0.35f);
        colors[ImGuiCol_NavCursor]              = accent;
        colors[ImGuiCol_DragDropTarget]         = ImVec4(accent.x, accent.y, accent.z, 0.95f);
    }

    void LightTheme(void) {
        ImVec4 *colors = ImGui::GetStyle().Colors;
        
        colors[ImGuiCol_Text]                  = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
        colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.30f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(accent.x, accent.y, accent.z, 0.3f);
        colors[ImGuiCol_FrameBgActive]         = accent;
        colors[ImGuiCol_TitleBg]               = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.95f, 0.95f, 0.95f, 0.75f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.75f, 0.75f, 0.75f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = accent;
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(accent.x * 0.8f, accent.y * 0.8f, accent.z * 0.8f, 1.00f);
        colors[ImGuiCol_CheckMark]             = accent;
        colors[ImGuiCol_SliderGrab]            = accent;
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(accent.x * 0.9f, accent.y * 0.9f, accent.z * 0.9f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(accent.x, accent.y, accent.z, 0.30f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(accent.x, accent.y, accent.z, 0.60f);
        colors[ImGuiCol_ButtonActive]          = accent;
        colors[ImGuiCol_Header]                = ImVec4(accent.x, accent.y, accent.z, 0.30f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(accent.x, accent.y, accent.z, 0.60f);
        colors[ImGuiCol_HeaderActive]          = accent;
        colors[ImGuiCol_Separator]             = ImVec4(accent.x, accent.y, accent.z, 0.40f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(accent.x, accent.y, accent.z, 0.75f);
        colors[ImGuiCol_SeparatorActive]       = accent;
        colors[ImGuiCol_ResizeGrip]            = ImVec4(accent.x, accent.y, accent.z, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(accent.x, accent.y, accent.z, 0.60f);
        colors[ImGuiCol_ResizeGripActive]      = accent;
        colors[ImGuiCol_Tab]                   = ImVec4(accent.x, accent.y, accent.z, 0.3f);
        colors[ImGuiCol_TabHovered]            = ImVec4(accent.x, accent.y, accent.z, 0.65f);
        colors[ImGuiCol_TabSelected]           = accent;
        colors[ImGuiCol_TabDimmed]             = ImVec4(0.80f, 0.80f, 0.80f, 0.60f);
        colors[ImGuiCol_TabDimmedSelected]     = ImVec4(accent.x, accent.y, accent.z, 0.40f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(accent.x, accent.y, accent.z, 0.15f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.80f, 0.80f, 0.85f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.95f, 0.95f, 0.95f, 0.60f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(accent.x, accent.y, accent.z, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(accent.x, accent.y, accent.z, 0.95f);
        colors[ImGuiCol_NavCursor]             = accent;
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.70f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    }

    int Init(void) {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
            Log::Error("SDL_Init failed: %s\n", SDL_GetError());
            return -1;
        }
        
        // Create window with SDL_Renderer graphics context
        window = SDL_CreateWindow("vitaPDF", 960, 544, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
        if (window == nullptr) {
            Log::Error("SDL_CreateWindow failed: %s\n", SDL_GetError());
        }

        renderer = SDL_CreateRenderer(window, nullptr);
        SDL_SetRenderVSync(renderer, 1);
        if (renderer == nullptr) {
            Log::Error("SDL_CreateRenderer failed: %s\n", SDL_GetError());
            return 0;
        }
        
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowWindow(window);
        
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.IniFilename = nullptr;
        
        // Setup Dear ImGui style
        //ImGui::StyleColorsDark();

        ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer3_Init(renderer);
        
        ImFontConfig font_config;
        font_config.OversampleH = 1;
        font_config.OversampleV = 1;
        font_config.PixelSnapH = 1;
        
        io.Fonts->AddFontFromFileTTF("sa0:/data/font/pvf/jpn0.pvf", 20.0f, std::addressof(font_config));

        cfg.darkTheme? GUI::DarkTheme() : GUI::LightTheme();
        return 0;
    }

    void Exit(void) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    int RenderLoop(void) {
        bool done = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        WindowData data;

        int ret = 0;
        const std::string path = cfg.device + cfg.cwd;

        if (R_FAILED(ret = FS::GetDirList(path, data.entries))) {
            return ret;
        }

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Get gamepad for right analog stick
        int count = 0;
        SDL_GetGamepads(std::addressof(count));
        if (count > 0) {
            SDL_Gamepad *gamepad = SDL_OpenGamepad(1);
            data.gamepad = gamepad;
        }

        while (!done) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);

                switch (event.type) {
                    case SDL_EVENT_QUIT:
                        done = true;
                        break;

                    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                        if (event.window.windowID == SDL_GetWindowID(window)) {
                            done = true;
                        }
                        break;
                        
                    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                        Windows::HandleInput(data, event);

                        if (event.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
                            done = true;
                        }
                        break;
                }
            }

            if (data.state == WINDOW_STATE_BOOKVIEWER) {
                Windows::HandleAnalogInput(data);
            }

            Renderer::Start();
            Windows::MainWindow(data);
            Renderer::End(io, clear_color, renderer);
        }
        
        if (data.gamepad) {
            SDL_CloseGamepad(data.gamepad);
        }
        
        data.entries.clear();
        return 0;
    }
}
