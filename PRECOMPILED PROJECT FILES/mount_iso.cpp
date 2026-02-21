#include "modwin.h"

// Opens the user selected image file so the contents can be edited
void Task_Mount() {
    std::string wimFile = g_InstallDir + "\\install.wim";
    std::string mountDir = g_InstallDir + "\\PATH";

    // Checks for the image file before starting the mount process
    if (!FileExists(wimFile)) {
        myLog.AddLog("[ERROR] install.wim not found at: %s\n", wimFile.c_str());
        isTaskRunning = false;
        return;
    }

    // Creates the folder where the image files will appear
    if (!DirectoryExists(mountDir)) {
        myLog.AddLog("[INFO] Creating mount directory: %s\n", mountDir.c_str());
        try {
            fs::create_directories(mountDir);
        }
        catch (const std::exception& e) {
            myLog.AddLog("[ERROR] Failed to create mount directory: %s\n", e.what());
            isTaskRunning = false;
            return;
        }
    }

    myLog.AddLog("[INFO] Mounting WIM...\n");
    std::string cmd = "dism.exe /mount-wim /wimfile:\"" + wimFile + "\" /mountdir:\"" + mountDir + "\" /index:1";
    RunCommand(cmd);

    if (!stopRequested) myLog.AddLog("[INFO] Mount Operation Finished.\n");
    isTaskRunning = false;
}

// Closes the image and either saves or throws away the changes
void Task_Unmount() {
    std::string mountDir = g_InstallDir + "\\PATH";

    if (unmount_SaveChanges) {
        myLog.AddLog("[INFO] Attempting to unmount and saving changes to WIM...\n");
        std::string cmd = "dism /Unmount-Image /MountDir:\"" + mountDir + "\" /Commit";
        RunCommand(cmd);
    }
    else {
        myLog.AddLog("[INFO] Attempting to force unmount (DISCARDING Changes)...\n");
        std::string cmd = "dism /Unmount-Image /MountDir:\"" + mountDir + "\" /Discard";
        RunCommand(cmd);
    }

    // Scrubs old system records to keep folders from locking up
    myLog.AddLog("[INFO] Running general mountpoint cleanup...\n");
    RunCommand("dism /Cleanup-Mountpoints");

    if (!stopRequested) {
        myLog.AddLog("[INFO] Unmount sequence complete.\n");
    }
    isTaskRunning = false;
}

// Gives the user full power over the files to allow manual changes
void Task_UnlockPermissions() {
    std::string mountDir = g_InstallDir + "\\PATH";

    if (!DirectoryExists(mountDir + "\\Windows")) {
        myLog.AddLog("[ERROR] WIM is not mounted. Cannot unlock permissions.\n");
        isTaskRunning = false;
        return;
    }

    myLog.AddLog("[INFO] Unlocking permissions (This allows deletion)...\n");

    // Takes control of the files so settings can be changed
    myLog.AddLog("[INFO] Step 1: Taking ownership...\n");
    std::string takeownCmd = "takeown /F \"" + mountDir + "\" /A /R /D Y";
    RunCommand(takeownCmd);

    if (stopRequested) { isTaskRunning = false; return; }

    // Opens up access so anyone can edit the files
    myLog.AddLog("[INFO] Step 2: Granting Full Control to Everyone...\n");
    std::string icaclsCmd = "icacls \"" + mountDir + "\" /grant Everyone:F /T /C /Q";
    RunCommand(icaclsCmd);

    if (stopRequested) { isTaskRunning = false; return; }

    // Strips away hidden or restricted tags that block file deletion
    myLog.AddLog("[INFO] Step 3: Removing Read-Only/System attributes...\n");
    std::string attribCmd = "attrib -r -s -h /s /d \"" + mountDir + "\\*\"";
    RunCommand(attribCmd);

    if (!stopRequested) {
        myLog.AddLog("[SUCCESS] Permissions unlocked. You can now delete files manually.\n");
    }
    isTaskRunning = false;
}