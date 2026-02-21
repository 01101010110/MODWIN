#include "modwin.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

// Pulls the user selected Windows version from the main image file
void Task_Extract() {
    std::string existingWim = g_InstallDir + "\\install.wim";

    // Blocks a new extraction if the file is already there to keep things clean
    if (FileExists(existingWim)) {
        int result = MessageBoxA(NULL, "An existing install.wim was found in the MODWIN folder.\n\nDo you want to delete it and extract the new one?\n\n- Click YES to overwrite and extract.\n- Click NO to cancel extraction and use the existing file (skip to Option 2).", "Existing File Found", MB_YESNO | MB_ICONQUESTION);
        if (result == IDNO) {
            myLog.AddLog("[INFO] Extraction cancelled by user. You may proceed to Option 2 (Mount WIM).\n");
            isTaskRunning = false;
            return;
        }
        try {
            myLog.AddLog("[INFO] Deleting old install.wim...\n");
            if (!std::filesystem::remove(existingWim)) {
                myLog.AddLog("[ERROR] Failed to delete existing install.wim. Is it open?\n");
                MessageBoxA(NULL, "Failed to delete existing install.wim. Please close any programs using it and try again.", "Error", MB_OK | MB_ICONERROR);
                isTaskRunning = false;
                return;
            }
        }
        catch (const std::exception& e) {
            myLog.AddLog("[ERROR] Exception deleting WIM: %s\n", e.what());
            isTaskRunning = false;
            return;
        }
    }

    // Checks that a source is picked before starting any work
    std::string sourceWimPath = detectedWimFile;
    std::string sourceRoot = inputSourcePath;
    if (sourceWimPath.empty() || sourceRoot.empty()) {
        myLog.AddLog("[ERROR] No Source Detected. Please select a drive first.\n");
        isTaskRunning = false;
        return;
    }

    // Prepares the landing spot for the new ISO files
    std::string destRoot = g_InstallDir + "\\ISO";
    if (!DirectoryExists(destRoot)) fs::create_directories(destRoot);

    myLog.AddLog("[INFO] Copying content from %s to ISO folder...\n", sourceRoot.c_str());

    // Moves everything over but leaves the large system files for the final export
    try {
        for (const auto& entry : fs::recursive_directory_iterator(sourceRoot, fs::directory_options::skip_permission_denied)) {
            if (stopRequested) { myLog.AddLog("[INFO] Copy cancelled by user.\n"); break; }

            fs::path relativePath = fs::relative(entry.path(), sourceRoot);
            fs::path destPath = fs::path(destRoot) / relativePath;
            std::string filename = entry.path().filename().string();

            // Skips the main installation files and system junk during the copy
            if (_stricmp(filename.c_str(), "install.wim") == 0 || _stricmp(filename.c_str(), "install.esd") == 0) continue;
            if (relativePath.string().find("System Volume Information") != std::string::npos || relativePath.string().find("$RECYCLE.BIN") != std::string::npos) continue;

            try {
                if (fs::is_directory(entry.status())) {
                    fs::create_directories(destPath);
                }
                else {
                    fs::create_directories(destPath.parent_path());
                    fs::copy_file(entry.path(), destPath, fs::copy_options::overwrite_existing);
                }
            }
            catch (const fs::filesystem_error& e) {
                // Passes over files that are locked or off-limits
                std::string errStr = e.what();
                if (errStr.find("Access is denied") == std::string::npos) {
                    myLog.AddLog("[WARNING] Could not copy %s: %s\n", filename.c_str(), e.what());
                }
            }
        }
        if (!stopRequested) myLog.AddLog("[INFO] File copy complete.\n");
    }
    catch (const std::exception& e) {
        myLog.AddLog("[ERROR] Critical Copy Failure: %s\n", e.what());
        isTaskRunning = false;
        return;
    }

    if (stopRequested) { isTaskRunning = false; return; }

    // Pulls just the user selected index into a fresh workspace
    std::string workingWim = g_InstallDir + "\\install.wim";
    myLog.AddLog("[INFO] Extracting Index %d to workspace...\n", wimIndex);
    std::string dismCmd = "dism /export-image /SourceImageFile:\"" + sourceWimPath + "\" /SourceIndex:" + std::to_string(wimIndex) + " /DestinationImageFile:\"" + workingWim + "\" /Compress:max /CheckIntegrity";
    RunCommand(dismCmd);

    if (stopRequested) { isTaskRunning = false; return; }

    // Clears out any stray image files in the destination folder
    std::string isoWim = destRoot + "\\sources\\install.wim";
    std::string isoEsd = destRoot + "\\sources\\install.esd";
    if (FileExists(isoWim)) fs::remove(isoWim);
    if (FileExists(isoEsd)) fs::remove(isoEsd);

    // Makes sure the boot files actually landed in the destination
    std::string bootWimCheck = destRoot + "\\sources\\boot.wim";
    if (!FileExists(bootWimCheck)) {
        myLog.AddLog("[ERROR] CRITICAL: boot.wim was NOT found in destination!\n");
        myLog.AddLog("[TIP] Check if the source drive has /sources/boot.wim\n");
    }

    if (FileExists(workingWim)) myLog.AddLog("[SUCCESS] Extraction Complete. You can now Mount the WIM.\n");
    else myLog.AddLog("[ERROR] Extraction Failed.\n");

    isTaskRunning = false;
}

// Scans the image file to see which Windows editions are inside
void ScanWimEditions() {
    std::string wimFile = inputSourcePath;
    if (!detectedWimFile.empty()) wimFile = detectedWimFile;
    if (wimFile.empty() || !FileExists(wimFile)) return;

    myLog.AddLog("[INFO] Scanning %s...\n", wimFile.c_str());
    editionNames.clear();

    std::string cmd = "dism /Get-WimInfo /WimFile:\"" + wimFile + "\"";

    // Runs the scan quietly behind the scenes
    std::string result = RunCommandCapture(cmd);

    // Sorts through the text to pull out and clean up edition names
    std::stringstream ss(result);
    std::string line;
    int idx = 1;
    while (std::getline(ss, line)) {
        if (line.find("Name : ") != std::string::npos) {
            std::string name = line.substr(7);
            name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
            name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
            editionNames.push_back(std::to_string(idx) + ". " + name);
            idx++;
        }
    }
    myLog.AddLog("[INFO] Found %d editions.\n", editionNames.size());
    selectedEditionComboIndex = 0;
    wimIndex = 1;
}