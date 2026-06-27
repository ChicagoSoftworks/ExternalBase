#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "playerdb.hpp"
#include "../util/loglib.hpp"

struct player_cache {
    std::vector<player_entry> list;
    uint64_t base = 0;
    uint64_t ve = 0;
    uint64_t dm = 0;
    uint64_t local_addr = 0;
    HANDLE proc = nullptr;
    instance_offsets ioffs = {};
    esp_offsets eoffs = {};
    offsets_t* offs = nullptr;
    uint64_t last_refresh = 0;
    uint64_t ttl = 1000;

    void init(HANDLE p, uint64_t base_addr, offsets_t* o, instance_offsets io, esp_offsets eo) {
        proc = p; base = base_addr; offs = o; ioffs = io; eoffs = eo;
    }

    void refresh() {
        uint64_t now = GetTickCount64();
        if (now - last_refresh < ttl && dm) return;
        ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
        if (!ve) { last_refresh = now; return; }
        uint64_t fake = 0;
        ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
        if (!fake) { last_refresh = now; return; }
        uint64_t new_dm = 0;
        ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &new_dm, 8, nullptr);
        if (!new_dm) { last_refresh = now; return; }
        dm = new_dm;
        list = get_players(proc, dm, ioffs);
        local_addr = get_local_player(proc, dm, ioffs, eoffs.LocalPlayer);
        last_refresh = now;
    }

    void force_refresh() {
        ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
        if (!ve) return;
        uint64_t fake = 0;
        ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
        if (!fake) return;
        ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
        if (!dm) return;
        list = get_players(proc, dm, ioffs);
        local_addr = get_local_player(proc, dm, ioffs, eoffs.LocalPlayer);
        last_refresh = GetTickCount64();
    }

    size_t count() { refresh(); return list.size(); }

    std::string get_name(size_t i) {
        refresh();
        if (i >= list.size()) return "";
        return list[i].name;
    }

    uint64_t get_addr(size_t i) {
        refresh();
        if (i >= list.size()) return 0;
        return list[i].address;
    }

    bool is_local(size_t i) {
        refresh();
        if (i >= list.size()) return false;
        return list[i].address == local_addr;
    }
};
