#include <cmath>
#include <psp2/kernel/threadmgr.h>

#include "config.h"
#include "db.h"
#include "fs.h"
#include "imgui.h"
#include "popups.h"
#include "tabs.h"
#include "utils.h"
#include "windows.h"

namespace Windows {
    static bool isZooming = false;
    static float zoomReleaseTimer = 0.0f;
    const float ZOOM_RENDER_DELAY = 0.3f;

    void SetupWindow(void) {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(960.0f, 544.0f), ImGuiCond_Once);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    };
    
    void ExitWindow(void) {
        ImGui::End();
        ImGui::PopStyleVar();
    };

    static void ClearTextures(WindowData& data) {
        if (data.book.page != nullptr) {
            SDL_DestroyTexture(data.book.page);
            data.book.page = nullptr;
        }
    }

    static void SaveDBEntry(WindowData& data, int page) {
        std::string path = FS::BuildPath(data.entries[data.selected]);
        
        BookEntry entry = {
            path.c_str(),
            page,
            data.book.zoom,
            data.book.rotate
        };
        
        DB::Save(entry);
    }

    static bool NavigatePage(WindowData &data, int direction) {
        int newPage = data.book.pageNumber + direction;
        
        if (newPage < 0 || newPage >= data.book.pageCount) {
            data.book.pageCount = 0;
            data.book.pageNumber = 0;
            return false;
        }
        
        data.book.pageNumber = newPage;
        Reader::RenderPage(data.book, RENDER_NAV);
        Reader::ResetPosition(data.book);
        Windows::SaveDBEntry(data, newPage);
        data.resetScroll = true;
        return true;
    }
    
    void HandleAnalogInput(WindowData& data) {
        if (!data.gamepad) {
            return;
        }
        
        Sint16 axisValue = SDL_GetGamepadAxis(data.gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
        float value = axisValue / 32767.0f;
        const float deadzone = 0.2f;
        
        if (std::fabs(value) > deadzone) {
            isZooming = true;
            zoomReleaseTimer = ZOOM_RENDER_DELAY;
            
            float zoomSpeed = 0.5f * ImGui::GetIO().DeltaTime;
            float newZoom = data.book.zoom - value * zoomSpeed;
            float old_zoom = data.book.zoom;
            data.book.zoom = newZoom;
            
            Reader::UpdateZoom(old_zoom, newZoom);
            Reader::MovePage(data.book, 0.f, 0.f);
        }
        else if (isZooming) {
            zoomReleaseTimer -= ImGui::GetIO().DeltaTime;
            if (zoomReleaseTimer <= 0.0f) {
                isZooming = false;
                Reader::RenderPage(data.book, RENDER_ZOOM);
            }
        }
    }

    void HandleInput(WindowData& data, SDL_Event& event) {
        int button = event.gbutton.button;

        switch (data.state) {
            case WINDOW_STATE_BOOKVIEWER:
                if (button == SDL_GAMEPAD_BUTTON_WEST) {
                    Reader::ToggleOrientation(data.book);
                    Windows::SaveDBEntry(data, data.book.pageNumber);
                }
                else if (button == SDL_GAMEPAD_BUTTON_EAST) {
                    Windows::ClearTextures(data);
                    data.book.pageCount = 0;
                    data.book.pageNumber = 0;
                    data.zoom_factor = 1.0f;
                    data.state = WINDOW_STATE_FILEBROWSER;
                }
                else if (button == SDL_GAMEPAD_BUTTON_LEFT_SHOULDER) {
                    Windows::ClearTextures(data);
                    sceKernelDelayThread(100000);
                    
                    if (!Windows::NavigatePage(data, -1)) {
                        data.state = WINDOW_STATE_FILEBROWSER;
                    }
                }
                else if (button == SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER) {
                    Windows::ClearTextures(data);
                    sceKernelDelayThread(100000);
                    
                    if (!Windows::NavigatePage(data, 1)) {
                        data.state = WINDOW_STATE_FILEBROWSER;
                    }
                }
                break;
            
            default:
                break;
        }
    }

    void MainWindow(WindowData& data) {
        Windows::SetupWindow();
        if (ImGui::Begin("vitaPDF", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
            if (ImGui::BeginTabBar("vitaPDF-tabs")) {
                Tabs::FileBrowser(data);
                Tabs::Settings();
                ImGui::EndTabBar();
            }

            if (data.state == WINDOW_STATE_BOOKVIEWER) {
                Windows::Viewer(data);
            }
        }

        Windows::ExitWindow();
    }
}
