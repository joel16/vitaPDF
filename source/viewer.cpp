#include <psp2/kernel/threadmgr.h>

#include "config.h"
#include "gui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "windows.h"

namespace Windows {
    void DrawPageCount(const std::string& text) {
        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        const float margin = 25.0f;
        const float padding = 8.0f;
        
        ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        ImVec2 textPos(960.0f - textSize.x - margin, 544.0f - textSize.y - margin);
        
        ImVec2 bgMin(textPos.x - padding, textPos.y - padding);
        ImVec2 bgMax(textPos.x + textSize.x + padding, textPos.y + textSize.y + padding);
        
        drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 100), 6.0f);
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), text.c_str());
    }

    void Viewer(WindowData &data) {
        Windows::SetupWindow();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGuiWindowFlags_ fileNameFlag = !cfg.filename ? ImGuiWindowFlags_NoTitleBar : ImGuiWindowFlags_None;
        
        const char* window_title = data.entries[data.selected].d_name;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | fileNameFlag;
            
        if (ImGui::Begin(window_title, nullptr, window_flags)) {
            if (data.resetScroll) {
                ImGui::SetScrollX(0.0f);
                ImGui::SetScrollY(0.0f);
                data.resetScroll = false;
            }

            ImVec2 image_size = ImVec2(data.book.width * data.book.zoom, data.book.height * data.book.zoom);
            ImVec2 region = ImGui::GetContentRegionAvail();
            
            if (image_size.x <= 960.0f && image_size.y <= 544.0f) {
                ImGui::SetCursorPos((region - image_size) * 0.5f);
            }

            ImGui::Image(reinterpret_cast<ImTextureID>(data.book.page), image_size);
            
            char buffer[16];
            std::snprintf(buffer, sizeof(buffer), "%d / %d", data.book.pageNumber + 1, data.book.pageCount);
            Windows::DrawPageCount(buffer);
        }
        
        Windows::ExitWindow();
        ImGui::PopStyleVar();
    }
}
