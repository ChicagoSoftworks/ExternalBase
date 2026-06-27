#pragma once
#include <windows.h>
#include <cstdint>
#include <thread>
#include <atomic>
#include <vector>
#include <string>
#include <unordered_map>
#include "../util/loglib.hpp"
#include "../rbx/playerdb.hpp"

struct player_handler {
    HANDLE proc = nullptr;
    uint64_t base = 0;
    offsets_t* offs = nullptr;
    instance_offsets ioffs;
    std::atomic<bool> active{ false };
    std::thread worker;
    std::unordered_map<uint64_t, std::string> known;

    ~player_handler() { stop(); }

    void init(HANDLE p, uint64_t b, offsets_t* o, instance_offsets io) {
        proc = p; base = b; offs = o; ioffs = io; active = true;
        worker = std::thread([this] { run(); });
    }

    void run() {
        while (active) {
            uint64_t ve = 0, fake = 0, dm = 0;
            ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
            if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
            if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
            if (!dm) { Sleep(2000); continue; }
            auto current = get_players(proc, dm, ioffs);
            std::unordered_map<uint64_t, std::string> now;
            for (auto& p : current) now[p.address] = p.name;
            for (auto& p : now) {
                if (!known.count(p.first))
                    //loglib::gpu("+ %s\n", p.second.c_str()); temporary
                    printf("");
            }
            for (auto& p : known) {
                if (!now.count(p.first))
                    //loglib::err("- %s\n", p.second.c_str()); temporary
                    printf("");
            }
            known = std::move(now);
            Sleep(2000);
        }
    }

    void stop() {
        active = false;
        if (worker.joinable()) worker.join();
    }
};
