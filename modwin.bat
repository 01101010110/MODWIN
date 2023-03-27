@echo off
:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)
 
REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )
 
:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"
 
    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B
 
:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
@echo off
title [MODWIN <3]
color 0A
if "%1" neq "" ( goto %1)

:MENU
cls
echo 1111   1111  000000   111111     00  00  00  111111  00      00   
echo 11 11 11 11 00    00  111  111   00  00  00    11    0000    00  Version:
echo 11  111  11 00    00  111   111  00  00  00    11    00 00   00  3.2 
echo 11   11  11 00    00  111   111  00  00  00    11    00  00  00  
echo 11       11 00    00  111   111  00  00  00    11    00   00 00  Thanks Indospot!
echo 11       11  000000   11111111   0000000000  111111  00    0000  And Xazac!
echo ===============================================================
echo Brought to you by Jenneh and Cats~!   
echo ===================================                       
echo Options:
echo ===================================
echo 1. Run An All-In-One Removal Script
echo 2. Source WIM and Extract
echo 3. Mount WIM
echo 4. Apps, Packages, and Features Options
echo 5. Mount WIM Registry (Optional: for those that need)
echo 6. Push USER folder to WIM
echo 7. Unmount WIM and Build ISO Options
echo 8. Exit
echo =============================================
set /p answer=Type a number above and press enter: 
if %answer%==1 goto SCRIPTS
if %answer%==2 goto SOURCE_WIM
if %answer%==3 goto MOUNT_WIM
if %answer%==4 goto APP_PACK
if %answer%==5 goto REG
if %answer%==6 goto USER
if %answer%==7 goto BUILD_OPTIONS
if %answer%==8 goto EXIT
pause

:SOURCE_WIM
cls
echo Does your ISO contain an ESD or a WIM?
echo ======================================
echo Type 1 for ESD
echo Type 2 for WIM
echo Type 3 to return to menu
echo ======================================
set /p answer=Type a number and press enter: 
if %answer%==1 goto ESD
if %answer%==2 goto WIM
if %answer%==3 goto MENU
pause

:ESD
cls
echo Here are your Options:
echo ======================
Dism /Get-WimInfo /WimFile:"C:\MODWIN\ISO\sources\install.esd"
echo ==============================================================
set /p answer=Type Source Index Number and Press Enter: 
dism /export-image /SourceImageFile:"C:\MODWIN\ISO\sources\install.esd" /SourceIndex:%answer% /DestinationImageFile:"C:\MODWIN\ISO\sources\install.wim" /Compress:max /CheckIntegrity
pause
del :"C:\MODWIN\ISO\sources\install.esd"
goto MENU

:WIM
cls
echo Here are your Options:
echo ======================
Dism /Get-WimInfo /WimFile:"C:\MODWIN\ISO\sources\install.wim"
echo ==========================================================
set /p answer=Type Source Index Number and press enter: 
dism /export-image /SourceImageFile:"C:\MODWIN\ISO\sources\install.wim" /SourceIndex:%answer% /DestinationImageFile:"C:\MODWIN\ISO\sources\install1.wim" /Compress:max /CheckIntegrity
pause
del "C:\MODWIN\ISO\sources\install.wim"
ren C:\MODWIN\ISO\sources\install1.wim install.wim
goto MENU

:MOUNT_WIM
cls
echo Mounting the WIM~!
echo ==================
dism.exe /mount-wim /wimfile:"C:\MODWIN\ISO\sources\install.wim" /mountdir:"C:\MODWIN\PATH" /index:1
goto MENU

:APP_PACK
cls
echo Here are your Options:
echo =======================
echo Press 1 to see APP PACKAGES
echo Press 2 to see PACKAGES
echo Press 3 to see FEATURES
echo Press 4 to return to the MENU
echo ==============================
set /p answer=Type a number and press enter: 
if %answer%==1 goto APPS
if %answer%==2 goto PACKAGES
if %answer%==3 goto FEATURES
if %answer%==4 goto MENU
pause

:APPS
cls
C:\Windows\System32\dism.exe /Image:C:\MODWIN\PATH /Get-ProvisionedAppxPackages > C:\MODWIN\apps.txt
find "PackageName : " C:\MODWIN\apps.txt > C:\MODWIN\newapps.txt
type C:\MODWIN\newapps.txt
echo ==============================
echo Would You Like to Remove Apps?
echo ==============================
echo Press 1 for yes
echo Press 2 for no
echo Press 3 to Add a App
echo ==============================
set /p answer=Type a number and press enter: 
if %answer%==1 goto APP_REMOVAL
if %answer%==2 goto APP_EXIT
if %answer%==3 goto APP_ADD

:APP_ADD
cls
dir C:\MODWIN\APPS
set /p answer=Paste your app name here: 
dism /Image:C:\MODWIN\PATH -Add-ProvisionedAppxPackage -packagePath:C:\MODWIN\APPS\%answer% /SkipLicense
echo Do you want to add more?
echo Press 1 for yes
echo Press 2 for no
set /p answer=Type a number and press enter: 
if %answer%==1 goto APP_ADD
if %answer%==2 goto APP_EXIT

:APP_REMOVAL
echo ======================
echo Paste Your App Below
echo ======================
set /p answer= 
dism /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:%answer%
pause
goto APPS

:APP_EXIT
del  C:\MODWIN\apps.txt
del  C:\MODWIN\newapps.txt
goto MENU

:PACKAGES
cls
dism /Image:C:\MODWIN\PATH /Get-Packages > C:\MODWIN\packages.txt
find "Package Identity : " C:\MODWIN\packages.txt > C:\MODWIN\newpackages.txt                 
type C:\MODWIN\newpackages.txt
echo ==============================
echo Would You Like to Remove Packages?
echo ==============================
echo Press 1 for yes
echo Press 2 for no
echo Press 3 to Add a Package
echo ==============================
set /p answer=Type a number and press enter: 
if %answer%==1 goto PACK_REMOVAL
if %answer%==2 goto PACK_EXIT
if %answer%==3 goto PACK_ADD

:PACK_ADD
cls
dir C:\MODWIN\PACKAGES
set /p answer=Paste your Package name here: 
dism /Image:C:\MODWIN\PATH /Add-Package /PackagePath:C:\MODWIN\PACKAGES>%answer%
echo Do you want to add more?
echo Press 1 for yes
echo Press 2 for no
set /p answer=Type a number and press enter: 
if %answer%==1 goto PACK_ADD
if %answer%==2 goto PACK_EXIT

:PACK_REMOVAL
echo ==========================
echo Paste Your Package Below
echo ==========================
set /p answer= 
dism /Image:C:\MODWIN\PATH /Remove-Package /PackageName:%answer%
pause
goto PACKAGES

:PACK_EXIT
del  C:\MODWIN\newpackages.txt
del  C:\MODWIN\packages.txt
goto MENU

:FEATURES
cls
Dism /Image:C:\MODWIN\PATH /Get-Features > C:\MODWIN\features.txt
pause
type C:\MODWIN\features.txt
echo ==================================
echo Would You Like to Remove Features?
echo ==================================
echo Press 1 for yes
echo Press 2 for no
echo Press 3 to Enable Features
echo ==============================
set /p answer=Type a number and press enter: 
if %answer%==1 goto FEATURE_REMOVAL
if %answer%==2 goto FEATURE_EXIT
if %answer%==3 goto FEATURE_ENABLE
pause

:FEATURE_ENABLE
echo =========================
echo Paste Your Feature Below
echo =========================
set /p answer= 
Dism /Image:C:\MODWIN\PATH /Enable-Feature /FeatureName:%answer%
pause
goto FEATURES

:FEATURE_REMOVAL
echo =========================
echo Paste Your Feature Below
echo =========================
set /p answer= 
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:%answer%
pause
goto FEATURES

:FEATURE_EXIT
del  C:\MODWIN\features.txt
goto MENU

:REG
cls
echo Here are your Options:
echo =======================
echo Press 1 to open the SYSTEM Registry Hive
echo Press 2 to open the SOFTWARE Registry Hive
echo Press 3 to open the DEFAULT Registry Hive
echo Press 4 to open the DRIVERS Registry Hive
echo Press 5 to open the SAM Registry Hive
echo Press 6 to UNLOAD Current Hive
echo Press 7 to return to the main MENU
echo =========================================
set /p answer=Type a number and press enter: 
if %answer%==1 goto SYSTEM
if %answer%==2 goto SOFTWARE
if %answer%==3 goto DEFAULT
if %answer%==4 goto DRIVERS
if %answer%==5 goto SAM
if %answer%==6 goto UNLOAD
if %answer%==7 goto MENU
pause

:SYSTEM
cls
echo Loading WIM's SYSTEM Registry Hive
echo ===================================
reg load HKLM\OFFLINE C:\MODWIN\PATH\Windows\System32\Config\SYSTEM
pause
regedit
goto REG

:SOFTWARE
cls
echo Loading WIM's SOFTWARE Registry Hive
echo ===================================
reg load HKLM\OFFLINE C:\MODWIN\PATH\Windows\System32\Config\SOFTWARE
pause
regedit
goto REG

:DEFAULT
cls
echo Loading WIM's DEFAULT Registry Hive
echo ===================================
reg load HKLM\OFFLINE C:\MODWIN\PATH\Windows\System32\Config\DEFAULT
pause
regedit
goto REG

:DRIVERS
cls
echo Loading WIM's DRIVERS Registry Hive
echo ===================================
reg load HKLM\OFFLINE C:\MODWIN\PATH\Windows\System32\Config\DRIVERS
pause
regedit
goto REG

:SAM
cls
echo Loading WIM's SAM Registry Hive
echo ================================
reg load HKLM\OFFLINE C:\MODWIN\PATH\Windows\System32\Config\SAM
pause
regedit
goto REG

:UNLOAD
reg unload HKLM\OFFLINE
echo Successio~!
pause
goto REG

:USER
cls
xcopy C:\MODWIN\USER C:\MODWIN\PATH /h /i /c /k /e /r /y
pause
goto menu

:BUILD_OPTIONS
cls
echo Choose an option below:
echo 1. Unmount WIM, Cleanup, Save Changes, and Build ISO
echo 2. Unmount WIM and Discard Changes (if you make a mistake :-D)
echo 3. Unmount WIM Only and Save Changes
echo 4. Build ISO Only
echo =========================================
set /p answer=Type a number and press enter: 
if %answer%==1 goto SAVE
if %answer%==2 goto DISCARD
if %answer%==3 goto UNMOUNT
if %answer%==4 goto BUILDISO

:SAVE
cls
echo Saving Changes to the WIM
echo ==========================
Dism /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
echo =========================================================================================================
echo Do you want to Compress the WIM to an ESD now? This will take some time, resulting in a smaller iso size.
echo If you do not care about file size and / or need to edit the wim again later, say no.
echo =========================================================================================================
echo type 1 for yes
echo type 2 for no
echo ========================================
set /p answer=Type a number and press enter: 
if %answer%==1 goto COMPRESS
if %answer%==2 goto BUILDISO
pause

:COMPRESS
echo Compressing WIM into ESD Now~!
echo ==============================
dism /export-image /SourceImageFile:"C:\MODWIN\ISO\sources\install.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity
del "C:\MODWIN\ISO\sources\install.wim"
goto BUILDISO

:BUILDISO
cls
echo Building your ISO Now~!
echo =======================
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\winmod.iso
echo ===================================================
echo All Done, Boss~! Check C:\MODWIN\MOD For Your ISO!
echo ===================================================
pause
goto MENU

:DISCARD
cls
echo Unmounting and discarding the changes to the WIM
echo =================================================
dism /Cleanup-mountpoints
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /discard
pause
goto MENU

:UNMOUNT
cls
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
goto MENU

:EXIT
cls
echo Have Fun!!
pause
exit /b

:SCRIPTS
cls
echo Script Options:
echo ================
echo 1. Windows 11
echo 2. Windows 10
echo 3. UUP Windows 11 - 22H2
echo 4. UUP Windows 11 - 22H1
echo 5. UUP Windows 10 - 22H2
echo 6. UUP Windows 10 - 22H1
echo 7. Main Menu
echo =========================
set /p answer=Type a number and press enter:
if %answer%==1 goto WIN11
if %answer%==2 goto WIN10
if %answer%==3 goto UUP1122
if %answer%==4 goto UUP1121
if %answer%==5 goto UUP1022
if %answer%==6 goto UUP1021
if %answer%==7 goto MENU
pause

:WIN11
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.esd" C:\MODWIN\WIM
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\install.esd" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\WIM\mod.wim" /Compress:max /CheckIntegrity
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Clipchamp.Clipchamp_2.2.8.0_neutral_~_yxz26nhyzhsrt
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_3.2204.14815.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingNews_4.2.27001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_4.53.33420.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.DesktopAppInstaller_2022.310.2333.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_10.2201.421.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_2021.2204.1.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.43012.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEVCVideoExtension_1.0.50361.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_18.2204.1141.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_4.12.3171.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_4.2.2.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Paint_11.2201.22.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2020.901.1724.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.PowerAutomateDesktop_10.0.3735.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.RawImageExtension_2.1.30391.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2022.2201.12.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SecHealthUI_1000.22621.1.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_12008.1001.113.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Todos_2.54.42772.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.50901.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_1.0.42192.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.42351.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_21.21030.25003.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2022.2202.24.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2020.2103.8.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2022.2201.4.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_16005.14326.20544.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2022.106.2230.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2022.2202.6.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsNotepad_11.2112.32.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2021.2103.28.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_22204.1400.4.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_1.23.28004.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_1.47.2385.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2.622.3232.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_12.50.6001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_1.17.29001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_1.22022.147.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_11.2202.46.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2019.22020.10021.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftCorporationII.MicrosoftFamily_0.1.28.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftCorporationII.QuickAssist_2022.414.1758.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftWindows.Client.WebExperience_421.20070.195.0_neutral_~_cw5n1h2txyewy
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GamingApp_2021.427.138.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsTerminal_3001.12.10983.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Wallpaper-Content-Extended-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WMIC-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Handwriting-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-OCR-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Speech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.525
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-TextToSpeech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.525
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-InternetPrinting-Client /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-Features /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-TCP-PortSharing45 /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45 /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2 /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure /Remove
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features /Remove
Dism /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.wim" /Compress:max /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\win11lite.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
del "C:\MODWIN\WIM\install.esd"
echo ===================================================
echo All Done, Boss~! Check C:\WINMOD\MOD For Your ISO!
echo ===================================================
pause
goto EXIT

:WIN10
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.esd" C:\MODWIN\WIM
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\install.esd" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\WIM\mod.wim" /Compress:max /CheckIntegrity
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_1.1911.21713.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_4.25.20211.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.DesktopAppInstaller_2019.125.2243.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_10.1706.13331.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.22742.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Microsoft3DViewer_6.1908.2042.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MSPaint_2019.729.2301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2019.904.1644.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_11811.1001.1813.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.22681.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_1.0.20875.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.22753.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_2019.19071.12548.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2020.1906.55.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_11910.1002.513.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_8.2.22942.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_18.1903.1152.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_4.4.8204.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_3.6.73.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MixedReality.Portal_2000.19081.1301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Office.OneNote_16001.12026.20112.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2019.305.632.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SkypeApp_14.53.77.0_neutral_~_kzf8qxf38zg5c
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Wallet_2.4.18324.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2019.807.41.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2018.826.98.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_16005.11629.20316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2019.1111.2029.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2019.716.2316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2019.716.2313.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_1.23.28002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxApp_48.49.31001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_1.46.11001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2.34.28001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_12.50.6001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_1.17.29001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_2019.430.2026.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.746
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Migration-Package~31bf3856ad364e35~amd64~~10.0.19041.1202
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.19041.1202
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.19041.1202
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-QuickAssist-Package~31bf3856ad364e35~amd64~~10.0.19041.1266
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.19041.746
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.19041.746
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.19041.1266
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1202
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.19041.964
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.19041.1266
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MSPaint-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.746
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Notepad-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1081
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Printing-WFS-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.906
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:printing-PrintToPDFServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:printing-XPSServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WindowsMediaPlayer
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:printing-Foundation-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:printing-Foundation-InternetPrinting-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Deprecation
Dism /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase 
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\win10lite.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
del "C:\MODWIN\WIM\install.esd"
echo ===================================================
echo All Done, Boss~! Check C:\WINMOD\MOD For Your ISO!
echo ===================================================
pause
goto EXIT

:UUP1122
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.wim" C:\MODWIN\WIM\mod.wim
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingNews_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Clipchamp.Clipchamp_2.2.8.0_neutral_~_yxz26nhyzhsrt
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GamingApp_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.43012.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEVCVideoExtension_1.0.50361.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Framework.1.3_1.3.24211.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Framework.1.3_1.3.24211.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Framework.1.7_1.7.27413.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Framework.2.2_2.2.29512.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Runtime.1.4_1.4.24201.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Runtime.1.7_1.7.25531.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.NET.Native.Runtime.2.2_2.2.28604.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Paint_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.PowerAutomateDesktop_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.RawImageExtension_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SecHealthUI_1000.22621.1.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Todos_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.UI.Xaml.2.1_2.11906.6001.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.UI.Xaml.2.3_2.32002.13001.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.UI.Xaml.2.4_2.42007.9001.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.UI.Xaml.2.7_7.2203.17001.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VCLibs.140.00.UWPDesktop_14.0.30704.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VCLibs.140.00_14.0.30704.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.50901.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.42351.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsTerminal_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftCorporationII.MicrosoftFamily_2022.507.447.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftCorporationII.QuickAssist_2022.507.446.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftWindows.Client.WebExperience_2022.507.446.0_neutral_~_cw5n1h2txyewy
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-FodMetadata-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Handwriting-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-OCR-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Speech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-TextToSpeech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-PowerShell-ISE-FOD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Wallpaper-Content-Extended-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WMIC-FoD-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.22621.1
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-InternetPrinting-Client /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-TCP-PortSharing45 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features /Remove
dism.exe /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase 
dism.exe /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.wim" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\UUP1122.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
echo ===================================================
echo All Done, Boss~! Check C:\WINMOD\MOD For Your ISO!
echo ===================================================
pause
goto EXIT

:UUP1121
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.wim" C:\MODWIN\WIM\mod.wim
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_2.2106.2807.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingNews_4.7.28001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_4.9.2002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GamingApp_2021.427.138.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_10.2008.32311.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_10.2.41172.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.40978.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_18.2104.12721.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_4.6.3102.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_4.1.2.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Paint_10.2104.17.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2020.901.1724.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.PowerAutomateDesktop_10.0.561.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2021.2104.2.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SecHealthUI_1000.22000.1.0_neutral__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_12008.1001.113.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Todos_2.33.33351.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.UI.Xaml.2.4_2.42007.9001.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VCLibs.140.00_14.0.29231.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.41182.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_1.0.40831.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.32731.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_21.21030.25003.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2021.2101.27.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2020.2012.21.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2020.503.58.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_16005.12827.20400.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2021.427.1821.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2021.2012.10.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2021.2012.41.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_12104.1001.113.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsTerminal_2021.226.1915.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_1.23.28002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_1.46.11001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2.50.24002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_12.50.6001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_1.17.29001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_2019.430.2026.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_2019.21012.10511.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2019.21012.10511.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:MicrosoftWindows.Client.WebExperience_321.14700.0.9_neutral_~_cw5n1h2txyewy
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-FodMetadata-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Handwriting-en-us-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-OCR-en-us-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Speech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-TextToSpeech-en-us-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-PowerShell-ISE-FOD-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Printing-WFS-FoD-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-QuickAssist-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.22000.1
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-InternetPrinting-Client /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-Features /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-TCP-PortSharing45 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2 /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure /Remove
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features /Remove
dism.exe /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase 
dism.exe /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\UUP1121.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
echo ===================================================
echo All Done, Boss~! Check C:\WINMOD\MOD For Your ISO!
echo ===================================================
pause
goto EXIT

:UUP1022
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.wim" C:\MODWIN\WIM\mod.wim
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_1.1911.21713.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_4.25.20211.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.DesktopAppInstaller_2019.125.2243.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_10.1706.13331.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_8.2.22942.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.22742.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Microsoft3DViewer_6.1908.2042.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_18.1903.1152.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_4.4.8204.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_3.6.73.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MixedReality.Portal_2000.19081.1301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MSPaint_2019.729.2301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Office.OneNote_16001.12026.20112.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2019.305.632.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2019.904.1644.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SkypeApp_14.53.77.0_neutral_~_kzf8qxf38zg5c
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_11811.1001.1813.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VCLibs.140.00_14.0.27323.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.22681.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Wallet_2.4.18324.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_1.0.20875.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.22753.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_2019.19071.12548.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2019.807.41.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2020.1906.55.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2018.826.98.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_16005.11629.20316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2019.1111.2029.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2019.716.2316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2019.716.2313.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_11910.1002.513.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_1.23.28002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxApp_48.49.31001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_1.46.11001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2.34.28001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_12.50.6001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_1.17.29001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_2019.430.2026.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-FodMetadata-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Migration-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Handwriting-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-OCR-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Speech-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-TextToSpeech-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MSPaint-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-PowerShell-ISE-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Printing-WFS-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-QuickAssist-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.19041.1
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-XPSServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WindowsMediaPlayer
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-InternetPrinting-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Deprecation
Dism /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase 
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\UUP1022.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
echo ===================================================
echo All Done, Boss~! Check C:\WINMOD\MOD For Your ISO!
echo ===================================================
pause
goto EXIT

:UUP1021
cls
echo SET IT AND FORGET IT, BOSS~!
echo =============================
move "C:\MODWIN\ISO\sources\install.wim" C:\MODWIN\WIM\mod.wim
dism.exe /mount-wim /wimfile:"C:\MODWIN\WIM\mod.wim" /mountdir:"C:\MODWIN\PATH" /index:1
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.549981C3F5F10_1.1911.21713.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.BingWeather_4.25.20211.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.DesktopAppInstaller_2019.125.2243.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.GetHelp_10.1706.13331.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.HEIFImageExtension_1.0.22742.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Microsoft3DViewer_6.1908.2042.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MSPaint_2019.729.2301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ScreenSketch_2019.904.1644.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.StorePurchaseApp_11811.1001.1813.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.VP9VideoExtensions_1.0.22681.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebMediaExtensions_1.0.20875.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WebpImageExtension_1.0.22753.0_x64__8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Windows.Photos_2019.19071.12548.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCalculator_2020.1906.55.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsStore_11910.1002.513.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Getstarted_8.2.22942.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftOfficeHub_18.1903.1152.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftSolitaireCollection_4.4.8204.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MicrosoftStickyNotes_3.6.73.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.MixedReality.Portal_2000.19081.1301.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Office.OneNote_16001.12026.20112.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.People_2019.305.632.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.SkypeApp_14.53.77.0_neutral_~_kzf8qxf38zg5c
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Wallet_2.4.18324.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsAlarms_2019.807.41.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsCamera_2018.826.98.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:microsoft.windowscommunicationsapps_16005.11629.20316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsFeedbackHub_2019.1111.2029.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsMaps_2019.716.2316.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.WindowsSoundRecorder_2019.716.2313.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.Xbox.TCUI_1.23.28002.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxApp_48.49.31001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGameOverlay_1.46.11001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxGamingOverlay_2.34.28001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxIdentityProvider_12.50.6001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.XboxSpeechToTextOverlay_1.17.29001.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.YourPhone_2019.430.2026.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneMusic_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-ProvisionedAppxPackage /PackageName:Microsoft.ZuneVideo_2019.19071.19011.0_neutral_~_8wekyb3d8bbwe
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-OneCore-DirectX-Database-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-FodMetadata-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Migration-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Hello-Face-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-InternetExplorer-Optional-Package~31bf3856ad364e35~amd64~~11.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Handwriting-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-OCR-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-Speech-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-LanguageFeatures-TextToSpeech-en-us-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MediaPlayer-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-MSPaint-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-PowerShell-ISE-FOD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-Printing-WFS-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-QuickAssist-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-StepsRecorder-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-TabletPCMath-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-UserExperience-Desktop-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:Microsoft-Windows-WordPad-FoD-Package~31bf3856ad364e35~amd64~~10.0.19041.1
dism.exe /Image:C:\MODWIN\PATH /Remove-Package /PackageName:OpenSSH-Client-Package~31bf3856ad364e35~amd64~~10.0.19041.1
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-XPSServices-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WCF-Services45
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MediaPlayback
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WindowsMediaPlayer
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SearchEngine-Client-Package
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MSRDC-Infrastructure
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:WorkFolders-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-Features
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-Foundation-InternetPrinting-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2Root
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:MicrosoftWindowsPowerShellV2
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:NetFx4-AdvSrvs
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Client
Dism /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:SMB1Protocol-Deprecation
Dism /Image:"C:\MODWIN\PATH" /cleanup-image /StartComponentCleanup /ResetBase 
dism /Unmount-Image /MountDir:"C:\MODWIN\PATH" /Commit
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\UUP1021.iso

