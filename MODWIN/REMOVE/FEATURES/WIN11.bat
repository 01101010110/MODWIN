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
dism.exe /Image:C:\MODWIN\PATH /Disable-Feature /FeatureName:Printing-PrintToPDFServices-Features /Remove && pause