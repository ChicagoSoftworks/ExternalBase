#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include "rbx/offsets.hpp"
#include "util/loglib.hpp"
#include "rbx/playerdb.hpp"
#include "rbx/cache.hpp"
#include "features/overlay.hpp"
#include "features/esp.hpp"
#include "tphandler/tphandler.hpp"
#include "tphandler/playerhandler.hpp"

int main() {
    auto offs = load_offsets();
    SetConsoleTitleW(L"discord -> @1jym");
    if (offs.version.empty()) return 1;
    loglib::log("%s\n", offs.version.c_str());
    uint32_t pid = 0;
    uint64_t base = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = { sizeof(pe) };
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (!_wcsicmp(pe.szExeFile, L"RobloxPlayerBeta.exe")) { pid = pe.th32ProcessID; break; }
    }
    CloseHandle(snap);
    if (!pid) { loglib::log("process not found\n"); return 1; }
    HANDLE proc = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, pid);
    if (!proc) { loglib::log("OpenProcess failed\n"); return 1; }
    HANDLE mod = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    MODULEENTRY32W me = { sizeof(me) };
    for (BOOL ok = Module32FirstW(mod, &me); ok; ok = Module32NextW(mod, &me)) {
        if (!_wcsicmp(me.szModule, L"RobloxPlayerBeta.exe")) { base = (uint64_t)me.modBaseAddr; break; }
    }
    CloseHandle(mod);
    uint64_t ve = 0, fake = 0, dm = 0;
    ReadProcessMemory(proc, (LPCVOID)(base + offs.VisualEngine.Pointer), &ve, 8, nullptr);
    if (ve) {
        ReadProcessMemory(proc, (LPCVOID)(ve + offs.VisualEngine.FakeDataModel), &fake, 8, nullptr);
        if (fake) ReadProcessMemory(proc, (LPCVOID)(fake + offs.FakeDataModel.RealDataModel), &dm, 8, nullptr);
    }
    player_cache pcache;
    instance_offsets ioff = { offs.Instance.ChildrenStart, offs.Instance.ChildrenEnd, offs.Instance.Name };
    esp_offsets eoff = { offs.Player.ModelInstance, offs.Model.PrimaryPart, offs.BasePart.Primitive, offs.Primitive.Position, offs.Primitive.Size, offs.Primitive.Rotation, offs.VisualEngine.ViewMatrix, offs.VisualEngine.Dimensions, offs.Player.LocalPlayer };
    std::atomic<bool> tp_flag{ false };
    tp_handler tph;
    player_handler ph;
    aimbot_handler ah;
    if (dm) {
        pcache.init(proc, base, &offs, ioff, eoff);
        pcache.force_refresh();
        tph.init(proc, base, &offs, [&] { tp_flag = true; });
        ph.init(proc, base, &offs, ioff);
        ah.init(proc, base, &offs, ioff, eoff);
        for (size_t i = 0; i < pcache.count(); i++)
            printf("");
    }
    loglib::log("dm @ 0x%llx\n", dm); // we MUST know the datamodel address
    if (dm) {
        overlay::init([&] {
            if (tp_flag.exchange(false))
                pcache.force_refresh();
            pcache.refresh();
            vec2 dims = {}; matrix4 view = {};
            ReadProcessMemory(proc, (LPCVOID)(pcache.ve + pcache.eoffs.Dimensions), &dims, 8, nullptr);
            ReadProcessMemory(proc, (LPCVOID)(pcache.ve + pcache.eoffs.ViewMatrix), &view, 64, nullptr);
            if (esp::enabled)
                esp::draw_boxes(proc, pcache, dims, view, offs.Player.Team);
            aimbot::draw_fov(dims);
            if (overlay::menu)
                esp::render_menu(pcache);
        });
    } else {
        loglib::pause();
    }
    CloseHandle(proc);
    return dm ? 0 : 1;
}
