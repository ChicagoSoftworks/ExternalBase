#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cfloat>
#include "../util/loglib.hpp"
#include "../math/math.hpp"

struct instance_offsets {
    uint64_t ChildrenStart, ChildrenEnd, Name;
};

struct esp_offsets {
    uint64_t ModelInstance, PrimaryPart, BasePartPrimitive, Position, Size, Rotation, ViewMatrix, Dimensions, LocalPlayer;
};

struct player_entry {
    std::string name;
    uint64_t address;
};

inline std::string read_str(HANDLE proc, uint64_t addr) {
    uint64_t name_ptr = 0;
    ReadProcessMemory(proc, (LPCVOID)addr, &name_ptr, 8, nullptr);
    if (!name_ptr || name_ptr == 0xFFFFFFFFFFFFFFFF) return {};
    int32_t length = 0;
    ReadProcessMemory(proc, (LPCVOID)(name_ptr + 16), &length, 4, nullptr);
    if (length <= 0 || length > 255) return {};
    uint64_t data_addr = name_ptr;
    if (length >= 16) {
        ReadProcessMemory(proc, (LPCVOID)name_ptr, &data_addr, 8, nullptr);
        if (!data_addr) return {};
    }
    char buf[256] = {};
    SIZE_T br = 0;
    ReadProcessMemory(proc, (LPCVOID)data_addr, buf, (DWORD)length, &br);
    if (br < 1) return {};
    return std::string(buf, (size_t)br);
}

inline std::vector<uint64_t> get_children(HANDLE proc, uint64_t addr, instance_offsets offs) {
    std::vector<uint64_t> children;
    uint64_t cs = 0;
    ReadProcessMemory(proc, (LPCVOID)(addr + offs.ChildrenStart), &cs, 8, nullptr);
    if (!cs) return children;
    uint64_t start = 0, end = 0;
    ReadProcessMemory(proc, (LPCVOID)cs, &start, 8, nullptr);
    ReadProcessMemory(proc, (LPCVOID)(cs + offs.ChildrenEnd), &end, 8, nullptr);
    if (!start || !end || end <= start) return children;
    for (uint64_t p = start; p < end; p += 16) {
        uint64_t child = 0;
        ReadProcessMemory(proc, (LPCVOID)p, &child, 8, nullptr);
        if (child && child != 0xFFFFFFFFFFFFFFFF)
            children.push_back(child);
    }
    return children;
}

inline uint64_t find_child_by_name(HANDLE proc, uint64_t parent, instance_offsets offs, const char* target) {
    auto children = get_children(proc, parent, offs);
    for (auto& child : children) {
        std::string name = read_str(proc, child + offs.Name);
        if (name == target) return child;
    }
    return 0;
}

inline std::vector<player_entry> get_players(HANDLE proc, uint64_t dm, instance_offsets offs) {
    std::vector<player_entry> players;
    auto top = get_children(proc, dm, offs);
    for (auto& child : top) {
        std::string name = read_str(proc, child + offs.Name);
        if (name == "Players") {
            auto plrs = get_children(proc, child, offs);
            for (auto& p : plrs) {
                std::string pname = read_str(proc, p + offs.Name);
                if (!pname.empty()) players.push_back({ pname, p });
            }
            break;
        }
    }
    return players;
}

inline uint64_t get_local_player(HANDLE proc, uint64_t dm, instance_offsets offs, uint64_t local_offset) {
    auto top = get_children(proc, dm, offs);
    for (auto& child : top) {
        std::string name = read_str(proc, child + offs.Name);
        if (name == "Players") {
            uint64_t local = 0;
            ReadProcessMemory(proc, (LPCVOID)(child + local_offset), &local, 8, nullptr);
            return local;
        }
    }
    return 0;
}

inline bool world_to_screen_clip(vec3 world, vec2 dims, matrix4 view, float& sx, float& sy) {
    vec4 clip = mul(view, { world.x, world.y, world.z, 1.f });
    if (clip.w < 0.1f) return false;
    sx = (dims.x * 0.5f * clip.x / clip.w) + (dims.x * 0.5f);
    sy = -(dims.y * 0.5f * clip.y / clip.w) + (dims.y * 0.5f);
    return true;
}

inline bool compute_bounds(HANDLE proc, uint64_t player_addr, instance_offsets ioffs, esp_offsets eoffs, vec2 dims, matrix4 view, float& left, float& top, float& right, float& bottom) {
    left = top = FLT_MAX; right = bottom = -FLT_MAX;
    uint64_t model = 0;
    ReadProcessMemory(proc, (LPCVOID)(player_addr + eoffs.ModelInstance), &model, 8, nullptr);
    if (!model) return false;
    auto children = get_children(proc, model, ioffs);
    static const vec3 corner_signs[8] = {
        {-1,-1,-1},{1,-1,-1},{-1,1,-1},{1,1,-1},
        {-1,-1,1},{1,-1,1},{-1,1,1},{1,1,1}
    };
    int corner_count = 0;
    float sum_sx = 0, sum_sy = 0;
    float corners_sx[256], corners_sy[256];
    for (auto& child : children) {
        uint64_t prim = 0;
        ReadProcessMemory(proc, (LPCVOID)(child + eoffs.BasePartPrimitive), &prim, 8, nullptr);
        if (!prim) continue;
        vec3 pos = {}, size = {}; matrix3 rot = {};
        ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Position), &pos, 12, nullptr);
        ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Size), &size, 12, nullptr);
        ReadProcessMemory(proc, (LPCVOID)(prim + eoffs.Rotation), &rot, sizeof(matrix3), nullptr);
        if (!std::isfinite(pos.x) || !std::isfinite(pos.y) || !std::isfinite(pos.z)) continue;
        if (size.x < 0.001f || size.x > 200.f || size.y < 0.001f || size.y > 200.f || size.z < 0.001f || size.z > 200.f) continue;
        for (int c = 0; c < 8 && corner_count < 256; c++) {
            vec3 he = { corner_signs[c].x * size.x * 0.5f, corner_signs[c].y * size.y * 0.5f, corner_signs[c].z * size.z * 0.5f };
            vec3 w = { pos.x + rot.m[0] * he.x + rot.m[1] * he.y + rot.m[2] * he.z,
                       pos.y + rot.m[3] * he.x + rot.m[4] * he.y + rot.m[5] * he.z,
                       pos.z + rot.m[6] * he.x + rot.m[7] * he.y + rot.m[8] * he.z };
            float sx, sy;
            if (!world_to_screen_clip(w, dims, view, sx, sy)) continue;
            corners_sx[corner_count] = sx;
            corners_sy[corner_count] = sy;
            corner_count++;
            sum_sx += sx; sum_sy += sy;
        }
    }
    if (corner_count < 4) return false;
    float cx = sum_sx / corner_count;
    float cy = sum_sy / corner_count;
    for (int i = 0; i < corner_count; i++) {
        float dx = fabsf(corners_sx[i] - cx);
        float dy = fabsf(corners_sy[i] - cy);
        if (dx > dims.x * 0.8f || dy > dims.y * 0.8f) continue;
        if (corners_sx[i] < left) left = corners_sx[i];
        if (corners_sy[i] < top) top = corners_sy[i];
        if (corners_sx[i] > right) right = corners_sx[i];
        if (corners_sy[i] > bottom) bottom = corners_sy[i];
    }
    if (left >= right || top >= bottom) return false;
    float w = right - left, h = bottom - top;
    if (w > dims.x * 0.6f || h > dims.y * 0.9f) {
        float ocx = (left + right) * 0.5f;
        float ocy = (top + bottom) * 0.5f;
        float cw = w < dims.x * 0.3f ? w : dims.x * 0.3f;
        float ch = h < dims.y * 0.45f ? h : dims.y * 0.45f;
        left = ocx - cw; right = ocx + cw;
        top = ocy - ch; bottom = ocy + ch;
    }
    return true;
}
