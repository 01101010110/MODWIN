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
dism /export-image /SourceImageFile:"C:\MODWIN\WIM\mod.wim" /SourceIndex:1 /DestinationImageFile:"C:\MODWIN\ISO\sources\install.esd" /Compress:recovery /CheckIntegrity && pause
C:\MODWIN\oscdimg.exe -m -oc -u2 -udfver102 -bC:\MODWIN\ISO\efi\microsoft\boot\efisys_noprompt.bin C:\MODWIN\ISO C:\MODWIN\MOD\win11lite.iso
rmdir "C:\MODWIN\ISO" /S /Q
mkdir "C:\MODWIN\ISO"
del "C:\MODWIN\WIM\mod.wim"
del "C:\MODWIN\WIM\install.esd"