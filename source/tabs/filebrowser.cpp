#include <algorithm>
#include <cstring>

#include "config.h"
#include "fs.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "reader.h"
#include "utils.h"
#include "windows.h"

int sort = 0;

namespace Tabs {
    static const ImVec2 texSize = ImVec2(22, 22);
    static const char *devices[] = { "os0:", "pd0:", "sa0:", "tm0:", "ud0:", "ur0:", "ux0:", "vd0:", "vs0:"};

    // Sort using ImGuiTableSortSpecs
    bool Sort(const SceIoDirent &entryA, const SceIoDirent &entryB) {
        bool descending = false;
        ImGuiTableSortSpecs *tableSortSpecs = ImGui::TableGetSortSpecs();
        
        for (int i = 0; i < tableSortSpecs->SpecsCount; ++i) {
            const ImGuiTableColumnSortSpecs *columnSortSpec = &tableSortSpecs->Specs[i];
            descending = (columnSortSpec->SortDirection == ImGuiSortDirection_Descending);

            // Make sure ".." stays at the top regardless of sort direction
            if (strcasecmp(entryA.d_name, "..") == 0) {
                return true;
            }
            
            if (strcasecmp(entryB.d_name, "..") == 0) {
                return false;
            }
            
            if ((SCE_S_ISDIR(entryA.d_stat.st_mode)) && !(SCE_S_ISDIR(entryB.d_stat.st_mode))) {
                return true;
            }
            else if (!(SCE_S_ISDIR(entryA.d_stat.st_mode)) && (SCE_S_ISDIR(entryB.d_stat.st_mode))) {
                return false;
            }
            else {
                switch (columnSortSpec->ColumnIndex) {
                    case 0: // filename
                        sort = descending? FS_SORT_ALPHA_DESC : FS_SORT_ALPHA_ASC;
                        return descending? (strcasecmp(entryB.d_name, entryA.d_name) < 0) : (strcasecmp(entryA.d_name, entryB.d_name) < 0);
                        break;
                        
                    case 1: // Size
                        sort = descending? FS_SORT_SIZE_DESC : FS_SORT_SIZE_ASC;
                        return descending? (entryB.d_stat.st_size < entryA.d_stat.st_size) : (entryA.d_stat.st_size < entryB.d_stat.st_size);
                        break;
                        
                    default:
                        break;
                }
            }
        }
        
        return false;
    }

    void FileBrowser(WindowData &data) {
        if (ImGui::BeginTabItem("File Browser")) {
            ImGui::PushID("device_list");
            ImGui::PushItemWidth(75.f);
            if (ImGui::BeginCombo("", cfg.device.c_str())) {
                for (int i = 0; i < IM_ARRAYSIZE(devices); i++) {
                    const bool is_selected = (cfg.device == devices[i]);
                    
                    if (ImGui::Selectable(devices[i], is_selected)) {
                        cfg.device = devices[i];
                        cfg.cwd = "/";
                        data.entries.clear();
                        const std::string path = cfg.device + cfg.cwd;
                        FS::GetDirList(path, data.entries);
                        sort = -1;
                    }
                        
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::PopID();
            
            ImGui::SameLine();

            // Display current working directory
            ImGui::TextColored(ImVec4(1.00f, 1.00f, 1.00f, 1.00f), cfg.cwd.c_str());

            ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;
            
            if (ImGui::BeginTable("Directory List", 2, tableFlags)) {
                // Make header always visible
                ImGui::TableSetupScrollFreeze(0, 1);

                ImGui::TableSetupColumn("Filename", ImGuiTableColumnFlags_DefaultSort);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 125.f);
                ImGui::TableHeadersRow();

                if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs()) {
                    if (sort == -1) {
                        sorts_specs->SpecsDirty = true;
                    }
                    
                    if (sorts_specs->SpecsDirty) {
                        std::sort(data.entries.begin(), data.entries.end(), Tabs::Sort);
                        sorts_specs->SpecsDirty = false;
                    }
                }

                for (SceOff i = 0; i < data.entries.size(); i++) {
                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    if (SCE_S_ISDIR(data.entries[i].d_stat.st_mode)) {
                        ImGui::Image(reinterpret_cast<ImTextureID>(icons[FOLDER].ptr), texSize);
                    }
                    else {
                        ImGui::Image(reinterpret_cast<ImTextureID>(icons[BOOK].ptr), texSize);
                    }
                    
                    ImGui::SameLine();

                    if (ImGui::Selectable(data.entries[i].d_name, false)) {
                        if (SCE_S_ISDIR(data.entries[i].d_stat.st_mode)) {
                            if (std::strncmp(data.entries[i].d_name, "..", 2) == 0) {
                                FS::ChangeDirPrev(data.entries);
                            }
                            else {
                                FS::ChangeDirNext(data.entries[i].d_name, data.entries);
                            }

                            // Reset navigation ID -- TODO: Scroll to top
                            ImGuiContext& g = *GImGui;
                            ImGui::SetNavID(ImGui::GetID(data.entries[0].d_name, 0), g.NavLayer, 0, ImRect());

                            // Reapply sort
                            ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs();
                            sorts_specs->SpecsDirty = true;
                        }
                        else {
                            std::string path = FS::BuildPath(data.entries[i]);

                            if (FS::IsBookType(data.entries[i].d_name)) {
                                Reader::OpenDocument(path, data.book);
                                data.state = WINDOW_STATE_BOOKVIEWER;
                            }
                        }
                    }

                    if (ImGui::IsItemHovered()) {
                        data.selected = i;
                    }

                    ImGui::TableNextColumn();
                    if ((data.entries[i].d_stat.st_size != 0) && (!SCE_S_ISDIR(data.entries[i].d_stat.st_mode))) {
                        char size[16];
                        Utils::GetSizeString(size, static_cast<double>(data.entries[i].d_stat.st_size));
                        ImGui::Text(size);
                    }
                }

                ImGui::EndTable();
            }

            ImGui::EndTabItem();
        }
    }
}
