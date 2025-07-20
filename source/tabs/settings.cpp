#include <mupdf/fitz/version.h>
#include <SDL3/SDL.h>

#include "config.h"
#include "fs.h"
#include "gui.h"
#include "imgui.h"
#include "windows.h"

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

namespace Tabs {
    static std::string vitapdf_ver = APP_VERSION;

    void Settings(void) {
        if (ImGui::BeginTabItem("Settings")) {
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            ImGui::Indent(5.f);
            
            ImGui::TextColored(ImVec4(0.44f, 0.78f, 0.94f, 1.0f), "Book Viewer:");
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            ImGui::Indent(15.f);
            
            if (ImGui::Checkbox(" Display filename", &cfg.filename)) {
                Config::Save(cfg);
            }

            ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Spacing

            if (ImGui::Checkbox(" Dark theme", &cfg.darkTheme)) {
                if (cfg.darkTheme) {
                    GUI::DarkTheme();
                }
                else {
                    GUI::LightTheme();
                }
                Config::Save(cfg);
            }
            
            ImGui::Dummy(ImVec2(0.0f, 10.0f)); // Spacing
            ImGui::Unindent();
            ImGui::Separator();

            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Indent(5.f);
            ImGui::TextColored(ImVec4(0.44f, 0.78f, 0.94f, 1.0f), "About:");
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            ImGui::Indent(15.f);

            vitapdf_ver.erase(0, std::min(vitapdf_ver.find_first_not_of('0'), vitapdf_ver.size() - 1));
            ImGui::Text("vitaPDF version: %s", vitapdf_ver.c_str());
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            
            ImGui::Text("Author: Joel16");
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing

            ImGui::Text("LiveArea Assets: PreetiSketch");
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            
            ImGui::Text("Dear imGui version: %s", ImGui::GetVersion());
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing
            
            ImGui::Text("SDL version: %u.%u.%u", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
            ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Spacing

            ImGui::Text("MuPDF version: %s", FZ_VERSION);

            ImGui::Unindent();
            ImGui::EndTabItem();
        }
    }
}
