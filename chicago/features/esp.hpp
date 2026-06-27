#pragma once
#include <windows.h>
#include <cmath>
#include "../deps/imgui/imgui.h"
#include "../rbx/cache.hpp"
#include "aimbot.hpp"

namespace esp {

inline bool enabled = true;
inline bool box = true;
inline bool corner = false;
inline bool name = true;
inline bool distance = false;
inline bool localplayer = false;
inline float box_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
inline float name_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
inline void text_outline(ImDrawList* dl, float x, float y, ImU32 col, const char* text) {
    ImU32 black = IM_COL32(0, 0, 0, 255);
    dl->AddText(ImVec2(x - 1, y - 1), black, text);
    dl->AddText(ImVec2(x + 1, y - 1), black, text);
    dl->AddText(ImVec2(x - 1, y + 1), black, text);
    dl->AddText(ImVec2(x + 1, y + 1), black, text);
    dl->AddText(ImVec2(x, y), col, text);
}

inline void draw_boxes(HANDLE proc, player_cache& pcache, vec2 dims, matrix4 view) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImU32 black = IM_COL32(0, 0, 0, 255);

    pcache.refresh();
    auto list = pcache.list;

    for (size_t i = 0; i < list.size(); i++) {
        if (!localplayer && pcache.is_local(i)) continue;

        auto addr = list[i].address;
        float left, top, right, bottom;
        if (!compute_bounds(proc, addr, pcache.ioffs, pcache.eoffs, dims, view, left, top, right, bottom))
            continue;

        ImU32 box_col = IM_COL32(
            (int)(box_color[0] * 255),
            (int)(box_color[1] * 255),
            (int)(box_color[2] * 255),
            (int)(box_color[3] * 255)
        );
        ImU32 name_col = IM_COL32(
            (int)(name_color[0] * 255),
            (int)(name_color[1] * 255),
            (int)(name_color[2] * 255),
            (int)(name_color[3] * 255)
        );

        if (box) {
            if (corner) {
                float len = (right - left) * 0.2f;
                float t = 1.5f;
                dl->AddLine(ImVec2(left, top + len), ImVec2(left, top), black, t + 2);
                dl->AddLine(ImVec2(left + len, top), ImVec2(left, top), black, t + 2);
                dl->AddLine(ImVec2(right - len, top), ImVec2(right, top), black, t + 2);
                dl->AddLine(ImVec2(right, top + len), ImVec2(right, top), black, t + 2);
                dl->AddLine(ImVec2(left, bottom), ImVec2(left, bottom - len), black, t + 2);
                dl->AddLine(ImVec2(left + len, bottom), ImVec2(left, bottom), black, t + 2);
                dl->AddLine(ImVec2(right, bottom), ImVec2(right, bottom - len), black, t + 2);
                dl->AddLine(ImVec2(right - len, bottom), ImVec2(right, bottom), black, t + 2);
                dl->AddLine(ImVec2(left, top + len), ImVec2(left, top), box_col, t);
                dl->AddLine(ImVec2(left + len, top), ImVec2(left, top), box_col, t);
                dl->AddLine(ImVec2(right - len, top), ImVec2(right, top), box_col, t);
                dl->AddLine(ImVec2(right, top + len), ImVec2(right, top), box_col, t);
                dl->AddLine(ImVec2(left, bottom), ImVec2(left, bottom - len), box_col, t);
                dl->AddLine(ImVec2(left + len, bottom), ImVec2(left, bottom), box_col, t);
                dl->AddLine(ImVec2(right, bottom), ImVec2(right, bottom - len), box_col, t);
                dl->AddLine(ImVec2(right - len, bottom), ImVec2(right, bottom), box_col, t);
            } else {
                dl->AddRect(ImVec2(left - 1, top - 1), ImVec2(right + 1, bottom + 1), black);
                dl->AddRect(ImVec2(left, top), ImVec2(right, bottom), box_col);
                dl->AddRect(ImVec2(left + 1, top + 1), ImVec2(right - 1, bottom - 1), black);
            }
        }

        if (name) {
            auto& pname = list[i].name;
            if (!pname.empty()) {
                float cx = (left + right) * 0.5f;
                float tw = ImGui::CalcTextSize(pname.c_str()).x;
                text_outline(dl, cx - tw * 0.5f, top - 14, name_col, pname.c_str());
            }
        }

        if (distance) {
            uint64_t model = 0, root = 0, prim = 0;
            vec3 pos = {};
            ReadProcessMemory(proc, (LPCVOID)(addr + pcache.eoffs.ModelInstance), &model, 8, nullptr);
            if (model) {
                ReadProcessMemory(proc, (LPCVOID)(model + pcache.eoffs.PrimaryPart), &root, 8, nullptr);
                if (root) {
                    ReadProcessMemory(proc, (LPCVOID)(root + pcache.eoffs.BasePartPrimitive), &prim, 8, nullptr);
                    if (prim) ReadProcessMemory(proc, (LPCVOID)(prim + pcache.eoffs.Position), &pos, 12, nullptr);
                }
            }
            float dist = sqrtf(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z) * 0.28f;
            char buf[32]; sprintf_s(buf, "%.0fm", dist);
            float cx = (left + right) * 0.5f;
            float tw = ImGui::CalcTextSize(buf).x;
            text_outline(dl, cx - tw * 0.5f, bottom + 2, IM_COL32(200, 200, 200, 255), buf);
        }
    }
}

inline void render_menu(player_cache& pcache) {
    ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
    ImGui::Begin("chicago", nullptr, ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("players")) {
            pcache.refresh();
            for (size_t i = 0; i < pcache.list.size(); i++) {
                std::string label = pcache.list[i].name;
                if (pcache.is_local(i)) label += " (you)";
                ImGui::Text("%s", label.c_str());
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("visual")) {
            ImGui::Checkbox("enabled", &enabled);
            ImGui::Checkbox("box", &box);
            ImGui::Checkbox("corner", &corner);
            ImGui::Checkbox("name", &name);
            ImGui::Checkbox("distance", &distance);
            ImGui::Checkbox("localplayer", &localplayer);
            ImGui::ColorEdit4("box color", box_color);
            ImGui::ColorEdit4("name color", name_color);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("aimbot")) {
            aimbot::render_tab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

}
