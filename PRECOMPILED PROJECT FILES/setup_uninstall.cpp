#include "modwin.h"
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include "mkisofs_exe.h"
#include "mkisofs_cygwin_dll.h"

// Swaps text pieces inside a string
void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
}

// Brings up a window for the user to pick a folder
std::string PickFolderDialog() {
    BROWSEINFOA bi = { 0 };
    bi.lpszTitle = "Select Folder";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != 0) {
        char path[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, path)) {
            CoTaskMemFree(pidl);
            return std::string(path);
        }
        CoTaskMemFree(pidl);
    }
    return "";
}

// Shows a window for selecting a file to open
bool OpenFileDialog(char* buffer, int bufferSize, const char* filter) {
    OPENFILENAMEA ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    if (GetOpenFileNameA(&ofn)) {
        strncpy_s(buffer, bufferSize, fileName, _TRUNCATE);
        return true;
    }
    return false;
}

// Shows a window for choosing where to save the ISO
bool SaveFileDialog(char* buffer, int bufferSize) {
    OPENFILENAMEA ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "ISO Files\0*.iso\0All Files\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "iso";
    if (GetSaveFileNameA(&ofn)) {
        strncpy_s(buffer, bufferSize, fileName, _TRUNCATE);
        return true;
    }
    return false;
}

// Looks to see if a file actually exists on the drive
bool FileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

// Checks if a folder path is valid and present
bool DirectoryExists(const std::string& dirName) {
    DWORD ftyp = GetFileAttributesA(dirName.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES) return false;
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;
    return false;
}

// Checks if the path is a drive root (e.g. C:\ or D:\)
bool IsDriveRoot(const std::string& path) {
    fs::path p = fs::path(path);
    p = p.lexically_normal();
    return (p == p.root_path());
}

// Verifies if the app is running with admin
bool IsUserAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) return false;
    if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) isAdmin = FALSE;
    FreeSid(adminGroup);
    return isAdmin == TRUE;
}

// Pulls the saved folder path from the registry
std::string GetRegistryInstallPath() {
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\MODWIN", 0, KEY_READ, &hKey) != ERROR_SUCCESS) return "";
    char value[MAX_PATH];
    DWORD bufferSize = sizeof(value);
    if (RegQueryValueExA(hKey, "InstallPath", 0, NULL, (LPBYTE)value, &bufferSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "";
    }
    RegCloseKey(hKey);
    return std::string(value);
}

// Saves the chosen folder path into the registry
void SetRegistryInstallPath(const std::string& path) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\MODWIN", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "InstallPath", 0, REG_SZ, (const BYTE*)path.c_str(), (DWORD)(path.size() + 1));
        RegCloseKey(hKey);
    }
}

// Checks if an image is currently opened for editing
bool IsWimMounted() {
    return DirectoryExists(g_InstallDir + "\\PATH\\Windows");
}

// Tests if the folder allows creating and deleting files
bool CanWriteToPath(const std::string& path) {
    try {
        if (path.empty()) return false;
        if (!DirectoryExists(path)) {
            fs::create_directories(path);
        }
        std::string testFile = path + "\\.write_test";
        std::ofstream f(testFile);
        if (f.is_open()) {
            f.close();
            fs::remove(testFile);
            return true;
        }
    }
    catch (...) {
        return false;
    }
    return false;
}

// Prepares the app folder when it starts for the first time
void SetupInstallDirectory() {
    g_InstallDir = GetRegistryInstallPath();

    // Clears out folder paths that are off limits
    if (!g_InstallDir.empty()) {
        if (g_InstallDir == "C:\\Users" || g_InstallDir == "C:\\Windows" || IsDriveRoot(g_InstallDir)) {
            g_InstallDir = "";
        }
        else if (!CanWriteToPath(g_InstallDir)) {
            g_InstallDir = "";
        }
    }

    // Asks the user to pick a home for the app files
    if (g_InstallDir.empty()) {
        MessageBoxA(NULL, "Welcome to MODWIN V7.\nPlease select a folder to install/store MODWIN files.", "First Run Setup", MB_OK);

        while (true) {
            std::string chosen = PickFolderDialog();
            if (chosen.empty()) {
                // Warns the user that they can't skip this part
                int ret = MessageBoxA(NULL, "No folder selected. You must select a location to continue.\n\nClick OK to select a folder.\nClick Cancel to exit MODWIN.", "Selection Required", MB_OKCANCEL | MB_ICONWARNING);

                // Quits the app if they hit cancel
                if (ret == IDCANCEL) {
                    exit(0);
                }
            }
            else {
                // Rejects drive roots like C:\ or D
                if (IsDriveRoot(chosen)) {
                    MessageBoxA(NULL, "You cannot install directly to the root of a drive (e.g. C:\\).\nPlease select or create a subfolder instead.", "Invalid Location", MB_OK | MB_ICONWARNING);
                    continue;
                }

                fs::path p = fs::path(chosen);
                std::string filename = p.filename().string();

                // Automatically names the folder 
                if (_stricmp(filename.c_str(), "MODWIN") != 0) {
                    p /= "MODWIN";
                }

                std::string candidate = p.string();

                if (CanWriteToPath(candidate)) {
                    g_InstallDir = candidate;
                    break;
                }
                else {
                    MessageBoxA(NULL, "The selected location is Read-Only or not accessible.\nPlease select a location on your local Hard Drive (C:).", "Invalid Location", MB_OK | MB_ICONERROR);
                }
            }
        }
        SetRegistryInstallPath(g_InstallDir);
    }
}

// Creates the internal folders and puts the tools in place
void BuildModwinFolder() {
    try {
        fs::create_directories(g_InstallDir);
        fs::create_directories(g_InstallDir + "\\BIN");
        fs::create_directories(g_InstallDir + "\\PATH");
        fs::create_directories(g_InstallDir + "\\BIN\\IMGUI");

        const fs::path binDir = g_InstallDir + "\\BIN";

        // Unpacks the ISO building tools into the BIN folder
        fs::path mkisofsDir = binDir / "MKISOFS";
        fs::create_directories(mkisofsDir);
        fs::path mkExePath = mkisofsDir / "mkisofs.exe";
        if (!fs::exists(mkExePath)) {
            std::ofstream mkOut(mkExePath, std::ios::binary);
            if (mkOut.is_open()) { mkOut.write(reinterpret_cast<const char*>(mkisofs_exe_data), mkisofs_exe_size); mkOut.close(); }
        }
        fs::path mkDllPath = mkisofsDir / "cygwin1.dll";
        if (!fs::exists(mkDllPath)) {
            std::ofstream dllOut(mkDllPath, std::ios::binary);
            if (dllOut.is_open()) { dllOut.write(reinterpret_cast<const char*>(mkisofs_cygwin_dll_data), mkisofs_cygwin_dll_size); dllOut.close(); }
        }
    }
    catch (const std::exception& e) {
        std::string msg = "Failed to create MODWIN directories at: " + g_InstallDir + "\n\nError: " + std::string(e.what());
        MessageBoxA(NULL, msg.c_str(), "Startup Error", MB_OK | MB_ICONERROR);
        exit(1);
    }
}

// Wipes the app and its files from the system
void Task_Uninstall() {
    // Asks one last time before deleting everything
    if (MessageBoxA(NULL, "Are you sure you want to completely remove MODWIN?\nThis will delete the MODWIN folder and all settings.", "Confirm Uninstall", MB_YESNO | MB_ICONQUESTION) != IDYES) {
        isTaskRunning = false;
        return;
    }

    myLog.AddLog("[INFO] Starting Uninstall Process...\n");

    // Refuses to run if the stored path looks dangerous or too short to be real
    if (g_InstallDir.length() < 10 || g_InstallDir == "C:\\" || g_InstallDir == "C:\\Windows" || g_InstallDir == "C:\\Users") {
        myLog.AddLog("[ERROR] Install path looks unsafe, refusing to delete: %s\n", g_InstallDir.c_str());
        MessageBoxA(NULL, "The install path looks invalid or dangerous.\nUninstall has been aborted for safety.", "Uninstall Aborted", MB_OK | MB_ICONERROR);
        isTaskRunning = false;
        return;
    }

    // Stops the uninstall if the path contains characters that could break the cleanup script
    if (g_InstallDir.find('"') != std::string::npos || g_InstallDir.find('\n') != std::string::npos) {
        myLog.AddLog("[ERROR] Install path contains unsafe characters. Aborting uninstall.\n");
        MessageBoxA(NULL, "The install path contains characters that are not allowed.\nUninstall has been aborted for safety.", "Uninstall Aborted", MB_OK | MB_ICONERROR);
        isTaskRunning = false;
        return;
    }

    // Tries to shut down any open images before deleting the folder
    bool unmountSuccess = false;
    while (!unmountSuccess) {
        myLog.AddLog("[INFO] Attempting to unmount and cleanup...\n");
        Task_Unmount();

        RunCommand("dism /Cleanup-Mountpoints");

        if (DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
            myLog.AddLog("[WARNING] Mount folder still contains data. Unmount may have failed.\n");
            int ret = MessageBoxA(NULL, "Failed to unmount the image properly.\nThe folder appears to be in use (Locked).\n\n1. Close any open folders inside MODWIN.\n2. Close any programs accessing the ISO.\n\nClick RETRY to force unmount again.\nClick CANCEL to abort uninstall.", "Unmount Error", MB_RETRYCANCEL | MB_ICONERROR);
            if (ret == IDCANCEL) {
                myLog.AddLog("[INFO] Uninstall aborted by user.\n");
                isTaskRunning = false;
                return;
            }
        }
        else {
            unmountSuccess = true;
        }
    }

    myLog.AddLog("[INFO] Deleting Registry Keys...\n");

    // Uses RegDeleteTreeA so subkeys get wiped too
    RegDeleteTreeA(HKEY_CURRENT_USER, "Software\\MODWIN");

    myLog.AddLog("[INFO] Scheduling final deletion...\n");

    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    std::string scriptPath = std::string(tempPath) + "modwin_cleanup.bat";

    // Writes a small script that deletes the folder after the app closes
    std::ofstream script(scriptPath);
    if (script.is_open()) {
        script << "@echo off\n";
        script << "timeout /t 3 /nobreak > nul\n";

        // Tracks how many times the delete has been attempted
        script << "set count=0\n";

        // Keeps trying to delete until it works or the attempt limit is hit
        script << ":retry\n";
        script << "rmdir /s /q \"" << g_InstallDir << "\"\n";
        script << "set /a count+=1\n";
        script << "if exist \"" << g_InstallDir << "\" (\n";
        script << "    if %count% lss 10 (\n";
        script << "        timeout /t 2 /nobreak > nul\n";
        script << "        goto retry\n";
        script << "    )\n";
        script << ")\n";

        script << "del \"%~f0\"\n";
        script.close();
        MessageBoxA(NULL, "MODWIN has been uninstalled.\n\nThe application will now close and the remaining files will be deleted automatically in a few seconds.", "Uninstall Complete", MB_OK | MB_ICONINFORMATION);

        // Runs the cleanup script
        ShellExecuteA(NULL, "open", scriptPath.c_str(), NULL, NULL, SW_HIDE);

        exit(0);
    }
    else {
        MessageBoxA(NULL, "Failed to create cleanup script. Please delete the MODWIN folder manually.", "Manual Cleanup Required", MB_OK | MB_ICONEXCLAMATION);
        exit(0);
    }
}

// Creates the right click tool for taking ownership of files
void Task_ManageContextTakeOwn(bool install) {
    std::string batPath = g_InstallDir + "\\BIN\\modwin_takeown.bat";
    std::string vbsPath = g_InstallDir + "\\BIN\\modwin_elevate.vbs";

    if (install) {
        myLog.AddLog("[INFO] Generating Ownership Scripts in BIN folder...\n");

        // Builds the script that resets file permissions
        std::ofstream bat(batPath);
        if (bat.is_open()) {
            bat << "@echo off\n";
            bat << "takeown /f \"%~1\" /r /d y >nul\n";
            bat << "icacls \"%~1\" /grant Everyone:F /t /c /q >nul\n";
            bat << "icacls \"%~1\" /grant Administrators:F /t /c /q >nul\n";
            bat.flush();
            bat.close();
        }

        // Creates a hidden script to run the command as an admin
        std::ofstream vbs(vbsPath);
        if (vbs.is_open()) {
            vbs << "Set objShell = CreateObject(\"Shell.Application\")\n";
            vbs << "If WScript.Arguments.Count >= 2 Then\n";
            vbs << "    qt = Chr(34)\n";
            vbs << "    strArgs = \"/c \" & qt & qt & WScript.Arguments(0) & qt & \" \" & qt & WScript.Arguments(1) & qt & qt\n";
            vbs << "    objShell.ShellExecute \"cmd.exe\", strArgs, \"\", \"runas\", 0\n";
            vbs << "End If\n";
            vbs.flush();
            vbs.close();
        }

        bool filesOk = true;
        std::ifstream bCheck(batPath); if (bCheck.peek() == std::ifstream::traits_type::eof()) filesOk = false;
        std::ifstream vCheck(vbsPath); if (vCheck.peek() == std::ifstream::traits_type::eof()) filesOk = false;

        if (!filesOk) {
            myLog.AddLog("[ERROR] Failed to write scripts. Check folder permissions.\n");
            isTaskRunning = false;
            return;
        }

        myLog.AddLog("[INFO] Adding Registry Keys...\n");

        std::string command = "wscript.exe \\\"" + vbsPath + "\\\" \\\"" + batPath + "\\\" \\\"%1\\\"";

        // Registers the tool so it appears on files
        RunCommand("reg add \"HKCR\\*\\shell\\TakeOwnership\" /d \"Take Ownership\" /f");
        RunCommand("reg add \"HKCR\\*\\shell\\TakeOwnership\" /v \"HasLUAShield\" /d \"\" /f");
        RunCommand("reg add \"HKCR\\*\\shell\\TakeOwnership\" /v \"NoWorkingDirectory\" /d \"\" /f");
        RunCommand("reg add \"HKCR\\*\\shell\\TakeOwnership\\command\" /ve /d \"" + command + "\" /f");

        // Registers the tool so it appears on folders
        RunCommand("reg add \"HKCR\\Directory\\shell\\TakeOwnership\" /d \"Take Ownership\" /f");
        RunCommand("reg add \"HKCR\\Directory\\shell\\TakeOwnership\" /v \"HasLUAShield\" /d \"\" /f");
        RunCommand("reg add \"HKCR\\Directory\\shell\\TakeOwnership\" /v \"NoWorkingDirectory\" /d \"\" /f");
        RunCommand("reg add \"HKCR\\Directory\\shell\\TakeOwnership\\command\" /ve /d \"" + command + "\" /f");

        myLog.AddLog("[SUCCESS] Installed. Right-click any folder -> 'Take Ownership'.\n");
    }
    else {
        myLog.AddLog("[INFO] Removing Registry Keys...\n");
        RunCommand("reg delete \"HKCR\\*\\shell\\TakeOwnership\" /f");
        RunCommand("reg delete \"HKCR\\Directory\\shell\\TakeOwnership\" /f");

        if (FileExists(batPath)) std::filesystem::remove(batPath);
        if (FileExists(vbsPath)) std::filesystem::remove(vbsPath);

        myLog.AddLog("[SUCCESS] Context Menu removed.\n");
    }
    isTaskRunning = false;
}
