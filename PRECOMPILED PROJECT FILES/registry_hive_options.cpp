#include "modwin.h"
#include <iostream>

// Dropdown menu options for the Registry tool
int selectedHiveIndex = 0;
const char* hiveList[] = { "SOFTWARE", "SYSTEM", "SAM", "SECURITY", "DEFAULT", "NTUSER.DAT (Default User)" };
int hiveList_Size = sizeof(hiveList) / sizeof(hiveList[0]);

// Mounts the user specified registry hive from the WIM so the user can edit it manually
void Task_Registry() {
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
        myLog.AddLog("[ERROR] WIM is not mounted.\n");
        isTaskRunning = false;
        return;
    }

    std::string hiveFile;
    std::string mountKey = "OFFLINE";
    std::string mountPath = g_InstallDir + "\\PATH";

    // Map the dropdown selection to the actual file path on disk
    switch (selectedHiveIndex) {
    case 0: hiveFile = mountPath + "\\Windows\\System32\\config\\SOFTWARE"; break;
    case 1: hiveFile = mountPath + "\\Windows\\System32\\config\\SYSTEM"; break;
    case 2: hiveFile = mountPath + "\\Windows\\System32\\config\\SAM"; break;
    case 3: hiveFile = mountPath + "\\Windows\\System32\\config\\SECURITY"; break;
    case 4: hiveFile = mountPath + "\\Windows\\System32\\config\\DEFAULT"; break;
    case 5: hiveFile = mountPath + "\\Users\\Default\\NTUSER.DAT"; break;
    default: hiveFile = mountPath + "\\Windows\\System32\\config\\SOFTWARE"; break;
    }

    if (!FileExists(hiveFile)) {
        myLog.AddLog("[ERROR] Hive file not found: %s\n", hiveFile.c_str());
        isTaskRunning = false;
        return;
    }

    // Load the file into the host registry as a temporary key
    myLog.AddLog("[INFO] Loading Hive: %s...\n", mountKey.c_str());
    std::string loadCmd = "reg load HKLM\\" + mountKey + " \"" + hiveFile + "\"";
    RunCommand(loadCmd);

    if (!stopRequested) {
        // Force Regedit to open exactly where the user wants
        SetRegeditLastKey(mountKey);

        myLog.AddLog("[INFO] Opening Registry Editor...\n");
        myLog.AddLog("[TIP] Look for HKEY_LOCAL_MACHINE\\%s\n", mountKey.c_str());
        myLog.AddLog("[WAITING] Close Regedit to save changes and unload hive...\n");
        RunCommand("start /wait regedit");

        // Commit changes and detach the file
        myLog.AddLog("[INFO] Regedit closed. Unloading hive...\n");
        std::string unloadCmd = "reg unload HKLM\\" + mountKey;
        RunCommand(unloadCmd);
        myLog.AddLog("[SUCCESS] Hive Unloaded.\n");
    }
    isTaskRunning = false;
}

// Sets the 'LastKey' registry value so Regedit jumps to the mounted hive automatically
void SetRegeditLastKey(std::string keyName) {
    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        std::string val = "Computer\\HKEY_LOCAL_MACHINE\\" + keyName;
        RegSetValueExA(hKey, "LastKey", 0, REG_SZ, (const BYTE*)val.c_str(), (DWORD)(val.size() + 1));
        RegCloseKey(hKey);
    }
}