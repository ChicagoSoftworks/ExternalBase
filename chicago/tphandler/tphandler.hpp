#pragma once
#include <windows.h>
#include <cstdint>
#include <thread>
#include <atomic>
#include <functional>
#include "../util/loglib.hpp"
#include "../rbx/playerdb.hpp"

struct tp_handler {
    HANDLE proc = nullptr;
    uint64_t base = 0;
    offsets_t* offs = nullptr;
    std::function<void()> on_teleport;
    std::atomic<bool> active{ false };
    std::thread worker;
    uint64_t last_ve = 0;
    uint64_t last_dm = 0;
    uint64_t last_game = 0;
    uint64_t last_place = 0;
    uint64_t last_ws = 0;
    uint64_t last_players = 0;

    ~tp_handler() { stop(); }

    void init(HANDLE p, uint64_t b, offsets_t* o, std::function<void()> cb) {
        proc = p; base = b; offs = o; on_teleport = cb;
        snapshot();
        active = true;
        worker = std::thread([this] { run(); });
    }

    void snapshot() {
        instance_offsets io = { offs->Instance.ChildrenStart, offs->Instance.ChildrenEnd, offs->Instance.Name };
        uint64_t ve = 0, fake = 0, dm = 0;
        ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
        if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
        if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
        last_ve = ve; last_dm = dm;
        if (dm) {
            ReadProcessMemory(proc, (LPCVOID)(dm + offs->DataModel.GameId), &last_game, 8, nullptr);
            ReadProcessMemory(proc, (LPCVOID)(dm + offs->DataModel.PlaceId), &last_place, 8, nullptr);
            last_ws = find_child_by_name(proc, dm, io, "Workspace");
            last_players = find_child_by_name(proc, dm, io, "Players");
        }
    }

    void run() {
        instance_offsets io = { offs->Instance.ChildrenStart, offs->Instance.ChildrenEnd, offs->Instance.Name };
        while (active) {
            uint64_t ve = 0, fake = 0, dm = 0, game = 0, place = 0;
            ReadProcessMemory(proc, (LPCVOID)(base + offs->VisualEngine.Pointer), &ve, 8, nullptr);
            if (ve) ReadProcessMemory(proc, (LPCVOID)(ve + offs->VisualEngine.FakeDataModel), &fake, 8, nullptr);
            if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs->FakeDataModel.RealDataModel), &dm, 8, nullptr);
            if (dm) {
                ReadProcessMemory(proc, (LPCVOID)(dm + offs->DataModel.GameId), &game, 8, nullptr);
                ReadProcessMemory(proc, (LPCVOID)(dm + offs->DataModel.PlaceId), &place, 8, nullptr);
            }
            if (dm && (dm != last_dm || ve != last_ve || game != last_game || place != last_place)) {
                last_ve = ve; last_dm = dm; last_game = game; last_place = place;
                if (dm) {
                    last_ws = find_child_by_name(proc, dm, io, "Workspace");
                    last_players = find_child_by_name(proc, dm, io, "Players");
                }
                if (on_teleport) on_teleport();
            }
            Sleep(500);
        }
    }

    void stop() {
        active = false;
        if (worker.joinable()) worker.join();
    }
};
