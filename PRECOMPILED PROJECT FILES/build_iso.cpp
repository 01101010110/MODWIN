#include "modwin.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Sets default build configuration flags.
bool buildOpt_Cleanup = true;
int buildOpt_Format = 1;
bool buildOpt_SkipPrompt = false;
bool buildOpt_TPM = false;
bool buildOpt_Unattend = false;
bool buildOpt_WipeDisk = true;
bool buildOpt_AutoLogin = false;

// Stores default credentials for unattended user creation.
char unattendUser[64] = "User";
char unattendPass[64] = "";

// Holds path and type information for custom file injection.
char customFileSource[MAX_PATH] = "";
bool isCustomSourceAFolder = false;

// Base XML template 
const char* INTERNAL_AUTOUNATTEND = R"(<?xml version="1.0" encoding="utf-8"?>
<unattend xmlns="urn:schemas-microsoft-com:unattend">
    <settings pass="windowsPE">
        <component name="Microsoft-Windows-International-Core-WinPE" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <SetupUILanguage><UILanguage>en-US</UILanguage></SetupUILanguage>
            <InputLocale>0409:00000409</InputLocale>
            <SystemLocale>en-US</SystemLocale>
            <UILanguage>en-US</UILanguage>
            <UserLocale>en-US</UserLocale>
        </component>
        <component name="Microsoft-Windows-Setup" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <UpgradeData>
                <Upgrade>false</Upgrade>
                <WillShowUI>Never</WillShowUI>
            </UpgradeData>
            %DISK_CONFIGURATION_BLOCK%
            %RUN_SYNC_PE%
            <UserData>
                <AcceptEula>true</AcceptEula>
            </UserData>
        </component>
    </settings>
    <settings pass="specialize">
        <component name="Microsoft-Windows-Shell-Setup" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <ComputerName>*</ComputerName>
            <TimeZone>Central Standard Time</TimeZone>
        </component>
        <component name="Microsoft-Windows-Deployment" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            %RUN_SYNC_SPEC%
        </component>
    </settings>
    <settings pass="oobeSystem">
        <component name="Microsoft-Windows-International-Core" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            <InputLocale>0409:00000409</InputLocale>
            <SystemLocale>en-US</SystemLocale>
            <UILanguage>en-US</UILanguage>
            <UserLocale>en-US</UserLocale>
        </component>
        <component name="Microsoft-Windows-Shell-Setup" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            %AUTOLOGON_BLOCK%
            <OOBE>
                <HideEULAPage>true</HideEULAPage>
                <HideLocalAccountScreen>true</HideLocalAccountScreen>
                <HideOnlineAccountScreens>true</HideOnlineAccountScreens>
                <HideWirelessSetupInOOBE>true</HideWirelessSetupInOOBE>
                <ProtectYourPC>3</ProtectYourPC>
            </OOBE>
            <UserAccounts>
                <LocalAccounts>
                    <LocalAccount wcm:action="add">
                        <Password><Value>%PASSWORD%</Value><PlainText>true</PlainText></Password>
                        <Description>Primary User</Description><DisplayName>%USER_NAME%</DisplayName>
                        <Group>Administrators</Group><Name>%USER_NAME%</Name>
                    </LocalAccount>
                </LocalAccounts>
            </UserAccounts>
        </component>
    </settings>
</unattend>
)";

// Minimal XML template for TPM bypass only
const char* TPM_ONLY_AUTOUNATTEND = R"(<?xml version="1.0" encoding="utf-8"?>
<unattend xmlns="urn:schemas-microsoft-com:unattend">
    <settings pass="windowsPE">
        <component name="Microsoft-Windows-Setup" processorArchitecture="amd64" publicKeyToken="31bf3856ad364e35" language="neutral" versionScope="nonSxS" xmlns:wcm="http://schemas.microsoft.com/WMIConfig/2002/State" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
            %RUN_SYNC_PE%
        </component>
    </settings>
</unattend>
)";

// Formats commands for the XML run synchronous block.
std::string BuildRunSyncBlock(const std::vector<std::string>& commands) {
    if (commands.empty()) return "";

    std::string block = "<RunSynchronous>\n";
    int order = 1;
    for (const auto& cmd : commands) {
        block += "                <RunSynchronousCommand wcm:action=\"add\">\n";
        block += "                    <Order>" + std::to_string(order++) + "</Order>\n";
        block += "                    <Path>" + cmd + "</Path>\n";
        block += "                </RunSynchronousCommand>\n";
    }
    block += "            </RunSynchronous>\n";
    return block;
}

// Injects a custom file or folder into the mounted WIM.
void Task_AddCustomFile() {
    std::string src = customFileSource;
    std::string finalDestPath = g_InstallDir + "\\PATH";

    if (!DirectoryExists(finalDestPath + "\\Windows")) {
        myLog.AddLog("[ERROR] WIM is not mounted.\n");
        isTaskRunning = false;
        return;
    }

    if (!fs::exists(src)) {
        myLog.AddLog("[ERROR] Source path does not exist: %s\n", src.c_str());
        isTaskRunning = false;
        return;
    }

    std::string cmd;
    if (isCustomSourceAFolder) {
        if (!fs::is_directory(src)) {
            myLog.AddLog("[ERROR] Path is marked as a folder but is not one: %s\n", src.c_str());
            isTaskRunning = false;
            return;
        }
        myLog.AddLog("[INFO] Copying folder to image root...\n");
        cmd = "xcopy /E /I /Y /Q \"" + src + "\" \"" + finalDestPath + "\\" + fs::path(src).filename().string() + "\"";
    }
    else {
        if (!fs::is_regular_file(src)) {
            myLog.AddLog("[ERROR] Path is marked as a file but is not one: %s\n", src.c_str());
            isTaskRunning = false;
            return;
        }
        myLog.AddLog("[INFO] Copying file to image root...\n");
        cmd = "copy /Y \"" + src + "\" \"" + finalDestPath + "\"";
    }

    RunCommand(cmd);

    if (!stopRequested) myLog.AddLog("[SUCCESS] Item copied to image root.\n");
    isTaskRunning = false;
}

// Processes WIM and builds the final ISO.
void Task_BuildISO() {
    try {
        std::string target = outputIsoPath;
        if (target.empty()) { myLog.AddLog("[ERROR] No Output Path specified.\n"); isTaskRunning = false; return; }

        if (FileExists(target)) {
            myLog.AddLog("[INFO] Target ISO exists. Attempting to remove...\n");
            try {
                if (!std::filesystem::remove(target)) {
                    myLog.AddLog("[ERROR] Failed to delete existing ISO. It may be locked by another program.\n");
                    isTaskRunning = false;
                    return;
                }
            }
            catch (const std::exception& e) {
                myLog.AddLog("[ERROR] Exception while deleting ISO: %s\n", e.what());
                isTaskRunning = false;
                return;
            }
        }

        myLog.AddLog("[INFO] Starting Build Process...\n");

        if (buildOpt_Cleanup) {
            myLog.AddLog("[INFO] Running Component Cleanup...\n");
            if (DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
                std::string cleanCmd = "dism /Image:\"" + g_InstallDir + "\\PATH\" /cleanup-image /StartComponentCleanup /ResetBase";
                RunCommand(cleanCmd);
            }
            else {
                myLog.AddLog("[WARNING] WIM not mounted. Skipping cleanup.\n");
            }
        }

        if (stopRequested) { isTaskRunning = false; return; }

        if (DirectoryExists(g_InstallDir + "\\PATH\\Windows")) {
            myLog.AddLog("[INFO] Unmounting and Committing WIM...\n");
            std::string unmountCmd = "dism /Unmount-Image /MountDir:\"" + g_InstallDir + "\\PATH\" /Commit";
            RunCommand(unmountCmd);
        }

        std::string sourceWim = g_InstallDir + "\\install.wim";
        std::string destFile;
        std::string compressType;

        if (buildOpt_Format == 0) {
            destFile = g_InstallDir + "\\install.esd";
            compressType = "recovery";
            myLog.AddLog("[INFO] Compressing WIM to ESD (This is slow)...\n");
        }
        else {
            destFile = g_InstallDir + "\\install_opt.wim";
            compressType = "max";
            myLog.AddLog("[INFO] Exporting optimized WIM...\n");
        }

        std::string compressCmd = "dism /export-image /SourceImageFile:\"" + sourceWim + "\" /SourceIndex:1 /DestinationImageFile:\"" + destFile + "\" /Compress:" + compressType + " /CheckIntegrity";
        RunCommand(compressCmd);

        if (stopRequested) { isTaskRunning = false; return; }

        std::string isoSourceDir = g_InstallDir + "\\ISO";
        if (!DirectoryExists(isoSourceDir)) fs::create_directories(isoSourceDir);

        if (buildOpt_Unattend || buildOpt_TPM) {
            int wimIndex = 1;
            std::string xmlContent;
            myLog.AddLog("[INFO] Generating autounattend.xml...\n");

            if (buildOpt_Unattend) {
                // Full unattended template with user creation, OOBE bypass, etc.
                xmlContent = std::string(INTERNAL_AUTOUNATTEND);
            }
            else {
                // TPM only minimal template that just runs bypass commands during PE
                xmlContent = std::string(TPM_ONLY_AUTOUNATTEND);
                myLog.AddLog("[INFO] TPM bypass only mode (no unattended install).\n");
            }

            fs::create_directories(isoSourceDir + "\\sources");

            // ei.cfg and disk/partition config will only be used for unattended installs
            if (buildOpt_Unattend) {
                std::ofstream eiFile(isoSourceDir + "\\sources\\ei.cfg");
                if (eiFile.is_open()) {
                    eiFile << "[EditionID]\n\n[Channel]\nRetail\n[VL]\n0";
                    eiFile.close();
                    myLog.AddLog("[INFO] Generated ei.cfg to bypass BIOS edition checks.\n");
                }

                std::string createPartitionsLogic =
                    "                        <CreatePartition wcm:action=\"add\">\n"
                    "                            <Order>1</Order>\n"
                    "                            <Type>EFI</Type>\n"
                    "                            <Size>100</Size>\n"
                    "                        </CreatePartition>\n"
                    "                        <CreatePartition wcm:action=\"add\">\n"
                    "                            <Order>2</Order>\n"
                    "                            <Type>MSR</Type>\n"
                    "                            <Size>16</Size>\n"
                    "                        </CreatePartition>\n"
                    "                        <CreatePartition wcm:action=\"add\">\n"
                    "                            <Order>3</Order>\n"
                    "                            <Type>Primary</Type>\n"
                    "                            <Extend>true</Extend>\n"
                    "                        </CreatePartition>\n";

                std::string modifyPartitionsLogic =
                    "                        <ModifyPartition wcm:action=\"add\">\n"
                    "                            <Order>1</Order>\n"
                    "                            <PartitionID>1</PartitionID>\n"
                    "                            <Label>System</Label>\n"
                    "                            <Format>FAT32</Format>\n"
                    "                        </ModifyPartition>\n"
                    "                        <ModifyPartition wcm:action=\"add\">\n"
                    "                            <Order>2</Order>\n"
                    "                            <PartitionID>2</PartitionID>\n"
                    "                        </ModifyPartition>\n"
                    "                        <ModifyPartition wcm:action=\"add\">\n"
                    "                            <Order>3</Order>\n"
                    "                            <PartitionID>3</PartitionID>\n"
                    "                            <Label>Windows</Label>\n"
                    "                            <Letter>C</Letter>\n"
                    "                            <Format>NTFS</Format>\n"
                    "                        </ModifyPartition>\n";

                std::string diskConfigBlock = "";

                if (buildOpt_WipeDisk) {
                    myLog.AddLog("[INFO] Disk Wipe ENABLED (Destructive formatting set via XML).\n");
                    diskConfigBlock =
                        "            <DiskConfiguration>\n"
                        "                <WillShowUI>Never</WillShowUI>\n"
                        "                <Disk wcm:action=\"add\">\n"
                        "                    <DiskID>0</DiskID>\n"
                        "                    <WillWipeDisk>true</WillWipeDisk>\n"
                        "                    <CreatePartitions>\n"
                        + createPartitionsLogic +
                        "                    </CreatePartitions>\n"
                        "                    <ModifyPartitions>\n"
                        + modifyPartitionsLogic +
                        "                    </ModifyPartitions>\n"
                        "                </Disk>\n"
                        "            </DiskConfiguration>\n"
                        "            <ImageInstall>\n"
                        "                <OSImage>\n"
                        "                    <InstallFrom>\n"
                        "                        <MetaData wcm:action=\"add\">\n"
                        "                            <Key>/IMAGE/INDEX</Key>\n"
                        "                            <Value>" + std::to_string(wimIndex) + "</Value>\n"
                        "                        </MetaData>\n"
                        "                    </InstallFrom>\n"
                        "                    <InstallTo>\n"
                        "                        <DiskID>0</DiskID>\n"
                        "                        <PartitionID>3</PartitionID>\n"
                        "                    </InstallTo>\n"
                        "                    <WillShowUI>Never</WillShowUI>\n"
                        "                </OSImage>\n"
                        "            </ImageInstall>";
                }
                else {
                    myLog.AddLog("[INFO] Disk Wipe DISABLED. Setup will ask for an install partition.\n");
                    diskConfigBlock =
                        "            <ImageInstall>\n"
                        "                <OSImage>\n"
                        "                    <InstallFrom>\n"
                        "                        <MetaData wcm:action=\"add\">\n"
                        "                            <Key>/IMAGE/INDEX</Key>\n"
                        "                            <Value>" + std::to_string(wimIndex) + "</Value>\n"
                        "                        </MetaData>\n"
                        "                    </InstallFrom>\n"
                        "                    <WillShowUI>OnError</WillShowUI>\n"
                        "                </OSImage>\n"
                        "            </ImageInstall>";
                }

                ReplaceStringInPlace(xmlContent, "%DISK_CONFIGURATION_BLOCK%", diskConfigBlock);
            }

            // TPM bypass commands go into the PE phase
            std::vector<std::string> peCommands;

            if (buildOpt_TPM) {
                std::vector<std::string> tpmCmds = {
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\LabConfig /v BypassTPMCheck /t REG_DWORD /d 1 /f",
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\LabConfig /v BypassSecureBootCheck /t REG_DWORD /d 1 /f",
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\LabConfig /v BypassRAMCheck /t REG_DWORD /d 1 /f",
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\LabConfig /v BypassStorageCheck /t REG_DWORD /d 1 /f",
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\LabConfig /v BypassCPUCheck /t REG_DWORD /d 1 /f",
                    "cmd.exe /c reg add HKLM\\SYSTEM\\Setup\\MoSetup /v AllowUpgradesWithUnsupportedTPMOrCPU /t REG_DWORD /d 1 /f"
                };
                peCommands.insert(peCommands.end(), tpmCmds.begin(), tpmCmds.end());
            }

            ReplaceStringInPlace(xmlContent, "%RUN_SYNC_PE%", BuildRunSyncBlock(peCommands));

            // Specialize pass, OOBE, user accounts only for unattended mode
            if (buildOpt_Unattend) {
                std::vector<std::string> specCommands;
                specCommands.push_back("cmd.exe /c reg add HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OOBE /v BypassNRO /t REG_DWORD /d 1 /f");

                ReplaceStringInPlace(xmlContent, "%RUN_SYNC_SPEC%", BuildRunSyncBlock(specCommands));

                std::string autoLogonBlock;

                if (buildOpt_AutoLogin) {
                    autoLogonBlock =
                        "            <AutoLogon>\n"
                        "                <Password>\n"
                        "                    <Value>%PASSWORD%</Value>\n"
                        "                    <PlainText>true</PlainText>\n"
                        "                </Password>\n"
                        "                <Enabled>true</Enabled>\n"
                        "                <LogonCount>9999999</LogonCount>\n"
                        "                <Username>%USER_NAME%</Username>\n"
                        "            </AutoLogon>";

                    myLog.AddLog("[INFO] Auto-Login enabled.\n");
                }
                else {
                    autoLogonBlock = "";
                    myLog.AddLog("[INFO] Auto-Login disabled (User will be created, but requires manual login).\n");
                }

                ReplaceStringInPlace(xmlContent, "%AUTOLOGON_BLOCK%", autoLogonBlock);
                ReplaceStringInPlace(xmlContent, "%USER_NAME%", std::string(unattendUser));
                ReplaceStringInPlace(xmlContent, "%PASSWORD%", std::string(unattendPass));
            }

            std::ofstream xmlFile(isoSourceDir + "\\autounattend.xml");
            if (xmlFile.is_open()) { xmlFile << xmlContent; xmlFile.close(); }
        }

        if (FileExists(isoSourceDir + "\\sources\\cversion.ini")) {
            fs::remove(isoSourceDir + "\\sources\\cversion.ini");
            myLog.AddLog("[INFO] Removed cversion.ini to fully bypass upgrade blocks.\n");
        }

        myLog.AddLog("[INFO] Generating ISO...\n");
        std::string mkisofsPath = g_InstallDir + "\\BIN\\MKISOFS\\mkisofs.exe";
        if (!FileExists(mkisofsPath)) {
            myLog.AddLog("[ERROR] mkisofs.exe NOT FOUND at: %s\n", mkisofsPath.c_str());
            isTaskRunning = false;
            return;
        }

        std::string finalInstallFile = (buildOpt_Format == 0) ? "install.esd" : "install.wim";
        std::string exportedFile = (buildOpt_Format == 0) ? (g_InstallDir + "\\install.esd") : (g_InstallDir + "\\install_opt.wim");
        fs::create_directories(isoSourceDir + "\\sources");

        std::string oldWim = isoSourceDir + "\\sources\\install.wim";
        std::string oldEsd = isoSourceDir + "\\sources\\install.esd";
        if (FileExists(oldWim)) fs::remove(oldWim);
        if (FileExists(oldEsd)) fs::remove(oldEsd);

        std::string targetInstallPath = isoSourceDir + "\\sources\\" + finalInstallFile;
        try {
            fs::rename(exportedFile, targetInstallPath);
            myLog.AddLog("[INFO] Moved %s to ISO/sources...\n", finalInstallFile.c_str());
        }
        catch (const std::exception& e) {
            myLog.AddLog("[ERROR] Failed to move install file: %s\n", e.what());
        }

        std::string biosBootFile = "boot/etfsboot.com";
        std::string efiBootFile = buildOpt_SkipPrompt ? "efi/microsoft/boot/efisys_noprompt.bin" : "efi/microsoft/boot/efisys.bin";
        if (buildOpt_SkipPrompt && !FileExists(isoSourceDir + "\\efi\\microsoft\\boot\\efisys_noprompt.bin")) {
            myLog.AddLog("[WARNING] efisys_noprompt.bin missing, using standard efisys.bin\n");
            efiBootFile = "efi/microsoft/boot/efisys.bin";
        }

        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        SetCurrentDirectoryA((g_InstallDir + "\\BIN\\MKISOFS").c_str());
        std::string cmd = ".\\mkisofs.exe -iso-level 3 -udf -D -N -joliet-long -relaxed-filenames "
            "-b \"" + biosBootFile + "\" -no-emul-boot -boot-load-size 8 "
            "-eltorito-alt-boot -b \"" + efiBootFile + "\" -no-emul-boot -boot-load-size 1 "
            "-o \"" + target + "\" \"" + isoSourceDir + "\"";
        RunCommand(cmd);
        SetCurrentDirectoryA(currentDir);

        if (!stopRequested) myLog.AddLog("[SUCCESS] ISO Build process finished.\n");
        isTaskRunning = false;
    }
    catch (const std::exception& e) {
        myLog.AddLog("[CRITICAL ERROR] Thread Exception: %s\n", e.what());
        isTaskRunning = false;
    }
    catch (...) {
        myLog.AddLog("[CRITICAL ERROR] Unknown Thread Exception.\n");
        isTaskRunning = false;
    }
}
