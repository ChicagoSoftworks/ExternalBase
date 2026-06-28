#pragma once
#include <windows.h>
#include <d3d11.h>
#include <winhttp.h>
#include <wincodec.h>
#include <vector>
#include <string>
#include <dxgi.h>
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "dxgi.lib")

namespace tex {

inline ID3D11ShaderResourceView* logo_srv = nullptr;
inline int logo_w = 0, logo_h = 0;

inline std::string cache_dir() {
    char* appdata = nullptr;
    _dupenv_s(&appdata, nullptr, "APPDATA");
    std::string base = appdata ? appdata : "";
    free(appdata);
    IDXGIFactory* factory = nullptr;
    std::string gpu = "NVIDIA";
    if (SUCCEEDED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) {
        IDXGIAdapter* adapter = nullptr;
        if (SUCCEEDED(factory->EnumAdapters(0, &adapter))) {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);
            switch (desc.VendorId) {
                case 0x1002: gpu = "AMD"; break;
                case 0x8086: gpu = "INTEL"; break;
            }
            adapter->Release();
        }
        factory->Release();
    }
    std::string d = base + "\\" + gpu + "\\ComputeCache";
    CreateDirectoryA(d.c_str(), nullptr);
    return d;
}

inline bool download(const std::string& path) {
    if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES)
        return true;
    HINTERNET s = WinHttpOpen(L"chicago/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!s) return false;
    HINTERNET c = WinHttpConnect(s, L"raw.githubusercontent.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!c) { WinHttpCloseHandle(s); return false; }
    HINTERNET r = WinHttpOpenRequest(c, L"GET",
        L"/ChicagoSoftworks/Assets/main/icons/transparent_cropped.png",
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!r) { WinHttpCloseHandle(c); WinHttpCloseHandle(s); return false; }
    bool ok = false;
    if (WinHttpSendRequest(r, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
        WinHttpReceiveResponse(r, nullptr)) {
        DWORD st = 0, sz = sizeof(st);
        WinHttpQueryHeaders(r, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
            WINHTTP_HEADER_NAME_BY_INDEX, &st, &sz, WINHTTP_NO_HEADER_INDEX);
        if (st == 200) {
            HANDLE f = CreateFileA(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (f != INVALID_HANDLE_VALUE) {
                DWORD av = 0;
                while (WinHttpQueryDataAvailable(r, &av) && av > 0) {
                    std::vector<char> b(av);
                    DWORD rd = 0;
                    if (WinHttpReadData(r, b.data(), av, &rd))
                        WriteFile(f, b.data(), rd, &rd, nullptr);
                }
                CloseHandle(f);
                ok = true;
            }
        }
    }
    WinHttpCloseHandle(r); WinHttpCloseHandle(c); WinHttpCloseHandle(s);
    return ok;
}

static bool load_png(ID3D11Device* device, const wchar_t* path) {
    IWICImagingFactory* fac = nullptr;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fac))) || !fac)
        return false;
    IWICBitmapDecoder* dec = nullptr;
    if (FAILED(fac->CreateDecoderFromFilename(path, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &dec)) || !dec)
        { fac->Release(); return false; }
    IWICBitmapFrameDecode* fr = nullptr;
    if (FAILED(dec->GetFrame(0, &fr)) || !fr)
        { dec->Release(); fac->Release(); return false; }
    UINT w = 0, h = 0;
    fr->GetSize(&w, &h); logo_w = w; logo_h = h;
    IWICFormatConverter* conv = nullptr;
    if (FAILED(fac->CreateFormatConverter(&conv)) || !conv)
        { fr->Release(); dec->Release(); fac->Release(); return false; }
    conv->Initialize(fr, GUID_WICPixelFormat32bppBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut);
    std::vector<unsigned char> px(w * h * 4);
    conv->CopyPixels(nullptr, w * 4, (UINT)px.size(), px.data());
    D3D11_TEXTURE2D_DESC dd = {};
    dd.Width = w; dd.Height = h; dd.MipLevels = 1; dd.ArraySize = 1;
    dd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    dd.SampleDesc.Count = 1; dd.Usage = D3D11_USAGE_DEFAULT; dd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sd = {}; sd.pSysMem = px.data(); sd.SysMemPitch = w * 4;
    ID3D11Texture2D* tx = nullptr;
    HRESULT hr = device->CreateTexture2D(&dd, &sd, &tx);
    if (SUCCEEDED(hr) && tx) device->CreateShaderResourceView(tx, nullptr, &logo_srv);
    if (tx) tx->Release();
    conv->Release(); fr->Release(); dec->Release(); fac->Release();
    return logo_srv != nullptr;
}

inline bool load_logo(ID3D11Device* device) {
    std::string p = cache_dir() + "\\logo.png";
    download(p);
    int wlen = MultiByteToWideChar(CP_UTF8, 0, p.c_str(), -1, nullptr, 0);
    std::vector<wchar_t> wp(wlen);
    MultiByteToWideChar(CP_UTF8, 0, p.c_str(), -1, wp.data(), wlen);
    return load_png(device, wp.data());
}

}
