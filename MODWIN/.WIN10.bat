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
