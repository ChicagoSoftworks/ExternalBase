#pragma once
#define NOMINMAX
#include <windows.h>
#include <cmath>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include "../deps/imgui/imgui.h"
#include "../rbx/playerdb.hpp"
#include "../rbx/offsets.hpp"
#include "../util/loglib.hpp"
#include "ui.hpp"

namespace silent {

inline bool enabled = false;
inline bool sticky_aim = false;
inline bool show_fov = false;
inline float fov = 200.0f;
inline float fov_color[4] = { 1.0f, 1.0f, 1.0f, 0.3f };
inline bool fov_check = true;
inline bool knocked_check = false;
inline int aim_part = 0;
inline int keybind = 0;
inline int keybind_mode = 0;

}

static void w2s_silent(vec3 world, vec2 dims, matrix4 view, float& sx, float& sy) {
    float clipX = world.x * view.m[0][0] + world.y * view.m[0][1] + world.z * view.m[0][2] + view.m[0][3];
    float clipY = world.x * view.m[1][0] + world.y * view.m[1][1] + world.z * view.m[1][2] + view.m[1][3];
    float clipW = world.x * view.m[3][0] + world.y * view.m[3][1] + world.z * view.m[3][2] + view.m[3][3];
    if (clipW <= 1e-6f) { sx = -1; sy = -1; return; }
    float inv_w = 1.f / clipW;
    sx = (dims.x * 0.5f) * (clipX * inv_w + 1.f);
    sy = (dims.y * 0.5f) * (1.f - clipY * inv_w);
}

struct silent_handler {
    HANDLE proc = nullptr;
    uint64_t base = 0;
    offsets_t* offs = nullptr;
    instance_offsets ioffs;
    esp_offsets eoffs;
    std::atomic<bool> active{ false };
    std::thread worker1, worker2;
    std::atomic<bool> data_ready{ false };
    std::atomic<bool> found_target{ false };
    std::atomic<uint64_t> target_addr{ 0 };
    float cached_sx = 0, cached_sy = 0;
    int cached_player_idx = -1;
    bool locked = false;
    bool key_was_pressed = false;

    void init(HANDLE p, uint64_t b, offsets_t* o, instance_offsets io, esp_offsets eo) {
        proc = p; base = b; offs = o; ioffs = io; eoffs = eo; active = true;
        worker1 = std::thread([this] { thread_find(); });
        worker2 = std::thread([this] { thread_write(); });
    }

    void thread_find() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        while (active) {
            Sleep(10);
            if (!silent::enabled) { locked = false; data_ready = false; found_target = false; cached_player_idx = -1; Sleep(100); continue; }

            update_key_state();

            if (!locked) { data_ready = false; found_target = false; cached_player_idx = -1; Sleep(10); continue; }

            uint64_t ve = 0, fake = 0, dm = 0;
            ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
            if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
            if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
            if (!dm) { Sleep(100); continue; }

            vec2 dims = {}; matrix4 view = {};
            ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.Dimensions), &dims, 8, nullptr);
            ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.ViewMatrix), &view, 64, nullptr);

            auto players = get_players(proc, dm, ioffs);
            if (players.empty()) continue;

            uint64_t local = get_local_player(proc, dm, ioffs, eoffs.LocalPlayer);

            POINT cursor_point;
            HWND rblxWnd = FindWindowA(nullptr, "Roblox");
            if (!rblxWnd || !GetCursorPos(&cursor_point) || !ScreenToClient(rblxWnd, &cursor_point)) continue;

            vec2 cursor = { (float)cursor_point.x, (float)cursor_point.y };

            int best = -1;
            float best_dist = silent::fov;
            uint64_t best_part_addr = 0;
            vec3 best_part_pos = {};

            if (silent::sticky_aim && cached_player_idx >= 0 && (size_t)cached_player_idx < players.size()) {
                uint64_t old_addr = players[cached_player_idx].address;
                if (old_addr == local) { cached_player_idx = -1; }
                else if (old_addr) {
                    uint64_t model = 0;
                    ReadProcessMemory(proc, (LPCVOID)(old_addr + eoffs.ModelInstance), &model, 8, nullptr);
                    if (model) {
                        const char* part = silent::aim_part == 0 ? "Head" : "HumanoidRootPart";
                        uint64_t p = find_child_by_name(proc, model, ioffs, part);
                        if (!p && silent::aim_part == 1) {
                            p = find_child_by_name(proc, model, ioffs, "UpperTorso");
                            if (!p) p = find_child_by_name(proc, model, ioffs, "Torso");
                        }
                        if (p) {
                            uint64_t prim = 0;
                            ReadProcessMemory(proc, (LPCVOID)(p + eoffs.BasePartPrimitive), &prim, 8, nullptr);
                            if (prim) {
                                vec3 pos = {};
                                ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Position), &pos, 12, nullptr);
                                float sx, sy;
                                w2s_silent(pos, dims, view, sx, sy);
                                if (sx >= 0 && sy >= 0) {
                                    float d = sqrtf((sx - cursor.x) * (sx - cursor.x) + (sy - cursor.y) * (sy - cursor.y));
                                    if (!silent::fov_check || d <= silent::fov) {
                                        best = cached_player_idx; best_dist = d;
                                        best_part_addr = p; best_part_pos = pos;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (best < 0) {
                for (size_t i = 0; i < players.size(); i++) {
                    if (players[i].address == local) continue;
                    uint64_t model = 0;
                    ReadProcessMemory(proc, (LPCVOID)(players[i].address + eoffs.ModelInstance), &model, 8, nullptr);
                    if (!model) continue;
                    const char* part = silent::aim_part == 0 ? "Head" : "HumanoidRootPart";
                    uint64_t p = find_child_by_name(proc, model, ioffs, part);
                    if (!p && silent::aim_part == 1) {
                        p = find_child_by_name(proc, model, ioffs, "UpperTorso");
                        if (!p) p = find_child_by_name(proc, model, ioffs, "Torso");
                    }
                    if (!p) continue;
                    uint64_t prim = 0;
                    ReadProcessMemory(proc, (LPCVOID)(p + eoffs.BasePartPrimitive), &prim, 8, nullptr);
                    if (!prim) continue;
                    vec3 pos = {};
                    ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Position), &pos, 12, nullptr);
                    float sx, sy;
                    w2s_silent(pos, dims, view, sx, sy);
                    if (sx < 0 || sy < 0) continue;
                    float d = sqrtf((sx - cursor.x) * (sx - cursor.x) + (sy - cursor.y) * (sy - cursor.y));
                    if (silent::fov_check && d > silent::fov) continue;
                    if (d < best_dist) { best_dist = d; best = (int)i; best_part_addr = p; best_part_pos = pos; }
                }
            }

            if (best < 0) { data_ready = false; found_target = false; continue; }
            if (silent::sticky_aim && cached_player_idx < 0) cached_player_idx = best;

            float sx, sy;
            w2s_silent(best_part_pos, dims, view, sx, sy);
            if (sx < 0 || sy < 0) { data_ready = false; continue; }
            POINT cp;
            GetCursorPos(&cp);
            ScreenToClient(rblxWnd, &cp);
            float offset_y = dims.y - fabsf(dims.y - (float)cp.y) - 58;
            cached_sx = sx;
            cached_sy = sy;
            target_addr.store(players[best].address);
            data_ready = true;
            found_target = true;
        }
    }

    void thread_write() {
        uint64_t cached_input_object = 0;
        while (active) {
            Sleep(5);
            if (!silent::enabled || !locked || !data_ready) { Sleep(100); continue; }
            uint64_t ve = 0, fake = 0, dm = 0;
            ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
            if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
            if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
            if (!dm) { Sleep(100); continue; }
            uint64_t mouseservice = find_child_by_name(proc, dm, ioffs, "MouseService");
            if (!mouseservice) {
                auto children = get_children(proc, dm, ioffs);
                for (auto& child : children) {
                    std::string cls = read_str(proc, child + ioffs.ClassName);
                    if (cls == "MouseService") { mouseservice = child; break; }
                }
            }
            if (!mouseservice) { Sleep(100); continue; }
            cached_input_object = 0;
            uint64_t shared_ptr_data = 0;
            ReadProcessMemory(proc, (LPCVOID)(mouseservice + offs->MouseService.InputObject), &shared_ptr_data, 8, nullptr);
            ReadProcessMemory(proc, (LPCVOID)(mouseservice + offs->MouseService.InputObject + 8), &cached_input_object, 8, nullptr);
            if (!cached_input_object || cached_input_object == 0xFFFFFFFFFFFFFFFF) { Sleep(100); continue; }
            vec2 pos = { cached_sx, cached_sy };
            WriteProcessMemory(proc, (LPVOID)(cached_input_object + offs->MouseService.MousePosition), &pos, sizeof(vec2), nullptr);
        }
    }

    void update_key_state() {
        if (silent::keybind == 0) return;
        bool key_pressed = (GetAsyncKeyState(silent::keybind) & 0x8000) != 0;
        if (silent::keybind_mode == 0) {
            locked = key_pressed;
        } else {
            if (key_pressed && !key_was_pressed) locked = !locked;
            key_was_pressed = key_pressed;
        }
    }

    void stop() {
        active = false;
        if (worker1.joinable()) worker1.join();
        if (worker2.joinable()) worker2.join();
    }
};

namespace silent {

inline void draw_fov(vec2 dims) {
    if (!show_fov || !enabled) return;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImU32 col = IM_COL32((int)(fov_color[0] * 255), (int)(fov_color[1] * 255), (int)(fov_color[2] * 255), (int)(fov_color[3] * 255));
    dl->AddCircle(ImVec2(dims.x * 0.5f, dims.y * 0.5f), fov, col, 64);
}

inline void render_tab() {
    ImGui::Spacing();

    // left column
    ImGui::BeginChild("##sil", ImVec2(210, 265), false, ImGuiWindowFlags_NoScrollbar);

    ui::section("general", 0.6f);
    ui::checkbox("enabled", &enabled);

    ui::section("input", 0.6f);
    ui::combo("key mode", &keybind_mode, "hold\0toggle\0");
    ui::combo("aim part", &aim_part, "head\0root/torso\0");

    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::SetCursorPosX(250);

    // right column
    ImGui::BeginChild("##sir", ImVec2(225, 265), false, ImGuiWindowFlags_NoScrollbar);

    ui::section("targeting", 0.6f);
    ui::checkbox("sticky aim", &sticky_aim);
    ui::checkbox("fov check", &fov_check);
    ui::checkbox("knocked check", &knocked_check);

    ui::section("field of view", 0.6f);
    ui::checkbox("draw fov", &show_fov);
    if (show_fov) {
        ImGui::Indent();
        ui::slider_float("size", &fov, 10.f, 500.f, "%.0f");
        ui::color_edit4("color", fov_color);
        ImGui::Unindent();
    }

    ImGui::EndChild();
}

}
