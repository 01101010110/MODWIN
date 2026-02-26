#include "modwin.h"
#include <iostream>
#include <cstdio>
#include <algorithm>
#include <unordered_map>
#include <cctype>
#include <sstream>

// Holds the list of applications found on the system
std::vector<ItemData> detectedApps;
// Holds the list of system packages found
std::vector<ItemData> detectedPackages;
// Holds the list of Windows features
std::vector<ItemData> detectedFeatures;
// Stores the text typed into the search bar
char searchFilter[128] = "";
// Stores the file path when selecting an app to install
char appxInstallPath[MAX_PATH] = "";
// Stores the file path when selecting a package to install
char packageInstallPath[MAX_PATH] = "";
// Stores the path to the installation media for missing payloads
char sxsSourcePath[MAX_PATH] = "";

void Task_ScanApps();
void Task_ScanPackages();

// Makes characters lowercase so capitalization does not break the sort order
char SafeToLower(char c) {
    return std::tolower(static_cast<unsigned char>(c));
}

// Shortens long package, app, and feature names for display in window titles
std::string SimplifyName(const std::string& fullName) {
    std::string name = fullName;

    // Strip everything after ~ (package hashes and version info)
    size_t tildePos = name.find('~');
    if (tildePos != std::string::npos) name = name.substr(0, tildePos);

    // Strip everything after _ (appx version and architecture info)
    size_t underPos = name.find('_');
    if (underPos != std::string::npos) name = name.substr(0, underPos);

    // Remove common prefixes
    std::vector<std::string> prefixes = {
        "Microsoft-Windows-", "Microsoft-OneCore-", "Microsoft.",
        "Microsoft-", "Windows-", "MicrosoftWindows."
    };
    for (const auto& prefix : prefixes) {
        if (name.length() > prefix.length() && _strnicmp(name.c_str(), prefix.c_str(), prefix.length()) == 0) {
            name = name.substr(prefix.length());
            break;
        }
    }

    // Remove common suffixes
    std::vector<std::string> suffixes = {
        "-FOD-Package", "-Package", "-FoD", "-FOD", "-WOW64", "-Deployment"
    };
    for (const auto& suffix : suffixes) {
        if (name.length() > suffix.length()) {
            size_t pos = name.length() - suffix.length();
            if (_strnicmp(name.c_str() + pos, suffix.c_str(), suffix.length()) == 0) {
                name = name.substr(0, pos);
            }
        }
    }

    // Replace separators with spaces
    for (char& c : name) {
        if (c == '-' || c == '.') c = ' ';
    }

    // Trim trailing spaces
    while (!name.empty() && name.back() == ' ') name.pop_back();

    if (name.empty()) return fullName;
    return name;
}

// Arranges items alphabetically from A to Z
bool CompareItems(const ItemData& a, const ItemData& b) {
    return _stricmp(a.Name.c_str(), b.Name.c_str()) < 0;
}

// Runs the process to install user's application file
void Task_InstallAppx() {
    std::string path = appxInstallPath;
    // Checks if the selected file exists before trying to use it
    if (path.empty() || !FileExists(path)) {
        myLog.AddLog("[ERROR] Invalid file path selected.\n");
        isTaskRunning = false;
        return;
    }

    // Verifies the Windows image folder is available to be modified
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
        myLog.AddLog("[ERROR] WIM is not mounted.\n");
        isTaskRunning = false;
        return;
    }

    myLog.AddLog("[INFO] Installing Appx: %s\n", path.c_str());

    // Builds the command to add the appx package and runs it
    std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Add-ProvisionedAppxPackage /PackagePath:\"" + path + "\" /SkipLicense";
    RunCommand(cmd);

    myLog.AddLog("[SUCCESS] Appx install process finished. Refreshing list...\n");
    isTaskRunning = false;

    // Starts a background update to show the new app in the list
    std::thread t(Task_ScanApps);
    t.detach();
}

// Runs the process to install user's custom package file
void Task_InstallPackage() {
    std::string path = packageInstallPath;
    // Checks if the selected file exists before trying to use it
    if (path.empty() || !FileExists(path)) {
        myLog.AddLog("[ERROR] Invalid file path selected.\n");
        isTaskRunning = false;
        return;
    }

    // Verifies the Windows image folder is available to be modified
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
        myLog.AddLog("[ERROR] WIM is not mounted.\n");
        isTaskRunning = false;
        return;
    }

    myLog.AddLog("[INFO] Preparing Package: %s\n", path.c_str());

    // Create a Scratch directory to prevent DISM temp path overflow
    std::string scratchDir = g_InstallDir + "\\SCRATCH";
    if (!DirectoryExists(scratchDir)) {
        std::filesystem::create_directories(scratchDir);
    }

    // Create an extraction directory
    std::string extractDir = g_InstallDir + "\\PKG_EXTRACT";
    if (DirectoryExists(extractDir)) {
        std::filesystem::remove_all(extractDir);
    }
    std::filesystem::create_directories(extractDir);

    // Uses Windows expand.exe to extract the cab/msu file first.
    myLog.AddLog("[INFO] Extracting package contents...\n");
    std::string expandCmd = "expand.exe -f:* \"" + path + "\" \"" + extractDir + "\"";
    RunCommand(expandCmd);

    // Scan the extracted files to determine if this is an Update or a Driver
    bool hasMum = false;
    bool hasCabOrMsu = false;
    bool hasInf = false;

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(extractDir)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), SafeToLower);
                if (ext == ".mum") hasMum = true;
                else if (ext == ".inf") hasInf = true;
                else if (ext == ".cab" || ext == ".msu") hasCabOrMsu = true;
            }
        }
    }
    catch (...) {}

    std::string cmd;
    if (hasMum || hasCabOrMsu) {
        myLog.AddLog("[INFO] Update package detected. Installing...\n");
        cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Add-Package /PackagePath:\"" + extractDir + "\" /ScratchDir:\"" + scratchDir + "\"";
    }
    else if (hasInf) {
        myLog.AddLog("[INFO] Driver package detected. Installing...\n");
        cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Add-Driver /Driver:\"" + extractDir + "\" /Recurse /ScratchDir:\"" + scratchDir + "\"";
    }
    else {
        myLog.AddLog("[INFO] Unknown package type. Attempting standard package install...\n");
        cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Add-Package /PackagePath:\"" + extractDir + "\" /ScratchDir:\"" + scratchDir + "\"";
    }

    RunCommand(cmd);

    // Clean up the temporary folders
    myLog.AddLog("[INFO] Cleaning up temporary files...\n");
    try {
        std::filesystem::remove_all(extractDir);
    }
    catch (...) {}

    myLog.AddLog("[SUCCESS] Package install process finished. Refreshing list...\n");
    isTaskRunning = false;

    // Starts a background update to show the new package in the list
    std::thread t(Task_ScanPackages);
    t.detach();
}

// Draws the list interface on the screen
void DrawChecklistUI(const char* label, std::vector<ItemData>& items, void(*scanFunc)(), const char* btnLabel1, void(*actionFunc1)(), const char* btnLabel2, void(*actionFunc2)()) {
    ImGui::Text("%s", label);

    // Button to scan for apps, packages, features
    if (ImGui::Button("Scan / Refresh List")) { if (!isTaskRunning) { isTaskRunning = true; stopRequested = false; std::thread t(scanFunc); t.detach(); } }

    ImGui::SameLine(); ImGui::Text("Filter:"); ImGui::SameLine(); ImGui::PushItemWidth(150);
    // Input box for typing text to filter the results
    ImGui::InputText("##Search", searchFilter, IM_ARRAYSIZE(searchFilter));
    ImGui::PopItemWidth();

    ImGui::Separator();

    // Calculates how much screen space is left for the list
    float availableY = ImGui::GetContentRegionAvail().y;
    float listHeight = availableY - 110.0f;
    if (listHeight < 100) listHeight = 100;

    bool disabledMode = isTaskRunning;

    // Begins the scrollable area where items appear
    ImGui::BeginChild("ListRegion", ImVec2(0, listHeight), true);
    if (disabledMode) ImGui::BeginDisabled();
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        // Shows a message if the list is empty
        if (items.empty()) { ImGui::TextDisabled("List is empty. Click Scan."); }
        else {
            // Loops through every item found to display it
            for (size_t i = 0; i < items.size(); i++) {
                // Checks if text is in the filter box and hides items that do not match
                if (strlen(searchFilter) > 0) {
                    std::string n = items[i].Name;
                    std::string f = searchFilter;
                    bool match = false;

                    // Checks for a match regardless of upper or lower case
                    if (n.find(f) != std::string::npos) match = true;
                    else {
                        std::transform(n.begin(), n.end(), n.begin(), SafeToLower);
                        std::transform(f.begin(), f.end(), f.begin(), SafeToLower);
                        if (n.find(f) != std::string::npos) match = true;
                    }

                    // Skips drawing this item if it does not match the search
                    if (!match) continue;
                }

                // Sets the text color: Green for Safe, Yellow for Caution, Red for Critical
                ImVec4 textColor = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
                if (items[i].SafetyColor == 1) textColor = ImVec4(0.4f, 1.0f, 0.4f, 1.00f); // Safe
                if (items[i].SafetyColor == 2) textColor = ImVec4(1.0f, 0.9f, 0.0f, 1.00f); // Caution
                if (items[i].SafetyColor == 3) textColor = ImVec4(1.0f, 0.4f, 0.4f, 1.00f); // Critical

                std::string displayLabel = items[i].Name;

                // Adds the status (like Enabled/Disabled) to the name if it exists
                if (!items[i].State.empty()) {
                    displayLabel += " (" + items[i].State + ")";
                }

                // Adds a star symbol if there is extra information available for this item
                if (!items[i].ExtraInfo.empty()) displayLabel += " *";

                ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                std::string id = "##" + std::to_string(i);

                // Draws the checkbox and name for the item
                ImGui::Checkbox((displayLabel + id).c_str(), &items[i].IsSelected);
                ImGui::PopStyleColor();

                // Right Mouse Button window
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !items[i].ExtraInfo.empty()) {
                    items[i].ShowDetails = true;
                }

                // Shows a popup with more information when hovering over the item
                if (ImGui::IsItemHovered() && !items[i].ExtraInfo.empty()) {
                    ImGui::BeginTooltip();
                    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.00f), "Knowledge Base Info (Right-Click to Pin):");
                    ImGui::Separator();

                    std::string cleanTip = items[i].ExtraInfo;
                    size_t linkPos = cleanTip.find("[Source:");
                    if (linkPos != std::string::npos) {
                        cleanTip = cleanTip.substr(0, linkPos);
                    }
                    ImGui::TextUnformatted(cleanTip.c_str());

                    if (items[i].SafetyColor == 3) {
                        ImGui::Dummy(ImVec2(0.0f, 5.0f));
                        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "WARNING: Do not remove unless necessary.");
                    }
                    ImGui::PopTextWrapPos();
                    ImGui::EndTooltip();
                }
            }
        }
    }
    if (disabledMode) ImGui::EndDisabled();
    ImGui::EndChild();

    // Loop through items again to draw any open detail windows
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (size_t i = 0; i < items.size(); i++) {
            if (items[i].ShowDetails) {
                // Creates a unique name for the window based on the item index
                std::string windowTitle = "Knowledge Base - " + SimplifyName(items[i].Name) + "##Dtl" + std::to_string(i);

                // Sets a default size for the window the first time it opens
                ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);

                // Pink title bar
                ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.35f, 0.10f, 0.55f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.45f, 0.15f, 0.65f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.25f, 0.08f, 0.40f, 1.0f));

                // Creates the "X" close button
                if (ImGui::Begin(windowTitle.c_str(), &items[i].ShowDetails, ImGuiWindowFlags_NoCollapse)) {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.00f), "Knowledge Base Info:");
                    ImGui::Separator();

                    // Separate description from URL
                    std::string fullText = items[i].ExtraInfo;
                    std::string cleanDesc = fullText;
                    std::string linkUrl = "";

                    // Looks for the link
                    size_t linkPos = fullText.find("[Source:");
                    if (linkPos != std::string::npos) {
                        cleanDesc = fullText.substr(0, linkPos);

                        // Extracts the URL
                        size_t urlStart = fullText.find("](", linkPos);
                        size_t urlEnd = fullText.find(")", urlStart);

                        // Are valid positions found?
                        if (urlStart != std::string::npos && urlEnd != std::string::npos) {
                            linkUrl = fullText.substr(urlStart + 2, urlEnd - (urlStart + 2));
                        }
                    }

                    // Displays the cleaned description
                    ImGui::TextWrapped("%s", cleanDesc.c_str());

                    // If a URL was found, display it as a clickable link
                    if (!linkUrl.empty()) {
                        ImGui::Dummy(ImVec2(0.0f, 8.0f));
                        ImGui::Text("Reference:");
                        ImGui::SameLine();

                        // Make the text look like a link
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.6f, 1.0f, 1.0f));
                        ImGui::Text("Open Official Documentation (Click Here)");

                        // Hover and Click
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand); 
                            if (ImGui::IsMouseClicked(0)) {
                                ShellExecuteA(NULL, "open", linkUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
                            }
                        }
                        ImGui::PopStyleColor();
                    }

                    if (items[i].SafetyColor == 3) {
                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "CRITICAL SYSTEM COMPONENT");
                        ImGui::TextWrapped("Removing this item may cause system instability or boot failure.");
                    }
                    else if (items[i].SafetyColor == 2) {
                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.0f, 1.0f), "CAUTION RECOMMENDED");
                        ImGui::TextWrapped("This item provides useful functionality. Ensure you do not need it before removing.");
                    }
                }
                ImGui::End();
                ImGui::PopStyleColor(3);
            }
        }
    }

    // Button to select every item in the list
    if (ImGui::Button("Select All")) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (auto& item : items) item.IsSelected = true;
    }
    ImGui::SameLine();
    // Button to uncheck every item in the list
    if (ImGui::Button("Deselect All")) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (auto& item : items) item.IsSelected = false;
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.00f));
    // Button to quickly select only the items known to be safe to remove
    if (ImGui::Button("Select All Safe")) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (auto& item : items) if (item.SafetyColor == 1) item.IsSelected = true;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.0f, 1.00f));
    // Button to quickly select items that require caution
    if (ImGui::Button("Select All Caution")) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (auto& item : items) if (item.SafetyColor == 2) item.IsSelected = true;
    }
    ImGui::PopStyleColor();

    // Shows Select All Enabled and Show All Disabled buttons only if viewing the Features list
    if (std::string(label).find("Features") != std::string::npos) {
        ImGui::SameLine();
        if (ImGui::Button("Select All Enabled")) {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (auto& item : items) {
                if (item.State.find("Enabled") != std::string::npos) item.IsSelected = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Select All Disabled")) {
            std::lock_guard<std::mutex> lock(dataMutex);
            for (auto& item : items) {
                if (item.State.find("Disabled") != std::string::npos) item.IsSelected = true;
            }
        }
    }

    // Shows add Custom Appx button only if viewing the "Appx" list
    if (std::string(label).find("(Appx)") != std::string::npos) {
        ImGui::SameLine();
        if (ImGui::Button("Add Custom Appx")) {
            ImGui::OpenPopup("Install Custom Appx Package");
        }
    }

    // Popup window for adding a custom file
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.35f, 0.10f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.45f, 0.15f, 0.65f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.25f, 0.08f, 0.40f, 1.0f));
    if (ImGui::BeginPopupModal("Install Custom Appx Package", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Install Custom Appx/Msix Package");
        ImGui::Separator();
        ImGui::Text("You can download offline .appx or .msixbundle files from:");

        if (ImGui::Button("store.rg-adguard.net (Open in Browser)")) {
            ShellExecuteA(nullptr, "open", "https://store.rg-adguard.net/", nullptr, nullptr, SW_SHOWNORMAL);
        }

        ImGui::Spacing();
        ImGui::Text("Once downloaded, select the file below to install it into the image.");
        ImGui::Spacing();

        if (ImGui::Button("Select File & Install")) {
            // Opens a window to browse for the file
            if (OpenFileDialog(appxInstallPath, MAX_PATH, "App Packages\0*.appx;*.msix;*.appxbundle;*.msixbundle\0All Files\0*.*\0")) {
                if (!isTaskRunning) {
                    isTaskRunning = true;
                    stopRequested = false;
                    std::thread t(Task_InstallAppx);
                    t.detach();
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(3);

    // Shows add Custom Package button only if viewing the "Packages" list
    if (std::string(label).find("Package") != std::string::npos) {
        ImGui::SameLine();
        if (ImGui::Button("Add Custom Package")) {
            ImGui::OpenPopup("Install Custom Package");
        }
    }

    // Popup window for adding a custom package file
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.35f, 0.10f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.45f, 0.15f, 0.65f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.25f, 0.08f, 0.40f, 1.0f));
    if (ImGui::BeginPopupModal("Install Custom Package", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Install Custom .cab or .msu Package");
        ImGui::Separator();
        ImGui::Text("You can download offline update packages from the Microsoft Update Catalog:");

        if (ImGui::Button("catalog.update.microsoft.com (Open in Browser)")) {
            ShellExecuteA(nullptr, "open", "https://www.catalog.update.microsoft.com/Home.aspx", nullptr, nullptr, SW_SHOWNORMAL);
        }

        ImGui::Spacing();
        ImGui::Text("Once downloaded, select the file below to install it into the image.");
        ImGui::Spacing();

        if (ImGui::Button("Select File & Install")) {
            // Opens a window to browse for the file
            if (OpenFileDialog(packageInstallPath, MAX_PATH, "Windows Packages\0*.cab;*.msu\0All Files\0*.*\0")) {
                if (!isTaskRunning) {
                    isTaskRunning = true;
                    stopRequested = false;
                    std::thread t(Task_InstallPackage);
                    t.detach();
                    ImGui::CloseCurrentPopup();
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing(); ImGui::Separator();

    // Shows a "Working" message if a task is currently running
    if (isTaskRunning) {
        ImGui::BeginDisabled(); ImGui::Button("WORKING...", ImVec2(-1.0f, 40.0f)); ImGui::EndDisabled();
    }
    else {
        if (btnLabel2) {
            float width = ImGui::GetContentRegionAvail().x * 0.5f - 5.0f;
            if (ImGui::Button(btnLabel1, ImVec2(width, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(actionFunc1); t.detach(); }
            ImGui::SameLine();
            if (ImGui::Button(btnLabel2, ImVec2(width, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(actionFunc2); t.detach(); }
        }
        else {
            if (ImGui::Button(btnLabel1, ImVec2(-1.0f, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(actionFunc1); t.detach(); }
        }
    }
}

// Scans the Windows image to find built-in applications
void Task_ScanApps() {
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) { myLog.AddLog("[ERROR] WIM is not mounted.\n"); isTaskRunning = false; return; }
    myLog.AddLog("[INFO] Scanning for Apps...\n");

    try {
        std::unordered_map<std::string, std::string> appMap;
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Get-ProvisionedAppxPackages";
        // Captures the text output from the scan command
        std::string output = RunCommandCapture(cmd);
        std::stringstream ss(output);
        std::string line;

        // Reads the output line by line to find package names
        while (std::getline(ss, line)) {
            if (stopRequested) break;
            size_t pos = line.find("PackageName : ");
            if (pos != std::string::npos) {
                std::string name = line.substr(pos + 14);
                name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
                name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
                if (!name.empty()) {
                    appMap[name] = "";
                }
            }
        }

        if (!stopRequested) {
            std::lock_guard<std::mutex> lock(dataMutex);
            detectedApps.clear();
            detectedApps.reserve(appMap.size());

            // Adds finding to the list and looks up details in the database
            for (const auto& kv : appMap) {
                AppKnowledge info = g_dbManager.GetAppInfo(kv.first);
                detectedApps.push_back({ kv.first, info.Description, info.SafetyRating, false, "", false });
            }

            // Sorts the final list alphabetically
            if (!detectedApps.empty()) {
                std::sort(detectedApps.begin(), detectedApps.end(), CompareItems);
            }
            myLog.AddLog("[SUCCESS] Found %d apps.\n", (int)detectedApps.size());
        }
    }
    catch (const std::exception& e) {
        myLog.AddLog("[ERROR] Scan Exception: %s\n", e.what());
    }
    isTaskRunning = false;
}

// Removes the selected applications from the Windows image
void Task_RemoveSelectedApps() {
    std::vector<std::string> toRemove;
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        // Gathers the names of all checked items
        for (const auto& item : detectedApps) {
            if (item.IsSelected) toRemove.push_back(item.Name);
        }
    }
    // Process each item one by one
    for (const auto& name : toRemove) {
        if (stopRequested) break;
        myLog.AddLog("[INFO] Removing: %s\n", name.c_str());
        g_dbManager.LogRemoval(name, "Appx");
        // Removes the user selected appx packages
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Remove-ProvisionedAppxPackage /PackageName:\"" + name + "\"";
        RunCommand(cmd);
    }
    myLog.AddLog("[SUCCESS] Removal complete. Refreshing list...\n");
    // Refreshes the list to show the items are gone
    Task_ScanApps();
}

// Scans the Windows image for system packages
void Task_ScanPackages() {
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) { myLog.AddLog("[ERROR] WIM is not mounted.\n"); isTaskRunning = false; return; }
    myLog.AddLog("[INFO] Scanning for Packages and Drivers...\n");

    try {
        std::unordered_map<std::string, std::string> pkgMap;
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Get-Packages";

        std::string output = RunCommandCapture(cmd);
        std::stringstream ss(output);
        std::string line;

        std::string currentPkgId = "";
        std::string currentPkgState = "";

        // Reads the output to match package IDs with their installation state
        while (std::getline(ss, line)) {
            if (stopRequested) break;

            size_t posId = line.find("Package Identity : ");
            if (posId != std::string::npos) {
                if (!currentPkgId.empty()) {
                    if (currentPkgState.find("Installed") != std::string::npos || currentPkgState.find("Staged") != std::string::npos) {
                        pkgMap[currentPkgId] = currentPkgState;
                    }
                }
                currentPkgId = line.substr(posId + 19);
                currentPkgId.erase(std::remove(currentPkgId.begin(), currentPkgId.end(), '\n'), currentPkgId.end());
                currentPkgId.erase(std::remove(currentPkgId.begin(), currentPkgId.end(), '\r'), currentPkgId.end());
                currentPkgState = "";
            }

            size_t posState = line.find("State : ");
            if (posState != std::string::npos) {
                currentPkgState = line.substr(posState + 8);
                currentPkgState.erase(std::remove(currentPkgState.begin(), currentPkgState.end(), '\n'), currentPkgState.end());
                currentPkgState.erase(std::remove(currentPkgState.begin(), currentPkgState.end(), '\r'), currentPkgState.end());
            }
        }

        // Adds the last item found to the list
        if (!currentPkgId.empty()) {
            if (currentPkgState.find("Installed") != std::string::npos || currentPkgState.find("Staged") != std::string::npos) {
                pkgMap[currentPkgId] = currentPkgState;
            }
        }

        // Scans for 3rd Party Drivers to merge into the UI list
        std::string cmdDrv = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Get-Drivers";
        std::string outputDrv = RunCommandCapture(cmdDrv);
        std::stringstream ssDrv(outputDrv);

        std::string currentPubName = "";
        std::string currentOrigName = "";
        std::string currentClass = "";

        while (std::getline(ssDrv, line)) {
            if (stopRequested) break;

            size_t posPub = line.find("Published Name : ");
            if (posPub != std::string::npos) {
                if (!currentPubName.empty()) {
                    pkgMap[currentPubName] = "Driver (" + currentClass + ") - " + currentOrigName;
                }
                currentPubName = line.substr(posPub + 17);
                currentPubName.erase(std::remove(currentPubName.begin(), currentPubName.end(), '\n'), currentPubName.end());
                currentPubName.erase(std::remove(currentPubName.begin(), currentPubName.end(), '\r'), currentPubName.end());
                currentOrigName = "";
                currentClass = "";
            }

            size_t posOrig = line.find("Original File Name : ");
            if (posOrig != std::string::npos) {
                currentOrigName = line.substr(posOrig + 21);
                currentOrigName.erase(std::remove(currentOrigName.begin(), currentOrigName.end(), '\n'), currentOrigName.end());
                currentOrigName.erase(std::remove(currentOrigName.begin(), currentOrigName.end(), '\r'), currentOrigName.end());
            }

            size_t posClass = line.find("Class Name : ");
            if (posClass != std::string::npos) {
                currentClass = line.substr(posClass + 13);
                currentClass.erase(std::remove(currentClass.begin(), currentClass.end(), '\n'), currentClass.end());
                currentClass.erase(std::remove(currentClass.begin(), currentClass.end(), '\r'), currentClass.end());
            }
        }

        if (!currentPubName.empty()) {
            pkgMap[currentPubName] = "Driver (" + currentClass + ") - " + currentOrigName;
        }

        if (!stopRequested) {
            std::lock_guard<std::mutex> lock(dataMutex);
            detectedPackages.clear();
            detectedPackages.reserve(pkgMap.size());

            // Adds package info to the display list
            for (const auto& kv : pkgMap) {
                AppKnowledge info = g_dbManager.GetAppInfo(kv.first);
                detectedPackages.push_back({ kv.first, info.Description, info.SafetyRating, false, kv.second, false });
            }

            if (!detectedPackages.empty()) {
                std::sort(detectedPackages.begin(), detectedPackages.end(), CompareItems);
            }
            myLog.AddLog("[SUCCESS] Found %d valid packages/drivers.\n", (int)detectedPackages.size());
        }
    }
    catch (const std::exception& e) {
        myLog.AddLog("[ERROR] Scan Exception: %s\n", e.what());
    }
    isTaskRunning = false;
}

// Removes the selected system packages
void Task_RemoveSelectedPackages() {
    std::vector<std::string> toRemove;
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (const auto& item : detectedPackages) {
            if (item.IsSelected) toRemove.push_back(item.Name);
        }
    }
    for (const auto& name : toRemove) {
        if (stopRequested) break;
        myLog.AddLog("[INFO] Removing: %s\n", name.c_str());
        g_dbManager.LogRemoval(name, "Package");

        std::string extCheck = name;
        std::transform(extCheck.begin(), extCheck.end(), extCheck.begin(), SafeToLower);

        std::string cmd;
        // Check if the item is a driver (.inf) or an update package
        if (extCheck.length() >= 4 && extCheck.substr(extCheck.length() - 4) == ".inf") {
            cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Remove-Driver /Driver:\"" + name + "\"";
        }
        else {
            cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Remove-Package /PackageName:\"" + name + "\"";
        }
        RunCommand(cmd);
    }
    myLog.AddLog("[SUCCESS] Removal complete. Refreshing list...\n");
    Task_ScanPackages();
}

// Scans for Windows Features
void Task_ScanFeatures() {
    if (!DirectoryExists(g_InstallDir + "\\PATH\\Windows")) { myLog.AddLog("[ERROR] WIM is not mounted.\n"); isTaskRunning = false; return; }
    myLog.AddLog("[INFO] Scanning for Features...\n");

    try {
        std::unordered_map<std::string, std::string> featMap;
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Get-Features";

        std::string output = RunCommandCapture(cmd);
        std::stringstream ss(output);
        std::string line;

        std::string currentFeatureName = "";
        // Reads output to get feature names and their current state (Enabled/Disabled)
        while (std::getline(ss, line)) {
            if (stopRequested) break;
            size_t posName = line.find("Feature Name : ");
            if (posName != std::string::npos) {
                currentFeatureName = line.substr(posName + 15);
                currentFeatureName.erase(std::remove(currentFeatureName.begin(), currentFeatureName.end(), '\n'), currentFeatureName.end());
                currentFeatureName.erase(std::remove(currentFeatureName.begin(), currentFeatureName.end(), '\r'), currentFeatureName.end());
            }
            size_t posState = line.find("State : ");
            if (posState != std::string::npos && !currentFeatureName.empty()) {
                std::string state = line.substr(posState + 8);
                state.erase(std::remove(state.begin(), state.end(), '\n'), state.end());
                state.erase(std::remove(state.begin(), state.end(), '\r'), state.end());

                if (state == "Enable Pending") state = "Enabled";
                if (state == "Disable Pending") state = "Disabled";

                featMap[currentFeatureName] = state;
                currentFeatureName = "";
            }
        }

        if (!stopRequested) {
            std::lock_guard<std::mutex> lock(dataMutex);
            detectedFeatures.clear();
            detectedFeatures.reserve(featMap.size());

            // Adds feature info to the display list
            for (const auto& kv : featMap) {
                // Looks up the feature in the database
                AppKnowledge info = g_dbManager.GetAppInfo(kv.first);

                std::string desc = info.Description;
                if (desc == "No description available.") {
                    desc = "Status: " + kv.second; // Uses status if no description is found
                }

                // Stores the state and description
                detectedFeatures.push_back({ kv.first, desc, info.SafetyRating, false, kv.second, false });
            }

            if (!detectedFeatures.empty()) {
                std::sort(detectedFeatures.begin(), detectedFeatures.end(), CompareItems);
            }
            myLog.AddLog("[SUCCESS] Found %d features.\n", (int)detectedFeatures.size());
        }
    }
    catch (const std::exception& e) {
        myLog.AddLog("[ERROR] Scan Exception: %s\n", e.what());
    }
    isTaskRunning = false;
}

// Turns on the selected Windows Features
void Task_EnableSelectedFeatures() {
    std::vector<std::string> toEnable;
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (const auto& item : detectedFeatures) {
            if (item.IsSelected) toEnable.push_back(item.Name);
        }
    }

    std::string sxsPath = "";
    std::string pathFromInput = std::string(inputSourcePath) + "\\sources\\sxs";
    std::string pathFromIso = g_InstallDir + "\\ISO\\sources\\sxs";

    if (DirectoryExists(pathFromInput)) {
        sxsPath = pathFromInput;
    }
    else if (DirectoryExists(pathFromIso)) {
        sxsPath = pathFromIso;
    }

    for (const auto& name : toEnable) {
        if (stopRequested) break;
        myLog.AddLog("[INFO] Enabling: %s\n", name.c_str());
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Enable-Feature /FeatureName:\"" + name + "\" /All";

        if (!sxsPath.empty()) {
            cmd += " /Source:\"" + sxsPath + "\" /LimitAccess";
        }
        else {
            myLog.AddLog("[WARNING] The sources\\sxs folder was not found in the extracted ISO files.\n");
            myLog.AddLog("[WARNING] If %s requires external installation payloads, it will fail.\n", name.c_str());
        }

        RunCommand(cmd);
    }
    myLog.AddLog("[SUCCESS] Enable complete. Refreshing list...\n");
    Task_ScanFeatures();
}

// Turns off the selected Windows Features
void Task_DisableSelectedFeatures() {
    std::vector<std::string> toDisable;
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (const auto& item : detectedFeatures) {
            if (item.IsSelected) toDisable.push_back(item.Name);
        }
    }
    for (const auto& name : toDisable) {
        if (stopRequested) break;
        myLog.AddLog("[INFO] Disabling: %s\n", name.c_str());
        std::string cmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /Disable-Feature /FeatureName:\"" + name + "\"";
        RunCommand(cmd);
    }
    myLog.AddLog("[SUCCESS] Disable complete. Refreshing list...\n");
    Task_ScanFeatures();
}
