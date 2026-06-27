#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <functional>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")
#include "../deps/imgui/imgui.h"
#include "../deps/imgui/imgui_impl_win32.h"
#include "../deps/imgui/imgui_impl_dx11.h"

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace overlay {

static HWND hwnd = nullptr;
static HWND target = nullptr;
static ID3D11Device* device = nullptr;
static ID3D11DeviceContext* ctx = nullptr;
static IDXGISwapChain* swap = nullptr;
static ID3D11RenderTargetView* rtv = nullptr;
static bool running = false;
inline bool menu = true;
static std::function<void()> render_fn = nullptr;

static BOOL CALLBACK enum_proc(HWND h, LPARAM lp) {
    DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
    if (pid == (DWORD)lp) { target = h; return FALSE; }
    return TRUE;
}

static void find_target() {
    target = nullptr;
    DWORD pid = GetCurrentProcessId();
    // pid is our own process, not Roblox's. We need the Roblox PID from outside.
    // Instead: find a visible window belonging to RobloxPlayerBeta
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W pe = { sizeof(pe) };
    DWORD rbx_pid = 0;
    for (BOOL ok = Process32FirstW(snap, &pe); ok; ok = Process32NextW(snap, &pe)) {
        if (!_wcsicmp(pe.szExeFile, L"RobloxPlayerBeta.exe")) { rbx_pid = pe.th32ProcessID; break; }
    }
    CloseHandle(snap);
    if (!rbx_pid) return;
    EnumWindows(enum_proc, (LPARAM)rbx_pid);
}

static bool resize_d3d(int w, int h) {
    if (!swap) return false;
    if (rtv) { rtv->Release(); rtv = nullptr; }
    HRESULT hr = swap->ResizeBuffers(1, w, h, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) return false;
    ID3D11Texture2D* back = nullptr;
    if (FAILED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back)) || !back) return false;
    device->CreateRenderTargetView(back, nullptr, &rtv);
    back->Release();
    return true;
}

static LRESULT CALLBACK wndproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hw, msg, wp, lp)) return true;
    switch (msg) {
        case WM_DESTROY: running = false; return 0;
    }
    return DefWindowProcA(hw, msg, wp, lp);
}

static bool create_device() {
    RECT r = {}; GetClientRect(target ? target : GetDesktopWindow(), &r);
    int w = r.right - r.left, h = r.bottom - r.top;
    if (w < 100 || h < 100) { w = GetSystemMetrics(SM_CXSCREEN); h = GetSystemMetrics(SM_CYSCREEN); }

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.Width = w;
    sd.BufferDesc.Height = h;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &swap, &device, nullptr, &ctx)))
        return false;
    ID3D11Texture2D* back = nullptr;
    if (FAILED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back)) || !back) return false;
    device->CreateRenderTargetView(back, nullptr, &rtv);
    back->Release();
    return true;
}

inline void reposition() {
    if (!target) { find_target(); if (!target) return; }
    POINT p = { 0, 0 }; ClientToScreen(target, &p);
    RECT c = {}; GetClientRect(target, &c);
    int cw = c.right, ch = c.bottom;
    if (cw < 50 || ch < 50) return;
    RECT cur = {}; GetWindowRect(hwnd, &cur);
    int cur_w = cur.right - cur.left, cur_h = cur.bottom - cur.top;
    if (p.x == cur.left && p.y == cur.top && cw == cur_w && ch == cur_h) return;
    SetWindowPos(hwnd, HWND_TOPMOST, p.x, p.y, cw, ch, SWP_SHOWWINDOW);
    resize_d3d(cw, ch);
}

inline bool init(std::function<void()> cb) {
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = wndproc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "ChicagoOverlay";
    RegisterClassExA(&wc);

    find_target();

    int x = 0, y = 0, w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
    if (target) {
        POINT p = { 0, 0 }; ClientToScreen(target, &p); x = p.x; y = p.y;
        RECT c = {}; GetClientRect(target, &c); w = c.right; h = c.bottom;
    }

    hwnd = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        "ChicagoOverlay", "", WS_POPUP,
        x, y, w, h, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return false;
    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    MARGINS m = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(hwnd, &m);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    if (!create_device()) return false;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, ctx);
    render_fn = cb;
    running = true;
    menu = true;
    MSG msg;
    bool prev_ins = false;
    while (running) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        reposition();
        bool ins = GetAsyncKeyState(VK_INSERT) & 1;
        if (ins && !prev_ins) {
            prev_ins = true;
            menu = !menu;
            LONG_PTR ex = WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST;
            if (!menu) ex |= WS_EX_TRANSPARENT;
            SetWindowLongPtrA(hwnd, GWL_EXSTYLE, ex);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        }
        if (!ins) prev_ins = false;
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        if (render_fn) render_fn();
        ImGui::Render();
        ctx->OMSetRenderTargets(1, &rtv, nullptr);
        float clear[4] = { 0, 0, 0, 0 };
        ctx->ClearRenderTargetView(rtv, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swap->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    if (rtv) { rtv->Release(); rtv = nullptr; }
    if (swap) { swap->Release(); swap = nullptr; }
    if (ctx) { ctx->Release(); ctx = nullptr; }
    if (device) { device->Release(); device = nullptr; }
    if (hwnd) { DestroyWindow(hwnd); hwnd = nullptr; }
    return true;
}

}
