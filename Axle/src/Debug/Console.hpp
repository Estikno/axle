#pragma once

#include "axpch.hpp"

#include "imgui.h"
#include "Core/Logger/Log.hpp"

#include <sstream>

#define IM_COUNTOF(_ARR) ((size_t) (sizeof(_ARR) / sizeof(*(_ARR))))

namespace Axle::Debug {
    struct DebugConsole {
        char InputBuf[256];
        ImVector<char*> Items;
        ImVector<const char*> Commands;
        ImVector<char*> History;
        int HistoryPos; // -1: new line, 0..History.Size-1 browsing history.
        ImGuiTextFilter Filter;
        bool AutoScroll;
        bool ScrollToBottom;
        bool Open = true;

        void Init();

        void Destroy() {
            ClearLog();
            for (int i = 0; i < History.Size; i++)
                ImGui::MemFree(History[i]);
        }

        // Portable helpers
        static int Stricmp(const char* s1, const char* s2) {
            int d;
            while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
                s1++;
                s2++;
            }
            return d;
        }
        static int Strnicmp(const char* s1, const char* s2, int n) {
            int d = 0;
            while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) {
                s1++;
                s2++;
                n--;
            }
            return d;
        }
        static char* Strdup(const char* s) {
            IM_ASSERT(s);
            size_t len = strlen(s) + 1;
            void* buf = ImGui::MemAlloc(len);
            IM_ASSERT(buf);
            return (char*) memcpy(buf, (const void*) s, len);
        }
        static void Strtrim(char* s) {
            char* str_end = s + strlen(s);
            while (str_end > s && str_end[-1] == ' ')
                str_end--;
            *str_end = 0;
        }

        void ClearLog() {
            for (int i = 0; i < Items.Size; i++)
                ImGui::MemFree(Items[i]);
            Items.clear();
        }

        void AddLog(const char* fmt, ...) IM_FMTARGS(2) {
            // FIXME-OPT
            char buf[1024];
            va_list args;
            va_start(args, fmt);
            vsnprintf(buf, IM_COUNTOF(buf), fmt, args);
            buf[IM_COUNTOF(buf) - 1] = 0;
            va_end(args);
            Items.push_back(Strdup(buf));
        }

        void Draw(const char* title, bool* p_open);

        void ExecCommand(const char* command_line);

        // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
        static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
            DebugConsole* console = (DebugConsole*) data->UserData;
            AX_CORE_ERROR(LogChannel::Debug, "CALLBACK");
            return console->TextEditCallback(data);
        }

        int TextEditCallback(ImGuiInputTextCallbackData* data);
    };
} // namespace Axle::Debug
