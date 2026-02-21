#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h> 
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "database.h" 

namespace fs = std::filesystem;

// Holds the name and status for items in a list
struct ItemData {
    std::string Name;
    std::string ExtraInfo;
    int SafetyColor;       
    bool IsSelected;
    std::string State;     
    bool ShowDetails;      
};

// Manages the text history for the on screen console
struct AppLog {
    std::vector<std::string> Lines;
    bool AutoScroll;
    bool ScrollToBottom;
    AppLog();
    void Clear();
    void AddLog(const char* fmt, ...);
    void DrawContent();
};

// Global paths and shared app variables
extern std::string g_InstallDir;
extern std::string g_IniPath;
extern int selectedOption;
extern int wimIndex;
extern char inputSourcePath[MAX_PATH];
extern std::string detectedWimFile;
extern char outputIsoPath[MAX_PATH];
extern std::vector<std::string> editionNames;
extern int selectedEditionComboIndex;

// Preferences for creating the ISO
extern bool buildOpt_Cleanup;
extern int buildOpt_Format;
extern bool buildOpt_SkipPrompt;
extern bool buildOpt_TPM;
extern bool buildOpt_Unattend;
extern bool buildOpt_WipeDisk;
extern bool buildOpt_UseProductKey;
extern bool buildOpt_AutoLogin;

// Choice for saving work when closing the image
extern bool unmount_SaveChanges;

// Details for the automated Windows setup
extern char unattendUser[64];
extern char unattendPass[64];
extern char unattendProductKey[30];

// Settings for adding local items into the image
extern char customFileSource[MAX_PATH];
extern char customFileDest[MAX_PATH];
extern bool isCustomSourceAFolder;

// Lists that store the results from the image scans
extern std::vector<ItemData> detectedApps;
extern std::vector<ItemData> detectedPackages;
extern std::vector<ItemData> detectedFeatures;
extern char searchFilter[128];

// Safety locks to keep background work from clashing
extern std::atomic<bool> isTaskRunning;
extern std::atomic<bool> stopRequested;
extern std::mutex logMutex;
extern std::mutex dataMutex;
extern AppLog myLog;

// Pointers for the graphics hardware
extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_mainRenderTargetView;

// Variables for picking a registry file
extern int selectedHiveIndex;
extern const char* hiveList[];
extern int hiveList_Size;

// Prepares the window and graphics engine
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Tools for managing files and the system registry
void SetupInstallDirectory();
void BuildModwinFolder();
std::string GetRegistryInstallPath();
void SetRegistryInstallPath(const std::string& path);
bool IsUserAdmin();
bool FileExists(const std::string& filename);
bool DirectoryExists(const std::string& dirName);
bool IsWimMounted();
std::string PickFolderDialog();
bool OpenFileDialog(char* buffer, int bufferSize, const char* filter);
bool SaveFileDialog(char* buffer, int bufferSize);

// Executes system commands silently
void RunCommand(std::string cmd);
std::string RunCommandCapture(std::string cmd);
void DrawFireText(const char* text);
void ReplaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
void ScanWimEditions();
void DrawChecklistUI(const char* label, std::vector<ItemData>& items, void(*scanFunc)(), const char* btnLabel1, void(*actionFunc1)(), const char* btnLabel2 = nullptr, void(*actionFunc2)() = nullptr);
void DrawCreditsUI();

// Tasks for iso modding
void Task_Extract();
void Task_Mount();
void Task_Unmount();
void Task_ScanApps();
void Task_RemoveSelectedApps();
void Task_ScanPackages();
void Task_RemoveSelectedPackages();
void Task_ScanFeatures();
void Task_EnableSelectedFeatures();
void Task_DisableSelectedFeatures();
void Task_Registry();
void SetRegeditLastKey(std::string keyName);
void Task_AddCustomFile();
void Task_BuildISO();
void Task_Uninstall();
void Task_ManageContextTakeOwn(bool install);