#pragma once
#include <windows.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include "../deps/imgui/imgui.h"
#include "../rbx/cache.hpp"
#include "../rbx/playerdb.hpp"
#include "aimbot.hpp"
#include "silent.hpp"
#include "exploits.hpp"
#include "../fonts.hpp"
#include "../util/texture.hpp"
#include "ui.hpp"

namespace esp {

inline bool enabled = true;
inline bool box = true;
inline bool corner = false;
inline bool name = true;
inline bool distance = false;
inline bool localplayer = false;
inline bool teamcheck = false;
inline float box_color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
inline float name_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

inline bool chams = false;
inline float chams_color[4] = { 1.0f, 1.0f, 1.0f, 0.3f };
inline bool healthbar = false;
inline float healthbar_color[3] = { 0.f, 1.f, 0.f };
inline bool health_text = false;
inline bool tool = false;
inline float tool_color[4] = { 1.f, 1.f, 1.f, 1.f };

inline void text_outline(ImDrawList* dl, float x, float y, ImU32 col, const char* text) {
    ImU32 black = IM_COL32(0, 0, 0, 255);
    dl->AddText(ImVec2(x - 1, y - 1), black, text);
    dl->AddText(ImVec2(x + 1, y - 1), black, text);
    dl->AddText(ImVec2(x - 1, y + 1), black, text);
    dl->AddText(ImVec2(x + 1, y + 1), black, text);
    dl->AddText(ImVec2(x, y), col, text);
}

struct convex_hull_result {
    std::vector<ImVec2> hull;
    bool valid;
};

static convex_hull_result compute_part_hull(HANDLE proc, uint64_t part_addr, uint64_t prim_off, uint64_t pos_off, uint64_t size_off, uint64_t rot_off, vec2 dims, matrix4 view) {
    convex_hull_result res;
    if (!part_addr) return res;
    uint64_t prim = 0;
    ReadProcessMemory(proc, (LPCVOID)(part_addr + prim_off), &prim, 8, nullptr);
    if (!prim) return res;
    vec3 pos = {}, size = {}; matrix3 rot = {};
    ReadProcessMemory(proc, (LPCVOID)(prim + pos_off), &pos, 12, nullptr);
    ReadProcessMemory(proc, (LPCVOID)(prim + size_off), &size, 12, nullptr);
    ReadProcessMemory(proc, (LPCVOID)(prim + rot_off), &rot, sizeof(matrix3), nullptr);
    if (size.x < 0.001f || size.x > 200.f || size.y < 0.001f || size.y > 200.f || size.z < 0.001f || size.z > 200.f)
        return res;
    static const vec3 cs[8] = {
        {-1,-1,-1},{1,-1,-1},{-1,1,-1},{1,1,-1},
        {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1}
    };
    std::vector<ImVec2> projected;
    for (int i = 0; i < 8; i++) {
        vec3 he = { cs[i].x * size.x * 0.5f, cs[i].y * size.y * 0.5f, cs[i].z * size.z * 0.5f };
        vec3 w = {
            pos.x + rot.m[0] * he.x + rot.m[1] * he.y + rot.m[2] * he.z,
            pos.y + rot.m[3] * he.x + rot.m[4] * he.y + rot.m[5] * he.z,
            pos.z + rot.m[6] * he.x + rot.m[7] * he.y + rot.m[8] * he.z
        };
        float sx, sy;
        if (!world_to_screen_clip(w, dims, view, sx, sy)) continue;
        if (sx >= -10 && sy >= -10 && sx < dims.x + 10 && sy < dims.y + 10)
            projected.push_back(ImVec2(sx, sy));
    }
    if (projected.size() < 3) return res;
    std::sort(projected.begin(), projected.end(), [](const ImVec2& a, const ImVec2& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    std::vector<ImVec2> hull;
    auto cross = [](const ImVec2& o, const ImVec2& a, const ImVec2& b) {
        return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
    };
    for (auto& p : projected) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    size_t t = hull.size() + 1;
    for (int i = (int)projected.size() - 1; i >= 0; --i) {
        auto& p = projected[i];
        while (hull.size() >= t && cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }
    hull.pop_back();
    res.hull = std::move(hull);
    res.valid = true;
    return res;
}

inline void draw_boxes(HANDLE proc, player_cache& pcache, vec2 dims, matrix4 view, uint64_t team_off = 0) {
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImU32 black = IM_COL32(0, 0, 0, 255);

    pcache.refresh();
    auto list = pcache.list;

    uint64_t local_team = 0;
    if (teamcheck && team_off && pcache.local_addr)
        ReadProcessMemory(proc, (LPCVOID)(pcache.local_addr + team_off), &local_team, 8, nullptr);

    for (size_t i = 0; i < list.size(); i++) {
        if (!localplayer && pcache.is_local(i)) continue;
        if (teamcheck && team_off) {
            uint64_t t = 0;
            ReadProcessMemory(proc, (LPCVOID)(list[i].address + team_off), &t, 8, nullptr);
            if (t && t == local_team) continue;
        }

        auto addr = list[i].address;
        float left, top, right, bottom;
        if (!compute_bounds(proc, addr, pcache.ioffs, pcache.eoffs, dims, view, left, top, right, bottom))
            continue;

        if (chams) {
            uint64_t model = 0;
            ReadProcessMemory(proc, (LPCVOID)(addr + pcache.eoffs.ModelInstance), &model, 8, nullptr);
            if (model) {
                auto children = get_children(proc, model, pcache.ioffs);
                std::vector<std::vector<ImVec2>> all_hulls;
                for (auto& child : children) {
                    std::string cname = read_str(proc, child + pcache.ioffs.ClassName);
                    if (cname.find("Part") == std::string::npos) continue;
                    auto hull = compute_part_hull(proc, child, pcache.eoffs.BasePartPrimitive, pcache.eoffs.Position, pcache.eoffs.Size, pcache.eoffs.Rotation, dims, view);
                    if (hull.valid && hull.hull.size() >= 3)
                        all_hulls.push_back(std::move(hull.hull));
                }
                ImU32 chams_col = IM_COL32(
                    (int)(chams_color[0] * 255), (int)(chams_color[1] * 255),
                    (int)(chams_color[2] * 255), (int)(chams_color[3] * 255)
                );
                for (auto& h : all_hulls)
                    dl->AddConcavePolyFilled(h.data(), (int)h.size(), chams_col);
                for (auto& h : all_hulls)
                    dl->AddPolyline(h.data(), (int)h.size(), IM_COL32_WHITE, true, 1.f);
            }
        }

        if (healthbar) {
            float hp = get_humanoid_health(proc, addr, pcache.eoffs.ModelInstance, pcache.offs->Humanoid.Health, pcache.ioffs);
            float max_hp = get_humanoid_max_health(proc, addr, pcache.eoffs.ModelInstance, pcache.offs->Humanoid.MaxHealth, pcache.ioffs);
            if (max_hp > 0.f && hp >= 0.f) {
                float bar_w = 3.f;
                float bar_h = bottom - top;
                float bx = left - 6.f;
                float by = top;
                dl->AddRectFilled(ImVec2(bx - 1, by - 1), ImVec2(bx + bar_w + 1, by + bar_h + 1), black);
                dl->AddRectFilled(ImVec2(bx, by), ImVec2(bx + bar_w, by + bar_h), IM_COL32(50, 50, 50, 255));
                float pct = hp / max_hp;
                if (pct < 0.f) pct = 0.f; if (pct > 1.f) pct = 1.f;
                float fill_h = bar_h * pct;
                float fy = by + bar_h - fill_h;
                uint8_t r = (uint8_t)((1.f - pct) * 255);
                uint8_t g = (uint8_t)(pct * 255);
                dl->AddRectFilled(ImVec2(bx, fy), ImVec2(bx + bar_w, by + bar_h), IM_COL32(
                    (int)(healthbar_color[0] * 255), (int)(healthbar_color[1] * 255), (int)(healthbar_color[2] * 255), 255
                ));
                if (health_text && pct < 1.f) {
                    char hbuf[16]; sprintf_s(hbuf, "%.0f", hp);
                    float tx = bx - ImGui::CalcTextSize(hbuf).x;
                    float ty = by + bar_h * 0.5f - 4.f;
                    text_outline(dl, tx - 2, ty, IM_COL32(255, 255, 255, 255), hbuf);
                }
            }
        }

        ImU32 box_col = IM_COL32(
            (int)(box_color[0] * 255), (int)(box_color[1] * 255),
            (int)(box_color[2] * 255), (int)(box_color[3] * 255)
        );
        ImU32 name_col = IM_COL32(
            (int)(name_color[0] * 255), (int)(name_color[1] * 255),
            (int)(name_color[2] * 255), (int)(name_color[3] * 255)
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

        float dist_label_y = bottom + 2;

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
            text_outline(dl, cx - tw * 0.5f, dist_label_y, IM_COL32(200, 200, 200, 255), buf);
            dist_label_y += 10;
        }

        if (tool) {
            std::string tname = get_tool_name(proc, addr, pcache.ioffs, pcache.eoffs.ModelInstance);
            if (!tname.empty()) {
                float cx = (left + right) * 0.5f;
                float tw = ImGui::CalcTextSize(tname.c_str()).x;
                ImU32 tool_col = IM_COL32(
                    (int)(tool_color[0] * 255), (int)(tool_color[1] * 255),
                    (int)(tool_color[2] * 255), (int)(tool_color[3] * 255)
                );
                text_outline(dl, cx - tw * 0.5f, dist_label_y, tool_col, tname.c_str());
            }
        }
    }
}

inline void render_menu(player_cache& pcache) {
    static int tab = 0;
    ImGui::SetNextWindowSize({ 500, 370 });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 1.f));
    ImGui::Begin("catrine", nullptr, ImGuiWindowFlags_NoDecoration);
    {
        auto draw = ImGui::GetWindowDrawList();
        auto pos = ImGui::GetWindowPos();
        auto size = ImGui::GetWindowSize();

        draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 51), ImColor(24, 24, 24), 9.0f, ImDrawFlags_RoundCornersTop);

        if (tex::logo_srv && tex::logo_w > 0) {
            float aspect = (float)tex::logo_w / (float)tex::logo_h;
            float ih = 22.f, iw = ih * aspect;
            float lx = pos.x + 18, ly = pos.y + (51 - ih) * 0.5f;
            draw->AddImage((ImTextureID)tex::logo_srv, ImVec2(lx, ly), ImVec2(lx + iw, ly + ih));
        }

        ImGui::SetCursorPos({ 125, 19 });
        ImGui::BeginGroup();
        if (elements::tab("players", tab == 0)) tab = 0;
        ImGui::SameLine();
        if (elements::tab("visual", tab == 1)) tab = 1;
        ImGui::SameLine();
        if (elements::tab("aim", tab == 2)) tab = 2;
        ImGui::SameLine();
        if (elements::tab("silent", tab == 3)) tab = 3;
        ImGui::SameLine();
        if (elements::tab("exploits", tab == 4)) tab = 4;
        ImGui::EndGroup();

        switch (tab) {
        case 0: {
            if (fonts::medium) draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "players");
            ImGui::SetCursorPos({ 25, 85 });
            ImGui::BeginChild("##players", ImVec2(450, 265), false, ImGuiWindowFlags_NoScrollbar);
            pcache.refresh();
            int count = (int)pcache.list.size();
            ImGui::Text("%d player(s)", count);
            ImGui::Separator();
            for (size_t i = 0; i < pcache.list.size(); i++) {
                std::string label = pcache.list[i].name;
                if (pcache.is_local(i)) label += "  (you)";
                ImGui::BulletText("%s", label.c_str());
            }
            ImGui::EndChild();
            break;
        }
        case 1: {
            if (fonts::medium) draw->AddText(fonts::medium, 14.0f, ImVec2(pos.x + 25, pos.y + 60), ImColor(1.0f, 1.0f, 1.0f, 0.6f), "visual");
            ImGui::SetCursorPos({ 25, 85 });
            ImGui::BeginChild("##vis1", ImVec2(210, 265), false, ImGuiWindowFlags_NoScrollbar);
            ui::checkbox("enabled", &enabled);
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1,1,1,0.5f), "player");
            ImGui::Separator();
            ui::checkbox("box", &box);
            if (box) { ImGui::SameLine(120); ui::checkbox("corner", &corner); }
            ui::checkbox("name", &name);
            ui::checkbox("distance", &distance);
            ui::checkbox("chams", &chams);
            if (chams) { ImGui::Indent(); ui::color_edit4("color", chams_color); ImGui::Unindent(); }
            ui::checkbox("healthbar", &healthbar);
            if (healthbar) { ImGui::Indent(); ui::color_edit3("color", healthbar_color); ImGui::SameLine(); ui::checkbox("text", &health_text); ImGui::Unindent(); }
            ui::checkbox("tool name", &tool);
            ImGui::EndChild();
            ImGui::SetCursorPos({ 250, 85 });
            ImGui::BeginChild("##vis2", ImVec2(225, 265), false, ImGuiWindowFlags_NoScrollbar);
            ImGui::TextColored(ImVec4(1,1,1,0.5f), "filter");
            ImGui::Separator();
            ui::checkbox("localplayer", &localplayer);
            ui::checkbox("teamcheck", &teamcheck);
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1,1,1,0.5f), "colors");
            ImGui::Separator();
            ui::color_edit4("box", box_color, false);
            ui::color_edit4("name", name_color, false);
            if (tool) ui::color_edit4("tool", tool_color, false);
            ImGui::EndChild();
            break;
        }
        case 2:
            ImGui::SetCursorPos({ 25, 85 });
            ImGui::BeginChild("##aim", ImVec2(470, 265), false, ImGuiWindowFlags_NoScrollbar);
            aimbot::render_tab();
            ImGui::EndChild();
            break;
        case 3:
            ImGui::SetCursorPos({ 25, 85 });
            ImGui::BeginChild("##silent", ImVec2(470, 265), false, ImGuiWindowFlags_NoScrollbar);
            silent::render_tab();
            ImGui::EndChild();
            break;
        case 4:
            ImGui::SetCursorPos({ 25, 85 });
            ImGui::BeginChild("##exploits", ImVec2(470, 265), false, ImGuiWindowFlags_NoScrollbar);
            exploits::render_tab();
            ImGui::EndChild();
            break;
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

}
