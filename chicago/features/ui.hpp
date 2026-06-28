#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../deps/imgui/imgui.h"
#include "../deps/imgui/imgui_internal.h"

namespace ui {

// helpers
inline const char* label_display(const char* label) {
    const char* hh = strstr(label, "##");
    return hh ? (hh == label ? "" : label) : label;
}

inline float label_width(const char* label) {
    const char* d = label_display(label);
    return d[0] ? ImGui::CalcTextSize(d, 0, true).x + ImGui::GetStyle().ItemInnerSpacing.x : 0;
}

inline ImDrawList* dl() { return ImGui::GetWindowDrawList(); }

inline bool checkbox(const char* label, bool* v) {
    ImGuiWindow* win = ImGui::GetCurrentWindow();
    if (win->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& s = g.Style;
    const ImGuiID id = win->GetID(label);
    const float h = ImGui::GetFrameHeight();
    ImVec2 pos = win->DC.CursorPos;
    const char* disp = label_display(label);
    float tw = disp[0] ? ImGui::CalcTextSize(disp, 0, true).x : 0;
    float sq = h - s.FramePadding.y * 2 - 2;
    ImRect bb(pos, ImVec2(pos.x + sq + tw + s.ItemInnerSpacing.x, pos.y + h));
    ImGui::ItemSize(bb, s.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;
    bool hovered = false, held = false;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    if (pressed) *v = !*v;

    ImVec2 cp(pos.x, pos.y + s.FramePadding.y + 1);

    ImU32 bg = ImGui::GetColorU32(
        *v ? ImVec4(0.25f,0.25f,0.25f,0.80f) :
        hovered ? ImVec4(0.25f,0.25f,0.25f,0.70f) : ImVec4(0.18f,0.18f,0.18f,0.60f));
    dl()->AddRectFilled(cp, ImVec2(cp.x+sq, cp.y+sq), bg, 3.f);

    if (*v) {
        ImU32 ck = ImGui::GetColorU32(ImVec4(0.75f,0.75f,0.78f,1.f));
        float in = sq * 0.25f;
        dl()->AddRectFilled(ImVec2(cp.x+in, cp.y+in), ImVec2(cp.x+sq-in, cp.y+sq-in), ck, 2.f);
    }

    if (disp[0])
        dl()->AddText(ImVec2(cp.x+sq+s.ItemInnerSpacing.x, pos.y+s.FramePadding.y),
            ImGui::GetColorU32(ImVec4(1,1,1,1)), disp);
    return pressed;
}

inline bool combo(const char* label, int* current_item, const char* items) {
    ImGuiWindow* win = ImGui::GetCurrentWindow();
    if (win->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& s = g.Style;
    const ImGuiID id = win->GetID(label);

    const char* preview = "";
    if (*current_item >= 0) {
        const char* p = items;
        for (int i = 0; i <= *current_item; i++) {
            preview = p;
            p += strlen(p) + 1;
        }
    }

    const char* disp = label_display(label);
    float h = ImGui::GetFrameHeight();
    float lw = label_width(label);
    ImVec2 ps = ImGui::CalcTextSize(preview, 0, true);
    float arrow_w = h * 0.55f;
    float cw = ps.x + arrow_w + s.FramePadding.x * 3;
    ImVec2 pos = win->DC.CursorPos;

    // draw label before widget
    if (disp[0]) {
        dl()->AddText(ImVec2(pos.x, pos.y + s.FramePadding.y),
            ImGui::GetColorU32(ImVec4(1,1,1,0.7f)), disp);
        pos.x += lw;
    }

    ImRect bb(pos, ImVec2(pos.x + cw, pos.y + h));
    ImGui::ItemSize(bb, s.FramePadding.y);
    ImRect total_bb = disp[0] ? ImRect(ImVec2(win->DC.CursorPos.x, pos.y), bb.Max) : bb;
    if (!ImGui::ItemAdd(total_bb, id)) return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImU32 bg = ImGui::GetColorU32(hovered ? ImVec4(0.28f,0.28f,0.28f,0.80f) : ImVec4(0.20f,0.20f,0.20f,0.70f));
    dl()->AddRectFilled(bb.Min, bb.Max, bg, 3.f);

    dl()->AddText(ImVec2(bb.Min.x + s.FramePadding.x, bb.Min.y + s.FramePadding.y),
        ImGui::GetColorU32(ImVec4(1,1,1,0.9f)), preview);

    // arrow
    ImVec2 ac(bb.Max.x - arrow_w * 0.5f - s.FramePadding.x, bb.Min.y + h * 0.5f);
    float asz = h * 0.12f;
    dl()->AddTriangleFilled(
        ImVec2(ac.x - asz, ac.y - asz * 0.5f),
        ImVec2(ac.x + asz, ac.y - asz * 0.5f),
        ImVec2(ac.x, ac.y + asz * 0.6f),
        ImGui::GetColorU32(ImVec4(1,1,1,0.4f)));

    // calculate popup item width from the combo width
    float popup_w = cw > 50.f ? cw : 150.f;

    ImGui::PushID((int)id);
    if (pressed) ImGui::OpenPopup("##pop");
    if (ImGui::BeginPopup("##pop")) {
        ImDrawList* pd = ImGui::GetWindowDrawList();
        const char* p = items;
        for (int i = 0; *p; i++) {
            ImGui::PushID(i);
            float fh = ImGui::GetFrameHeight();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImRect ir(pos, ImVec2(pos.x + popup_w, pos.y + fh));
            ImGui::ItemSize(ir);
            ImGuiID iid = ImGui::GetID("##i");
            if (ImGui::ItemAdd(ir, iid)) {
                bool ih, iheld;
                bool ip = ImGui::ButtonBehavior(ir, iid, &ih, &iheld);
                bool sel = (i == *current_item);
                ImU32 ibg = ih ? IM_COL32(45,45,45,230) :
                            sel ? IM_COL32(38,38,38,230) : IM_COL32(0,0,0,0);
                if (ibg) pd->AddRectFilled(ir.Min, ir.Max, ibg);
                pd->AddText(ImVec2(pos.x + 8, pos.y + (fh - ImGui::CalcTextSize(p).y) * 0.5f),
                    IM_COL32(255,255,255, sel ? 255 : 180), p);
                if (ip) { *current_item = i; ImGui::CloseCurrentPopup(); }
            }
            ImGui::PopID();
            p += strlen(p) + 1;
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    return pressed;
}

inline bool begin_keybind(const char* label, int* key) {
    ImGuiWindow* win = ImGui::GetCurrentWindow();
    if (win->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& s = g.Style;
    const ImGuiID id = win->GetID(label);
    const char* disp = label_display(label);
    float h = ImGui::GetFrameHeight();
    float lw = label_width(label);

    const char* key_names[] = { "RMB", "LMB", "XMB1", "XMB2" };
    int key_vals[] = { VK_RBUTTON, VK_LBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
    const char* preview = "key";
    for (int i = 0; i < 4; i++) {
        if (*key == key_vals[i]) { preview = key_names[i]; break; }
    }

    ImVec2 ps = ImGui::CalcTextSize(preview, 0, true);
    float arrow_w = h * 0.55f;
    float cw = ps.x + arrow_w + s.FramePadding.x * 3;
    ImVec2 pos = win->DC.CursorPos;

    if (disp[0]) {
        dl()->AddText(ImVec2(pos.x, pos.y + s.FramePadding.y),
            ImGui::GetColorU32(ImVec4(1,1,1,0.7f)), disp);
        pos.x += lw;
    }

    ImRect bb(pos, ImVec2(pos.x + cw, pos.y + h));
    ImGui::ItemSize(bb, s.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImU32 bg = ImGui::GetColorU32(hovered ? ImVec4(0.28f,0.28f,0.28f,0.80f) : ImVec4(0.20f,0.20f,0.20f,0.70f));
    dl()->AddRectFilled(bb.Min, bb.Max, bg, 3.f);
    dl()->AddText(ImVec2(bb.Min.x + s.FramePadding.x, bb.Min.y + s.FramePadding.y),
        ImGui::GetColorU32(ImVec4(1,1,1,0.9f)), preview);

    ImVec2 ac(bb.Max.x - arrow_w * 0.5f - s.FramePadding.x, bb.Min.y + h * 0.5f);
    float asz = h * 0.12f;
    dl()->AddTriangleFilled(
        ImVec2(ac.x - asz, ac.y - asz * 0.5f),
        ImVec2(ac.x + asz, ac.y - asz * 0.5f),
        ImVec2(ac.x, ac.y + asz * 0.6f),
        ImGui::GetColorU32(ImVec4(1,1,1,0.4f)));

    float popup_w = cw > 50.f ? cw : 150.f;
    bool changed = false;
    ImGui::PushID((int)id);
    if (pressed) ImGui::OpenPopup("##pop");
    if (ImGui::BeginPopup("##pop")) {
        ImDrawList* pd = ImGui::GetWindowDrawList();
        for (int i = 0; i < 4; i++) {
            ImGui::PushID(i);
            float fh = ImGui::GetFrameHeight();
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImRect ir(pos, ImVec2(pos.x + popup_w, pos.y + fh));
            ImGui::ItemSize(ir);
            ImGuiID iid = ImGui::GetID("##k");
            if (ImGui::ItemAdd(ir, iid)) {
                bool ih, iheld;
                bool ip = ImGui::ButtonBehavior(ir, iid, &ih, &iheld);
                bool sel = (*key == key_vals[i]);
                ImU32 ibg = ih ? IM_COL32(45,45,45,230) :
                            sel ? IM_COL32(38,38,38,230) : IM_COL32(0,0,0,0);
                if (ibg) pd->AddRectFilled(ir.Min, ir.Max, ibg);
                pd->AddText(ImVec2(pos.x + 8, pos.y + (fh - ImGui::CalcTextSize(key_names[i]).y) * 0.5f),
                    IM_COL32(255,255,255, sel ? 255 : 180), key_names[i]);
                if (ip) { *key = key_vals[i]; changed = true; ImGui::CloseCurrentPopup(); }
            }
            ImGui::PopID();
        }
        ImGui::EndPopup();
    }
    ImGui::PopID();

    return changed;
}

inline bool slider_float(const char* label, float* v, float v_min, float v_max, const char* fmt = "%.0f", float width = 0.f) {
    ImGuiWindow* win = ImGui::GetCurrentWindow();
    if (win->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& s = g.Style;
    const ImGuiID id = win->GetID(label);
    const float h = ImGui::GetFrameHeight();
    ImVec2 pos = win->DC.CursorPos;

    char buf[64];
    ImFormatString(buf, sizeof(buf), fmt, *v);
    float tw = ImGui::CalcTextSize(buf, 0, true).x;
    float w = width > 0.f ? width : tw + s.FramePadding.x * 4 + 20.f;
    if (w < 60.f) w = 60.f;

    const char* disp = label_display(label);
    float lw = label_width(label);

    if (disp[0]) {
        dl()->AddText(ImVec2(pos.x, pos.y + s.FramePadding.y),
            ImGui::GetColorU32(ImVec4(1,1,1,0.7f)), disp);
        pos.x += lw;
    }

    ImRect bb(pos, ImVec2(pos.x + w, pos.y + h));
    ImGui::ItemSize(bb, s.FramePadding.y);
    ImRect total_bb = disp[0] ? ImRect(ImVec2(win->DC.CursorPos.x, pos.y), bb.Max) : bb;
    if (!ImGui::ItemAdd(total_bb, id)) return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

    ImU32 bg = ImGui::GetColorU32(hovered ? ImVec4(0.25f,0.25f,0.25f,0.70f) : ImVec4(0.18f,0.18f,0.18f,0.60f));
    dl()->AddRectFilled(bb.Min, bb.Max, bg, 3.f);

    bool changed = false;
    if (held) {
        if (g.ActiveIdIsJustActivated) {
            float rel = (g.IO.MousePos.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
            if (rel < 0.f) rel = 0.f;
            if (rel > 1.f) rel = 1.f;
            *v = v_min + rel * (v_max - v_min);
        } else {
            *v += g.IO.MouseDelta.x * (v_max - v_min) * 0.005f;
        }
        changed = true;
    }

    if (*v < v_min) *v = v_min;
    if (*v > v_max) *v = v_max;

    float fill = (*v - v_min) / (v_max - v_min);
    if (fill < 0.f) fill = 0.f;
    if (fill > 1.f) fill = 1.f;
    if (fill > 0.01f) {
        ImU32 fc = ImGui::GetColorU32(ImVec4(0.32f,0.32f,0.32f,0.70f));
        dl()->AddRectFilled(bb.Min, ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x) * fill, bb.Max.y), fc, 3.f);
    }

    ImFormatString(buf, sizeof(buf), fmt, *v);
    dl()->AddText(ImVec2(bb.Min.x + s.FramePadding.x, bb.Min.y + s.FramePadding.y),
        ImGui::GetColorU32(ImVec4(1,1,1,0.9f)), buf);

    return changed;
}

// HSV conversion helpers
inline void hsv_to_rgb(float h, float s, float v, float& r, float& g, float& b) {
    float c = v * s;
    float hp = h * 6.f;
    float x = c * (1.f - fabsf(fmodf(hp, 2.f) - 1.f));
    float m = v - c;
    int i = ((int)hp) % 6;
    float r1 = 0, g1 = 0, b1 = 0;
    switch (i) {
        case 0: r1=c; g1=x; break;
        case 1: r1=x; g1=c; break;
        case 2: g1=c; b1=x; break;
        case 3: g1=x; b1=c; break;
        case 4: r1=x; b1=c; break;
        case 5: r1=c; b1=x; break;
    }
    r = r1 + m; g = g1 + m; b = b1 + m;
}

inline void rgb_to_hsv(float r, float g, float b, float& h, float& s, float& v) {
    float mx = fmaxf(r, fmaxf(g, b));
    float mn = fminf(r, fminf(g, b));
    float d = mx - mn;
    v = mx;
    if (d < 0.0001f) { h = 0.f; s = 0.f; return; }
    s = d / mx;
    if (mx == r) h = fmodf((g - b) / d, 6.f);
    else if (mx == g) h = (b - r) / d + 2.f;
    else h = (r - g) / d + 4.f;
    h /= 6.f;
    if (h < 0) h += 1.f;
}

inline bool color_edit4(const char* label, float col[4], bool alpha = true) {
    ImGuiWindow* win = ImGui::GetCurrentWindow();
    if (win->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& s = g.Style;
    const ImGuiID id = win->GetID(label);
    const float h = ImGui::GetFrameHeight();
    ImVec2 pos = win->DC.CursorPos;
    const char* disp = label_display(label);
    float tw = disp[0] ? ImGui::CalcTextSize(disp, 0, true).x : 0;
    float sq = h - s.FramePadding.y * 2 - 2;
    float total_w = sq + (disp[0] ? tw + s.ItemInnerSpacing.x : 0);

    ImRect bb(pos, ImVec2(pos.x + total_w, pos.y + h));
    ImGui::ItemSize(bb, s.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    ImRect sq_bb(ImVec2(pos.x, pos.y + s.FramePadding.y + 1),
        ImVec2(pos.x + sq, pos.y + sq + s.FramePadding.y + 1));
    bool pressed = ImGui::ButtonBehavior(sq_bb, id, &hovered, &held);

    ImU32 color = ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], col[3]));
    dl()->AddRectFilled(sq_bb.Min, sq_bb.Max, color, 3.f);

    if (disp[0])
        dl()->AddText(ImVec2(pos.x + sq + s.ItemInnerSpacing.x, pos.y + s.FramePadding.y),
            ImGui::GetColorU32(ImVec4(1,1,1,1)), disp);

    ImGui::PushID((int)id);
    if (pressed) ImGui::OpenPopup("##col");
    if (ImGui::BeginPopup("##col")) {
        float r = col[0], g2 = col[1], b = col[2], a = col[3];
        float hu, sa, va;
        rgb_to_hsv(r, g2, b, hu, sa, va);
        ImDrawList* pd = ImGui::GetWindowDrawList();
        ImVec2 cp = ImGui::GetCursorScreenPos();
        float pw = 200.f;

        // preview
        ImRect prev(cp, ImVec2(cp.x + pw, cp.y + 24));
        pd->AddRectFilled(prev.Min, prev.Max, IM_COL32((int)(r*255),(int)(g2*255),(int)(b*255),(int)(a*255)));
        ImGui::Dummy(ImVec2(pw, 26));

        // hue bar
        ImVec2 hb_min(cp.x, cp.y + 30);
        ImVec2 hb_max(cp.x + pw, cp.y + 42);
        for (int i = 0; i < 256; i++) {
            float hh = i / 255.f, _hr, _hg, _hb;
            hsv_to_rgb(hh, 1.f, 1.f, _hr, _hg, _hb);
            float x = hb_min.x + (hb_max.x - hb_min.x) * (i / 255.f);
            pd->AddRectFilled(ImVec2(x, hb_min.y), ImVec2(x + 1, hb_max.y), IM_COL32((int)(_hr*255),(int)(_hg*255),(int)(_hb*255),255));
        }
        float hhx = hb_min.x + hu * pw;
        pd->AddRect(ImVec2(hhx - 1, hb_min.y - 1), ImVec2(hhx + 2, hb_max.y + 1), IM_COL32(255,255,255,200));

        // SV square
        float sv_sz = pw;
        ImVec2 sv_min(cp.x, cp.y + 46);
        ImVec2 sv_max(cp.x + sv_sz, cp.y + 46 + sv_sz);
        float hr, hg, hb;
        hsv_to_rgb(hu, 1.f, 1.f, hr, hg, hb);
        ImU32 hue_c = IM_COL32((int)(hr*255),(int)(hg*255),(int)(hb*255),255);
        pd->AddRectFilledMultiColor(sv_min, sv_max, IM_COL32(255,255,255,255), hue_c, hue_c, IM_COL32(255,255,255,255));
        pd->AddRectFilledMultiColor(sv_min, sv_max, IM_COL32(0,0,0,0), IM_COL32(0,0,0,0), IM_COL32(0,0,0,255), IM_COL32(0,0,0,255));

        // SV handle
        float svx = sv_min.x + sa * sv_sz;
        float svy = sv_min.y + (1.f - va) * sv_sz;
        pd->AddCircleFilled(ImVec2(svx, svy), 5.f, IM_COL32(255,255,255,255));
        pd->AddCircle(ImVec2(svx, svy), 5.f, IM_COL32(0,0,0,120));

        // hue interaction
        ImGui::SetCursorScreenPos(hb_min);
        ImGui::InvisibleButton("##hue", ImVec2(pw, 12));
        if (ImGui::IsItemActive()) {
            float mpx = (ImGui::GetIO().MousePos.x - hb_min.x) / pw;
            if (mpx < 0) mpx = 0;
            if (mpx > 1) mpx = 1;
            hu = mpx;
            hsv_to_rgb(hu, sa, va, r, g2, b);
        }

        // SV interaction
        ImGui::SetCursorScreenPos(sv_min);
        ImGui::InvisibleButton("##sv", ImVec2(sv_sz, sv_sz));
        if (ImGui::IsItemActive()) {
            ImVec2 mp = ImGui::GetIO().MousePos;
            sa = (mp.x - sv_min.x) / sv_sz;
            va = 1.f - (mp.y - sv_min.y) / sv_sz;
            if (sa < 0) sa = 0;
            if (sa > 1) sa = 1;
            if (va < 0) va = 0;
            if (va > 1) va = 1;
            hsv_to_rgb(hu, sa, va, r, g2, b);
        }

        ImGui::SetCursorScreenPos(ImVec2(cp.x, sv_max.y + 6));

        // sliders
        slider_float("R", &r, 0.f, 1.f, "%.2f");
        slider_float("G", &g2, 0.f, 1.f, "%.2f");
        slider_float("B", &b, 0.f, 1.f, "%.2f");
        if (alpha) slider_float("A", &a, 0.f, 1.f, "%.2f");

        // sync back to hsv in case sliders were used
        rgb_to_hsv(r, g2, b, hu, sa, va);

        col[0] = r; col[1] = g2; col[2] = b; col[3] = a;
        ImGui::EndPopup();
    }
    ImGui::PopID();

    return pressed;}

inline bool color_edit3(const char* label, float col[3]) {
    float c4[4] = { col[0], col[1], col[2], 1.f };
    bool r = color_edit4(label, c4, false);
    col[0] = c4[0]; col[1] = c4[1]; col[2] = c4[2];
    return r;
}

inline void section(const char* name, float alpha = 0.5f) {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.f, 1.f, 1.f, alpha), name);
    ImGui::Separator();
}

}
