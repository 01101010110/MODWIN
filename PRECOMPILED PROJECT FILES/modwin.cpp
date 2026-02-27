#include "modwin.h"

// Tracking for folders and current app status
std::string g_InstallDir = "";
std::string g_IniPath = "";
int selectedOption = 0;
int wimIndex = 1;
char inputSourcePath[MAX_PATH] = "";
std::string detectedWimFile = "";
char outputIsoPath[MAX_PATH] = "";
std::vector<std::string> editionNames;
int selectedEditionComboIndex = 0;

// User choice for closing out the image
bool unmount_SaveChanges = false;

// Keeps background tasks and UI in sync
std::atomic<bool> isTaskRunning(false);
std::atomic<bool> stopRequested(false);
std::mutex logMutex;
std::mutex dataMutex;
AppLog myLog;

// Visual preference settings
void ApplyTitleBarColor(HWND hWnd);
void SetupImGuiStyle();

// Entry point for the setup and main loop
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        // Checks for admin rights and restarts if needed
        if (!IsUserAdmin()) {
            wchar_t szPath[MAX_PATH];
            if (GetModuleFileNameW(NULL, szPath, ARRAYSIZE(szPath))) {
                SHELLEXECUTEINFOW sei = { sizeof(sei) };
                sei.lpVerb = L"runas";
                sei.lpFile = szPath;
                sei.hwnd = NULL;
                sei.nShow = SW_NORMAL;
                if (!ShellExecuteExW(&sei)) return 1;
            }
            return 0;
        }

        // Builds the system components and folder structure
        (void)CoInitialize(NULL);
        SetupInstallDirectory();
        BuildModwinFolder();

        // Initialize the Database
        g_dbManager.Initialize(g_InstallDir + "\\BIN");

        WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, hInstance,
                           LoadIcon(hInstance, MAKEINTRESOURCE(101)),
                           nullptr, nullptr, nullptr, L"ModwinClass",
                           LoadIcon(hInstance, MAKEINTRESOURCE(101)) };
        ::RegisterClassExW(&wc);

        // Builds the main window frame
        HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"MODWIN v7",
            WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, wc.hInstance, nullptr);

        // Makes the title bar match the dark theme
        ApplyTitleBarColor(hwnd);

        // Starts the graphics hardware
        if (!CreateDeviceD3D(hwnd)) {
            CleanupDeviceD3D();
            ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        // Sets up the menu library and loads saved positions
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        fs::create_directories(g_InstallDir + "\\BIN\\IMGUI");
        g_IniPath = g_InstallDir + "\\BIN\\IMGUI\\imgui.ini";
        io.IniFilename = g_IniPath.c_str();
        ImGui::LoadIniSettingsFromDisk(io.IniFilename);

        SetupImGuiStyle();

        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        myLog.AddLog("Welcome to MODWIN v7.\nSelect your Windows ISO Drive or Folder to begin.\n");

        // Prepares the first frame of the background
        const float clear_color_init[4] = { 0.12f, 0.10f, 0.16f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_init);
        g_pSwapChain->Present(1, 0);

        // Brings the window into view
        ::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        ::UpdateWindow(hwnd);

        bool done = false;
        while (!done) {
            // Deals with mouse clicks and typing
            MSG msg;
            while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                if (msg.message == WM_QUIT) done = true;
            }
            if (done) break;

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            {
                // Makes the menu full screen
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(io.DisplaySize);
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

                ImGui::Begin("MODWIN Container", nullptr, window_flags);

                ImGui::Dummy(ImVec2(0.0f, 5.0f));
                ImGui::Separator();
                ImGui::Spacing();

                // Calculates space for the log console at the bottom
                float consoleHeight = 220.0f;
                float controlsHeight = ImGui::GetContentRegionAvail().y - consoleHeight;
                if (controlsHeight < 200) controlsHeight = 200;

                ImGui::BeginChild("Controls", ImVec2(0, controlsHeight), true);

                // Tracks if the app is currently busy with a task
                bool disabledMode = isTaskRunning;

                const char* menuItems[] = {
                    "1. Extract WIM/ESD",
                    "2. Mount WIM",
                    "3. Apps Manager",
                    "4. Package Manager",
                    "5. Features Manager",
                    "6. Registry Tools",
                    "7. Add Custom Files",
                    "8. Build ISO",
                    "9. Force Unmount",
                    "10. Uninstall MODWIN",
                    "11. Credits"
                };
                ImGui::Text("Select Operation:");

                ImGui::SetNextItemWidth(350.0f);
                ImGui::Combo("##Menu", &selectedOption, menuItems, IM_ARRAYSIZE(menuItems), 15);

                ImGui::Spacing();

                if (selectedOption == 0) {
                    // Lets the user pick where the Windows files are located
                    ImGui::Text("Windows ISO Drive / Extracted Folder:");
                    ImGui::PushItemWidth(350.0f);
                    ImGui::InputText("##InputPath", inputSourcePath, MAX_PATH, ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    if (ImGui::Button("Browse")) {
                        std::string folder = PickFolderDialog();
                        if (!folder.empty()) {
                            strncpy_s(inputSourcePath, folder.c_str(), MAX_PATH);
                            fs::path root = folder;
                            if (FileExists((root / "sources" / "install.wim").string())) {
                                detectedWimFile = (root / "sources" / "install.wim").string();
                                ScanWimEditions();
                            }
                            else if (FileExists((root / "sources" / "install.esd").string())) {
                                detectedWimFile = (root / "sources" / "install.esd").string();
                                ScanWimEditions();
                            }
                            else {
                                detectedWimFile = "";
                                editionNames.clear();
                                myLog.AddLog("[ERROR] No install.wim or install.esd found in %s\\sources\n", folder.c_str());
                            }
                        }
                    }
                    ImGui::Spacing();
                    if (detectedWimFile.empty()) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Please select the drive/folder containing the Windows files.");
                    }
                    else {
                        // Shows the user selected list of versions found in the image
                        ImGui::Text("Select Edition:");
                        std::vector<const char*> comboItems;
                        for (const auto& name : editionNames) comboItems.push_back(name.c_str());
                        ImGui::SetNextItemWidth(350.0f);
                        if (ImGui::Combo("##Editions", &selectedEditionComboIndex, comboItems.data(), static_cast<int>(comboItems.size()))) {
                            wimIndex = selectedEditionComboIndex + 1;
                        }
                        ImGui::Dummy(ImVec2(0.0f, 20.0f));
                        if (isTaskRunning) ImGui::Text("Working...");
                        else if (ImGui::Button("EXTRACT WIM & COPY FILES", ImVec2(350.0f, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(Task_Extract); t.detach(); }
                    }
                }
                else if (selectedOption == 1) {
                    // Opens up the image file so it can be edited
                    ImGui::Text("Mounts install.wim to PATH");
                    if (isTaskRunning) ImGui::Text("Working...");
                    else if (ImGui::Button("MOUNT WIM", ImVec2(350.0f, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(Task_Mount); t.detach(); }
                }
                else if (selectedOption == 2) { DrawChecklistUI("Manage Pre-Installed Apps (Appx)", detectedApps, Task_ScanApps, "REMOVE CHECKED APPS", Task_RemoveSelectedApps); }
                else if (selectedOption == 3) { DrawChecklistUI("Manage System Packages", detectedPackages, Task_ScanPackages, "REMOVE CHECKED PACKAGES", Task_RemoveSelectedPackages); }
                else if (selectedOption == 4) { DrawChecklistUI("Manage Windows Features", detectedFeatures, Task_ScanFeatures, "ENABLE SELECTED", Task_EnableSelectedFeatures, "DISABLE SELECTED", Task_DisableSelectedFeatures); }
                else if (selectedOption == 5) {
                    // Loads registry files for modifications
                    ImGui::Text("Select Registry Hive to Edit:");
                    ImGui::SetNextItemWidth(350.0f);
                    ImGui::Combo("##HiveSel", &selectedHiveIndex, hiveList, hiveList_Size);
                    ImGui::Spacing();
                    if (isTaskRunning) ImGui::Text("Working...");
                    else if (ImGui::Button("MOUNT HIVE & EDIT", ImVec2(350.0f, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(Task_Registry); t.detach(); }
                }
                else if (selectedOption == 6) {
                    // Moves the local files or folders into the ISO
                    ImGui::Text("Inject Custom File or Folder");
                    ImGui::TextWrapped("Selected items will be copied to the root (C:\\) of the image.");
                    ImGui::Spacing();
                    ImGui::Text("Source Path:");
                    ImGui::PushItemWidth(350.0f);
                    ImGui::InputText("##SrcFile", customFileSource, MAX_PATH, ImGuiInputTextFlags_ReadOnly);
                    ImGui::PopItemWidth();
                    ImGui::Spacing();

                    if (ImGui::Button("Select File", ImVec2(170.0f, 30.0f))) {
                        if (OpenFileDialog(customFileSource, MAX_PATH, "All Files\0*.*\0")) {
                            isCustomSourceAFolder = false;
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Select Folder", ImVec2(170.0f, 30.0f))) {
                        std::string folder = PickFolderDialog();
                        if (!folder.empty()) {
                            strncpy_s(customFileSource, MAX_PATH, folder.c_str(), _TRUNCATE);
                            isCustomSourceAFolder = true;
                        }
                    }

                    ImGui::Spacing();

                    if (isTaskRunning) ImGui::Text("Working...");
                    else {
                        if (strlen(customFileSource) == 0) ImGui::BeginDisabled();
                        if (ImGui::Button("ADD TO IMAGE", ImVec2(350.0f, 40.0f))) {
                            isTaskRunning = true;
                            stopRequested = false;
                            std::thread t(Task_AddCustomFile);
                            t.detach();
                        }
                        if (strlen(customFileSource) == 0) ImGui::EndDisabled();
                    }

                    ImGui::Spacing();
                    ImGui::TextDisabled("Right-Click Menu Options:");
                    ImGui::Spacing();

                    if (disabledMode) ImGui::BeginDisabled();

                    if (ImGui::Button("Install 'Take Ownership' Context Menu", ImVec2(350.0f, 30.0f))) {
                        isTaskRunning = true;
                        stopRequested = false;
                        std::thread t(Task_ManageContextTakeOwn, true);
                        t.detach();
                    }
                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
                    if (ImGui::Button("Remove", ImVec2(80.0f, 30.0f))) {
                        isTaskRunning = true;
                        stopRequested = false;
                        std::thread t(Task_ManageContextTakeOwn, false);
                        t.detach();
                    }
                    ImGui::PopStyleColor();

                    if (disabledMode) ImGui::EndDisabled();

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();

                    if (ImGui::Button("Open Path Folder", ImVec2(350.0f, 30.0f))) {
                        std::string path = g_InstallDir + "\\PATH";
                        if (DirectoryExists(path)) {
                            ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
                        }
                        else {
                            MessageBoxA(NULL, "The PATH folder does not exist.\nPlease Mount a WIM first.", "Error", MB_OK | MB_ICONERROR);
                        }
                    }
                }
                else if (selectedOption == 7) {
                    // Finalizes all changes and turns the workspace into a bootable disk
                    ImGui::Text("ISO Save Path:");
                    ImGui::PushItemWidth(350.0f);
                    ImGui::InputText("##OutPath", outputIsoPath, MAX_PATH);
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    if (ImGui::Button("Save As")) SaveFileDialog(outputIsoPath, MAX_PATH);
                    ImGui::Spacing();
                    ImGui::Checkbox("Run Cleanup (StartComponentCleanup)", &buildOpt_Cleanup);

                    // TPM Bypass & Warning
                    ImGui::Checkbox("Enable TPM Bypass", &buildOpt_TPM);
                    if (buildOpt_TPM) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                        ImGui::TextWrapped("WARNING: This injects a registry hack to bypass system requirements, which may impact your ability to receive updates or cause other problems in the future. Only use if you absolutely need to.");
                        ImGui::PopStyleColor();
                        ImGui::Spacing();
                    }

                    ImGui::Checkbox("Skip 'Press any key' (Instant Boot)", &buildOpt_SkipPrompt);
                    ImGui::Checkbox("Enable Unattended Install", &buildOpt_Unattend);
                    if (buildOpt_Unattend) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                        ImGui::TextWrapped("WARNING: Checking this will AUTOMATICALLY WIPE the entire installation drive of the computer / vm you are installing to! Then Windows will automatically install itself.");
                        ImGui::TextWrapped("WARNING: Credentials saved in plaintext on the ISO.Remember to delete your ISO or USB drive after installation for your safety.");
                        ImGui::PopStyleColor();
                        ImGui::Indent(20.0f);
                        ImGui::Spacing();
                        ImGui::PushItemWidth(250.0f);
                        ImGui::InputText("Username", unattendUser, IM_ARRAYSIZE(unattendUser));
                        ImGui::InputText("Password", unattendPass, IM_ARRAYSIZE(unattendPass), ImGuiInputTextFlags_Password);
                        ImGui::PopItemWidth();
                        ImGui::Checkbox("Auto-Login (Always Stay Logged In)", &buildOpt_AutoLogin);
                        if (buildOpt_AutoLogin) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
                            ImGui::TextWrapped("WARNING: This PC will auto-unlock for anyone physically present.");
                            ImGui::PopStyleColor();
                        }
                        ImGui::Unindent(20.0f);
                    }
                    ImGui::Spacing();
                    ImGui::Text("Compression:");
                    ImGui::RadioButton("ESD (Small/Slow)", &buildOpt_Format, 0);
                    ImGui::SameLine();
                    ImGui::RadioButton("WIM (Large/Fast)", &buildOpt_Format, 1);
                    ImGui::Spacing();
                    if (isTaskRunning) ImGui::Text("Working...");
                    else if (ImGui::Button("BUILD ISO", ImVec2(350.0f, 40.0f))) { isTaskRunning = true; stopRequested = false; std::thread t(Task_BuildISO); t.detach(); }
                }
                else if (selectedOption == 8) {
                    // Forces the image to close if something gets stuck
                    ImGui::Text("Emergency / Force Unmount");
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Warning: Default action discards unsaved changes.");

                    ImGui::Spacing();

                    if (disabledMode) ImGui::BeginDisabled();
                    ImGui::Checkbox("Save changes to WIM before unmounting?", &unmount_SaveChanges);

                    if (unmount_SaveChanges) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Changes will be saved to install.wim.");
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Changes will be DISCARDED.");
                    }
                    if (disabledMode) ImGui::EndDisabled();

                    ImGui::Spacing();
                    if (isTaskRunning) ImGui::Text("Working...");
                    else if (ImGui::Button("UNMOUNT", ImVec2(350.0f, 40.0f))) {
                        isTaskRunning = true;
                        stopRequested = false;
                        std::thread t(Task_Unmount);
                        t.detach();
                    }
                }
                else if (selectedOption == 9) {
                    // Wipes the app and all its data from the computer
                    ImGui::Text("Uninstall MODWIN");
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Warning: This will delete ALL MODWIN files and data!");

                    bool mounted = IsWimMounted();
                    if (mounted) {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "UNMOUNT REQUIRED: Please unmount the image before uninstalling.");
                    }

                    if (isTaskRunning) ImGui::Text("Working...");
                    else {
                        if (mounted) ImGui::BeginDisabled();
                        if (ImGui::Button("UNINSTALL MODWIN", ImVec2(350.0f, 40.0f))) {
                            isTaskRunning = true;
                            stopRequested = false;
                            std::thread t(Task_Uninstall);
                            t.detach();
                        }
                        if (mounted) ImGui::EndDisabled();
                    }
                }
                else if (selectedOption == 10) {
                    DrawCreditsUI();
                }
                ImGui::EndChild();
                if (ImGui::CollapsingHeader("Console", ImGuiTreeNodeFlags_DefaultOpen)) myLog.DrawContent();
                ImGui::End();
            }
            ImGui::Render();
            // Cleans up the frame before drawing the next one
            const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            g_pSwapChain->Present(1, 0);
        }

        // Shuts down and releases memory
        CoUninitialize();
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Critical Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    catch (...) {
        MessageBoxA(NULL, "An unknown error occurred.", "Critical Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
