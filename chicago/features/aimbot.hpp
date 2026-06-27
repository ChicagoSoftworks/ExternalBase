#pragma once
#define NOMINMAX
#include <windows.h>
#include <cmath>
#include <thread>
#include <atomic>
#include <algorithm>
#include "../deps/imgui/imgui.h"
#include "../rbx/playerdb.hpp"
#include "../rbx/offsets.hpp"
#include "../util/loglib.hpp"

namespace aimbot {

inline bool enabled = false;
inline bool aim_on_key = true;
inline int aim_key = VK_RBUTTON;
inline int aim_mode = 0;
inline bool show_fov = true;
inline float fov_size = 100.0f;
inline float fov_color[4] = { 1.0f, 1.0f, 1.0f, 0.3f };
inline int target_bone = 1;
inline bool smoothing = false;
inline float smooth_x = 10.0f;
inline float smooth_y = 10.0f;
inline int smooth_style = 0;
inline bool prediction = false;
inline float pred_x = 10.0f;
inline float pred_y = 10.0f;
inline bool sticky_target = true;
inline bool teamcheck = false;

}

static float easing_apply(int style, float t) {
        t = (std::max)(0.01f, (std::min)(1.f, t));
    switch (style) {
    case 1: return t;
    case 2: return t * t;
    case 3: return t * (2.f - t);
    case 4: return t < 0.5f ? 2.f * t * t : -1.f + (4.f - 2.f * t) * t;
    default: return t;
    }
}

static vec3 vec3_normalize(vec3 v) {
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 0.001f) return { 0,0,0 };
    return { v.x / len, v.y / len, v.z / len };
}

static vec3 cross3(vec3 a, vec3 b) {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

static void send_mouse_move(float dx, float dy) {
    INPUT in = {};
    in.type = INPUT_MOUSE;
    in.mi.dx = (LONG)dx;
    in.mi.dy = (LONG)dy;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    SendInput(1, &in, sizeof(INPUT));
}

static bool get_part_world_pos(HANDLE proc, uint64_t player_addr, instance_offsets ioffs, esp_offsets eoffs, const char* part, vec3& out) {
    uint64_t model = 0;
    ReadProcessMemory(proc, (LPCVOID)(player_addr + eoffs.ModelInstance), &model, 8, nullptr);
    if (!model) return false;
    uint64_t p = find_child_by_name(proc, model, ioffs, part);
    if (!p) return false;
    uint64_t prim = 0;
    ReadProcessMemory(proc, (LPCVOID)(p + eoffs.BasePartPrimitive), &prim, 8, nullptr);
    if (!prim) return false;
    ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Position), &out, 12, nullptr);
    return true;
}

static bool get_target_pos(HANDLE proc, uint64_t player_addr, instance_offsets ioffs, esp_offsets eoffs, vec3& out) {
    if (aimbot::target_bone == 0) {
        if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "Head", out)) return true;
        if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "HeadMesh", out)) return true;
    }
    if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "HumanoidRootPart", out)) return true;
    if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "UpperTorso", out)) return true;
    if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "Torso", out)) return true;
    if (get_part_world_pos(proc, player_addr, ioffs, eoffs, "LowerTorso", out)) return true;
    return false;
}

static matrix3 look_at_matrix(vec3 from, vec3 to) {
    vec3 fwd = vec3_normalize({ to.x - from.x, to.y - from.y, to.z - from.z });
    vec3 right = vec3_normalize(cross3({ 0, 1, 0 }, fwd));
    vec3 up = cross3(fwd, right);
    matrix3 m = {};
    m.m[0] = -right.x; m.m[1] = up.x; m.m[2] = -fwd.x;
    m.m[3] = -right.y; m.m[4] = up.y; m.m[5] = -fwd.y;
    m.m[6] = -right.z; m.m[7] = up.z; m.m[8] = -fwd.z;
    return m;
}

struct aimbot_handler {
    HANDLE proc = nullptr;
    uint64_t base = 0;
    offsets_t* offs = nullptr;
    instance_offsets ioffs;
    esp_offsets eoffs;
    std::atomic<bool> active{ false };
    std::thread worker;
    int sticky_idx = -1;

    void init(HANDLE p, uint64_t b, offsets_t* o, instance_offsets io, esp_offsets eo) {
        proc = p; base = b; offs = o; ioffs = io; eoffs = eo; active = true;
        worker = std::thread([this] { run(); });
    }

    void run() {
        while (active) {
            if (!aimbot::enabled || (aimbot::aim_on_key && !(GetAsyncKeyState(aimbot::aim_key) & 0x8000))) {
                sticky_idx = -1;
                Sleep(10);
                continue;
            }
            uint64_t ve = 0, fake = 0, dm = 0;
            ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
            if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
            if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
            if (!dm) { Sleep(100); continue; }

            vec2 dims = {}; matrix4 view = {};
            if (ve) {
                ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.Dimensions), &dims, 8, nullptr);
                ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.ViewMatrix), &view, 64, nullptr);
            }
            float cx = dims.x * 0.5f, cy = dims.y * 0.5f;

            auto players = get_players(proc, dm, ioffs);
            if (players.empty()) { Sleep(100); continue; }

            uint64_t local = get_local_player(proc, dm, ioffs, eoffs.LocalPlayer);

            uint64_t local_team = 0;
            if (aimbot::teamcheck && local)
                ReadProcessMemory(proc, (LPCVOID)(local + offs->Player.Team), &local_team, 8, nullptr);

            auto try_get_pos = [&](uint64_t addr, vec3& o) -> bool {
                return get_target_pos(proc, addr, ioffs, eoffs, o);
            };

            int best = -1;
            float best_dist = aimbot::fov_size;

            if (aimbot::sticky_target && sticky_idx >= 0 && (size_t)sticky_idx < players.size()) {
                vec3 wp;
                if (try_get_pos(players[sticky_idx].address, wp)) {
                    float sx, sy;
                    if (world_to_screen_clip(wp, dims, view, sx, sy)) {
                        float dx = sx - cx, dy = sy - cy;
                        float d = sqrtf(dx * dx + dy * dy);
                        if (d < aimbot::fov_size) {
                            best = sticky_idx;
                            best_dist = d;
                        } else {
                            sticky_idx = -1;
                        }
                    } else {
                        sticky_idx = -1;
                    }
                } else {
                    sticky_idx = -1;
                }
            }

            for (size_t i = 0; i < players.size(); i++) {
                if (players[i].address == local) continue;
                if (aimbot::teamcheck) {
                    uint64_t t = 0;
                    ReadProcessMemory(proc, (LPCVOID)(players[i].address + offs->Player.Team), &t, 8, nullptr);
                    if (t && t == local_team) continue;
                }
                if (aimbot::sticky_target && sticky_idx >= 0 && (int)i == sticky_idx) continue;
                vec3 wp;
                if (!try_get_pos(players[i].address, wp)) continue;
                float sx, sy;
                if (!world_to_screen_clip(wp, dims, view, sx, sy)) continue;
                float dx = sx - cx, dy = sy - cy;
                float d = sqrtf(dx * dx + dy * dy);
                if (d < best_dist) { best_dist = d; best = (int)i; }
            }

            if (best < 0) { Sleep(5); continue; }
            if (aimbot::sticky_target && sticky_idx < 0) sticky_idx = best;

            vec3 target_pos;
            if (!try_get_pos(players[best].address, target_pos)) { Sleep(5); continue; }

            uint64_t ws = find_child_by_name(proc, dm, ioffs, "Workspace");
            if (!ws) { Sleep(100); continue; }
            uint64_t camera = 0;
            ReadProcessMemory(proc, (LPCVOID)(ws + offs->Workspace.CurrentCamera), &camera, 8, nullptr);
            if (!camera) {
                camera = find_child_by_name(proc, ws, ioffs, "Camera");
                if (!camera) { Sleep(100); continue; }
            }

            if (aimbot::aim_mode == 0) {
                vec3 cam_pos;
                ReadProcessMemory(proc, (LPCVOID)(camera + offs->Camera.Position), &cam_pos, 12, nullptr);
                matrix3 target_rot = look_at_matrix(cam_pos, target_pos);

                if (!aimbot::smoothing) {
                    WriteProcessMemory(proc, (LPVOID)(camera + offs->Camera.Rotation), &target_rot, sizeof(matrix3), nullptr);
                } else {
                    matrix3 cur_rot;
                    ReadProcessMemory(proc, (LPCVOID)(camera + offs->Camera.Rotation), &cur_rot, sizeof(matrix3), nullptr);
                    float tx = 1.f / aimbot::smooth_x;
                    float ty = 1.f / aimbot::smooth_y;
                    float sfx = easing_apply(aimbot::smooth_style, tx);
                    float sfy = easing_apply(aimbot::smooth_style, ty);
                    matrix3 sm = {};
                    for (int r = 0; r < 3; r++)
                        for (int c = 0; c < 3; c++) {
                            float sf = (c == 0) ? sfx : sfy;
                            sm.m[r * 3 + c] = cur_rot.m[r * 3 + c] + (target_rot.m[r * 3 + c] - cur_rot.m[r * 3 + c]) * sf;
                        }
                    WriteProcessMemory(proc, (LPVOID)(camera + offs->Camera.Rotation), &sm, sizeof(matrix3), nullptr);
                }
            } else {
                float sx, sy;
                if (world_to_screen_clip(target_pos, dims, view, sx, sy)) {
                    POINT cp;
                    HWND wnd = FindWindowA(nullptr, "Roblox");
                    if (GetCursorPos(&cp)) {
                        if (wnd) ScreenToClient(wnd, &cp);
                        float dx = sx - (float)cp.x;
                        float dy = sy - (float)cp.y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist >= 3.f) {
                            if (!aimbot::smoothing) {
                                send_mouse_move((float)std::round(dx), (float)std::round(dy));
                            } else {
                                float tx = 1.f / aimbot::smooth_x;
                                float ty = 1.f / aimbot::smooth_y;
                                float sfx = easing_apply(aimbot::smooth_style, tx);
                                float sfy = easing_apply(aimbot::smooth_style, ty);
                                static float rem_x = 0.f, rem_y = 0.f;
                                rem_x += dx * sfx;
                                rem_y += dy * sfy;
                                int mx = (int)std::round(rem_x);
                                int my = (int)std::round(rem_y);
                                if (mx || my) {
                                    send_mouse_move((float)mx, (float)my);
                                    rem_x -= (float)mx;
                                    rem_y -= (float)my;
                                }
                            }
                        }
                    }
                }
            }
            Sleep(1);
        }
    }

    void stop() {
        active = false;
        if (worker.joinable()) worker.join();
    }
};

namespace aimbot {

inline void draw_fov(vec2 dims) {
    if (!show_fov || !enabled) return;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImU32 col = IM_COL32((int)(fov_color[0] * 255), (int)(fov_color[1] * 255), (int)(fov_color[2] * 255), (int)(fov_color[3] * 255));
    dl->AddCircle(ImVec2(dims.x * 0.5f, dims.y * 0.5f), fov_size, col, 64);
}

inline void render_tab() {
    ImGui::Checkbox("enabled", &enabled);
    ImGui::SameLine();
    ImGui::Checkbox("aim on key", &aim_on_key);
    if (aim_on_key) {
        int k = aim_key;
        ImGui::SameLine();
        ImGui::Text("key: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        if (ImGui::BeginCombo("##aimkey", aim_key == VK_RBUTTON ? "RMB" : aim_key == VK_LBUTTON ? "LMB" : aim_key == VK_XBUTTON1 ? "XMB1" : "XMB2")) {
            if (ImGui::Selectable("RMB", aim_key == VK_RBUTTON)) aim_key = VK_RBUTTON;
            if (ImGui::Selectable("LMB", aim_key == VK_LBUTTON)) aim_key = VK_LBUTTON;
            if (ImGui::Selectable("XMB1", aim_key == VK_XBUTTON1)) aim_key = VK_XBUTTON1;
            if (ImGui::Selectable("XMB2", aim_key == VK_XBUTTON2)) aim_key = VK_XBUTTON2;
            ImGui::EndCombo();
        }
    }
    ImGui::Combo("mode", &aim_mode, "camera\0mouse\0");
    ImGui::Combo("target", &target_bone, "head\0root\0");
    ImGui::Checkbox("smoothing", &smoothing);
    if (smoothing) {
        ImGui::SliderFloat("smooth x", &smooth_x, 1.f, 50.f, "%.0f");
        ImGui::SliderFloat("smooth y", &smooth_y, 1.f, 50.f, "%.0f");
        ImGui::Combo("easing", &smooth_style, "linear\0quad in\0quad out\0quad in-out\0");
    }
    ImGui::Checkbox("prediction", &prediction);
    if (prediction) {
        ImGui::SliderFloat("pred x", &pred_x, 0.f, 20.f, "%.0f");
        ImGui::SliderFloat("pred y", &pred_y, 0.f, 20.f, "%.0f");
    }
    ImGui::Checkbox("sticky target", &sticky_target);
    ImGui::Checkbox("teamcheck", &teamcheck);
    ImGui::Checkbox("show fov", &show_fov);
    if (show_fov) {
        ImGui::SliderFloat("fov size", &fov_size, 10.f, 500.f, "%.0f");
        ImGui::ColorEdit4("fov color", fov_color, ImGuiColorEditFlags_NoInputs);
    }
}

}
