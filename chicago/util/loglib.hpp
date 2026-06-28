#pragma once
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <conio.h>

namespace loglib {

inline HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

inline void set_color(WORD color) {
    SetConsoleTextAttribute(hOut, color);
}

inline WORD cons_log_color = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
inline WORD cons_arrow_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
inline WORD cons_text_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
inline WORD cons_err_color = FOREGROUND_RED | FOREGROUND_INTENSITY;
inline WORD cons_pau_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
inline WORD cons_gpu_color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

inline void log(const char* fmt, ...) {
    set_color(cons_log_color); printf("log");
    set_color(cons_arrow_color); printf(" -> ");
    set_color(cons_text_color);
    va_list args; va_start(args, fmt); vprintf(fmt, args); va_end(args);
}

inline void err(const char* fmt, ...) {
    set_color(cons_err_color); printf("err");
    set_color(cons_arrow_color); printf(" -> ");
    set_color(cons_text_color);
    va_list args; va_start(args, fmt); vprintf(fmt, args); va_end(args);
}

inline void gpu(const char* fmt, ...) {
    set_color(cons_gpu_color); printf("gpu");
    set_color(cons_arrow_color); printf(" -> ");
    set_color(cons_text_color);
    va_list args; va_start(args, fmt); vprintf(fmt, args); va_end(args);
}

inline void pause() {
    set_color(cons_pau_color); printf("pau");
    set_color(cons_arrow_color); printf(" -> ");
    set_color(cons_text_color); printf("press any key to quit");
    set_color(cons_text_color);
    int _ = _getch(); (void)_;
}

}
