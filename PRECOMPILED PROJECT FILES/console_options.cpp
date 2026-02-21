#include "modwin.h"
#include <iostream>
#include <cstdio>
#include <vector>

// Sets log defaults and adds the startup message
AppLog::AppLog() {
    AutoScroll = true;
    ScrollToBottom = false;
    Lines.push_back("--- MODWIN Log Started ---");
}

// Deletes all entries from the log history
void AppLog::Clear() {
    std::lock_guard<std::mutex> lock(logMutex);
    Lines.clear();
}

// Formats text input and handles line breaks for the UI console
void AppLog::AddLog(const char* fmt, ...) {
    std::lock_guard<std::mutex> lock(logMutex);

    va_list args;
    va_start(args, fmt);
    char tempBuf[1024];
    vsnprintf(tempBuf, 1024, fmt, args);
    va_end(args);

    std::string s = tempBuf;
    static std::string pendingLine = "";

    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i];

        if (c == '\r') {
            if (i + 1 < s.length() && s[i + 1] == '\n') continue;
            if (!Lines.empty() && pendingLine.empty()) {
                Lines.pop_back();
            }
            pendingLine.clear();
        }
        else if (c == '\n') {
            Lines.push_back(pendingLine);
            pendingLine.clear();
            if (AutoScroll) ScrollToBottom = true;
        }
        else {
            pendingLine += c;
        }
    }

    if (!pendingLine.empty()) {
        Lines.push_back(pendingLine);
        pendingLine.clear();
        if (AutoScroll) ScrollToBottom = true;
    }
}

// Runs background commands hidden from the user while capturing output
void ExecuteSilent(std::string cmd, bool outputToLog, std::string* outputStore) {
    // Blocks normal commands if a stop is requested but allows cleanup
    bool isEmergencyCmd = (cmd.find("taskkill") != std::string::npos || cmd.find("Cleanup-Mountpoints") != std::string::npos);

    if (stopRequested && !isEmergencyCmd) return;

    if (outputToLog) myLog.AddLog("[CMD] %s\n", cmd.c_str());

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    HANDLE hRead, hWrite;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return;
    SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;
    si.wShowWindow = SW_HIDE;

    ZeroMemory(&pi, sizeof(pi));

    std::string fullCmd = "cmd.exe /C \"" + cmd + "\"";
    std::vector<char> cmdBuffer(fullCmd.begin(), fullCmd.end());
    cmdBuffer.push_back(0);

    if (!CreateProcessA(NULL, cmdBuffer.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hWrite);
        CloseHandle(hRead);
        if (outputToLog) myLog.AddLog("[ERROR] Failed to start command process.\n");
        return;
    }

    CloseHandle(hWrite);

    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        if (stopRequested && !isEmergencyCmd) break;

        buffer[bytesRead] = 0;
        if (outputToLog) myLog.AddLog("%s", buffer);
        if (outputStore) outputStore->append(buffer);
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hRead);
}

// Executes a command and sends the output directly to the GUI log
void RunCommand(std::string cmd) {
    ExecuteSilent(cmd, true, nullptr);
}

// Executes a command and returns the output as a string
std::string RunCommandCapture(std::string cmd) {
    std::string result;
    ExecuteSilent(cmd, false, &result);
    return result;
}

// Renders the log window and stop operation modal
void AppLog::DrawContent() {
    if (ImGui::Button("Options")) ImGui::OpenPopup("Options");
    ImGui::SameLine();
    if (ImGui::Button("Copy All")) {
        std::string fullLog;
        std::lock_guard<std::mutex> lock(logMutex);
        for (const auto& line : Lines) fullLog += line + "\n";
        ImGui::SetClipboardText(fullLog.c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Log")) Clear();

    ImGui::SameLine();
    if (isTaskRunning) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.05f, 0.05f, 1.0f));
        if (ImGui::Button("STOP OPERATION")) {
            ImGui::OpenPopup("StopConfirm");
        }
        ImGui::PopStyleColor();
    }
    else {
        ImGui::BeginDisabled();
        ImGui::Button("STOP OPERATION");
        ImGui::EndDisabled();
    }

    // Modal to confirm killing active background processes
    if (ImGui::BeginPopupModal("StopConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to forcibly stop the current operation?\n\nThis will kill background processes (dism, mkisofs) and clear locks.");
        ImGui::Separator();

        if (ImGui::Button("YES, STOP IT", ImVec2(120, 0))) {
            stopRequested = true;

            RunCommand("taskkill /F /IM dism.exe");
            RunCommand("taskkill /F /IM mkisofs.exe");
            RunCommand("taskkill /F /IM oscdimg.exe");

            AddLog("[ALERT] Stopping... Running mountpoint cleanup to unlock folders.\n");
            ExecuteSilent("dism /Cleanup-Mountpoints", true, nullptr);

            isTaskRunning = false;
            AddLog("[SUCCESS] OPERATION ABORTED BY USER. Files should now be unlocked.\n");
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-scroll", &AutoScroll);
        ImGui::EndPopup();
    }

    ImGui::Separator();
    ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(logMutex);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
    for (const auto& line : Lines) {
        ImGui::TextUnformatted(line.c_str());
    }
    ImGui::PopStyleVar();

    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Clear")) Clear();
        if (ImGui::MenuItem("Copy All")) {
            std::string fullLog;
            for (const auto& line : Lines) fullLog += line + "\n";
            ImGui::SetClipboardText(fullLog.c_str());
        }
        ImGui::EndPopup();
    }

    if (AutoScroll && ScrollToBottom) {
        ImGui::SetScrollHereY(1.0f);
        ScrollToBottom = false;
    }

    ImGui::EndChild();
}