#include "database.h"
#include "modwin.h"
#include <iostream>
#include <filesystem>
#include <algorithm>

DatabaseManager g_dbManager;

DatabaseManager::DatabaseManager() : db(nullptr),
isConnected(false) {
}

DatabaseManager::~DatabaseManager() {
	if (isConnected && db) {
		// Closes the database connection safely on shutdown
		sqlite3_close(db);
	}
}

bool DatabaseManager::Initialize(std::string basePath) {
	// Sets the database file path relative to the executable
	dbPath = basePath + "\\modwin_knowledge.db";

	// Tries to open the database file or creates it if missing
	int rc = sqlite3_open(dbPath.c_str(), &db);
	if (rc) {
		myLog.AddLog("[ERROR] Can't open database: %s\n", sqlite3_errmsg(db));
		return false;
	}

	isConnected = true;

	// Stores app descriptions and safety ratings
	const char* sqlDefs =
		"CREATE TABLE IF NOT EXISTS AppDefinitions ("
		"PackageName TEXT PRIMARY KEY, "
		"Description TEXT, "
		"SafetyRating INTEGER, "
		"Category TEXT);";

	// Tracks history of deleted items
	const char* sqlHist =
		"CREATE TABLE IF NOT EXISTS RemovalHistory ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"PackageName TEXT, "
		"RemovalDate DATETIME DEFAULT CURRENT_TIMESTAMP, "
		"Type TEXT);";

	char* errMsg = 0;
	// Creates the definitions table
	sqlite3_exec(db, sqlDefs, 0, 0, &errMsg);
	if (errMsg) {
		myLog.AddLog("[ERROR] DB Init Defs: %s\n", errMsg);
		sqlite3_free(errMsg);
	}

	// Creates the history table
	sqlite3_exec(db, sqlHist, 0, 0, &errMsg);
	if (errMsg) {
		myLog.AddLog("[ERROR] DB Init Hist: %s\n", errMsg);
		sqlite3_free(errMsg);
	}

	// Loads the master list of app descriptions
	CheckAndPopulateDefaults();

	return true;
}

void DatabaseManager::CheckAndPopulateDefaults() {
	char* errMsg = 0;

	// Wipes the existing definitions to prevent duplicates before reloading
	sqlite3_exec(db, "DELETE FROM AppDefinitions;", NULL, NULL, &errMsg);

	// Starts a transaction to make the bulk insert faster
	sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

	std::vector<std::string> defaults = {
		// Critical
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-LanguageFeatures-Basic', 'Core Windows language components. Breaking this breaks text rendering and UI display. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/add-language-packs-to-windows)', 3, 'Language');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-FodMetadata', 'Feature on Demand Metadata. Acts as an index for installed system features; removal breaks DISM. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-v2--capabilities)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Foundation-Package', 'The Windows Foundation Package. This contains the kernel and critical OS libraries. Do not remove. [Source: Microsoft](https://learn.microsoft.com/en-us/uwp/api/windows.foundation?view=winrt-26100)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Kernel-LA57-FoD-Package', 'Support for 57-bit linear addressing (5-level paging). Required for newer CPUs and memory management. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/drivers/kernel/overview-of-windows-components)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Client-LanguagePack-Package', 'The primary user interface language pack. Required for the OS to display text. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/add-language-packs-to-windows)', 3, 'Language');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Licenses-Core-Package', 'Core licensing management files. Required for Windows Activation. [Source: Microsoft](https://learn.microsoft.com/windows/deployment/volume-activation/plan-for-volume-activation-client)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Licenses-Professional-Package', 'Edition-specific licensing for Windows Professional. [Source: Microsoft](https://learn.microsoft.com/windows/deployment/volume-activation/plan-for-volume-activation-client)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('NetFx4-AdvSrvs', '.NET Framework 4.8 Advanced Services. The standard framework for modern Windows applications. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/install/on-windows-10)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Printing-Foundation-Features', 'Core printing infrastructure. Required for physical printing and Print-to-PDF. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/drivers/print/)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('NetFx3', '.NET Framework 3.5 (Legacy). Needed for older software compatibility. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/install/dotnet-35-windows)', 3, 'Compatibility');",

		// Important Features
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-NetFx3-OnDemand-Package', '.NET Framework 3.5. Required for many legacy games and applications (DirectX 9 era). [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/install/dotnet-35-windows)', 2, 'Compatibility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-VBSCRIPT-FoD-Package', 'VBScript Engine. Deprecated, but required by some legacy installers and system scripts. [Source: Microsoft](https://techcommunity.microsoft.com/t5/windows-it-pro-blog/vbscript-deprecation-timelines-and-next-steps/ba-p/4148301)', 2, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-OneCore-ApplicationModel-Sync-Desktop-FOD-Package', 'OneSync. Exchange ActiveSync and Internet Mail Sync Engine. Synchronizes mail, calendar, and people data across UWP apps. [Source: Microsoft](https://learn.microsoft.com/en-us/windows-hardware/manufacture/desktop/features-on-demand-non-language-fod#onesync)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-OneCore-Graphics-Tools-Package', 'DirectX Graphics Tools. Used for graphics debugging and some gaming features. [Source: Microsoft](https://learn.microsoft.com/windows/uwp/gaming/use-the-directx-runtime-and-visual-studio-graphics-diagnostic-features)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-OneCore-DirectX-Database-FOD-Package', 'DirectX Database. Contains compatibility configurations for various GPUs. [Source: Microsoft](https://microsoft.github.io/DirectX-Specs/)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Notepad-System-FoD-Package', 'System Notepad (notepad.exe). The legacy win32 version found in System32. [Source: Microsoft](https://apps.microsoft.com/detail/9msmlrh6lzf3)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-PowerShell-ISE-FOD-Package', 'PowerShell ISE. Integrated Scripting Environment for editing .ps1 files. [Source: Microsoft](https://learn.microsoft.com/powershell/scripting/windows-powershell/ise/introducing-the-windows-powershell-ise)', 2, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-LanguageFeatures-OCR', 'Optical Character Recognition. Used by Snipping Tool and Indexer to read text in images. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-language-fod)', 2, 'Input');",
		"INSERT INTO AppDefinitions VALUES ('VirtualMachinePlatform', 'Virtual Machine Platform. Required for WSL2 and Windows Subsystem for Android. [Source: Microsoft](https://learn.microsoft.com/windows/wsl/install-manual#step-3---enable-virtual-machine-feature)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Client-ProjFS', 'Windows Projected File System. Allows applications (like Git VFS) to project virtual filesystems. [Source: Microsoft](https://learn.microsoft.com/windows/win32/projfs/projected-file-system)', 2, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('MediaPlayback', 'Core Media Playback. for audio/video playback APIs. [Source: Microsoft](https://learn.microsoft.com/uwp/api/windows.media.playback)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Subsystem-Linux', 'Windows Subsystem for Linux (WSL). Allows running Linux binaries on Windows. [Source: Microsoft](https://learn.microsoft.com/windows/wsl/about)', 2, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('SearchEngine-Client-Package', 'Windows Search Indexer. Deleting this disables instant file search. [Source: Microsoft](https://learn.microsoft.com/en-us/windows/win32/search/-search-3x-wds-overview)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('MSRDC-Infrastructure', 'Remote Desktop Client Infrastructure. Required for connecting to remote PCs. [Source: Microsoft](https://learn.microsoft.com/windows-server/remote/remote-desktop-services/clients/remote-desktop-clients)', 2, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('Printing-PrintToPDFServices-Features', 'Microsoft Print to PDF. Virtual printer driver to save documents as PDF. [Source: Microsoft](https://learn.microsoft.com/en-us/answers/questions/5706312/how-to-print-to-pdf)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Printing-Foundation-InternetPrinting-Client', 'Internet Printing Client (IPP). Allows printing to web-connected printers. [Source: Microsoft](https://learn.microsoft.com/en-us/troubleshoot/windows-server/printing/manage-connect-printers-use-web-browser)', 2, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Hello-Face-Package', 'Windows Hello Face. Biometric authentication drivers. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/design/device-experiences/windows-hello-face-authentication)', 2, 'Biometrics');",
		"INSERT INTO AppDefinitions VALUES ('HypervisorPlatform', 'Windows Hypervisor Platform. Required for virtualization (VMware, Docker) and Core Isolation security. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/reference/hypervisor-platform)', 2, 'Security');",

		// Optional Features
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Wallpaper-Content-Extended-FoD-Package', 'Extended Wallpapers. High-resolution 4K background images. [Source: Microsoft](https://learn.microsoft.com/windows/configuration/windows-spotlight)', 1, 'Cosmetic');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-MediaPlayer-Package', 'Legacy Windows Media Player 12. Obsoleted by the new Media Player app. [Source: Microsoft](https://support.microsoft.com/windows/get-windows-media-player-81718e0d-cfce-25b1-aee3-94596b658287)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-StepsRecorder-Package', 'Steps Recorder (psr.exe). Tool for recording UI interactions. Deprecated. [Source: Microsoft](https://support.microsoft.com/windows/steps-recorder-deprecation-a64888d7-8482-4965-8ce3-25fb004e975f)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-TabletPCMath-Package', 'Math Input Panel. Recognizes handwritten math equations. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-non-language-fod)', 1, 'Input');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-WMIC-FoD-Package', 'WMIC. Legacy command-line management interface. Deprecated in favor of PowerShell. [Source: Microsoft](https://learn.microsoft.com/windows/win32/wmisdk/wmic)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-LanguageFeatures-Handwriting', 'Handwriting Recognition. Required for pen/stylus input. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-language-fod)', 1, 'Input');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-LanguageFeatures-Speech', 'Speech Recognition. Required for Voice Access and dictation. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-language-fod)', 1, 'Input');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-LanguageFeatures-TextToSpeech', 'Text-to-Speech (TTS). Voices used by Narrator and other accessibility tools. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-language-fod)', 1, 'Input');",
		"INSERT INTO AppDefinitions VALUES ('Printing-XPSServices-Features', 'XPS Viewer. Legacy document format similar to PDF. [Source: Microsoft](https://learn.microsoft.com/en-us/windows/win32/printdocs/documents)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('TelnetClient', 'Telnet Client. Insecure text-based remote protocol. Use SSH instead. [Source: Microsoft](https://learn.microsoft.com/windows-server/administration/windows-commands/telnet)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('TFTP', 'Trivial File Transfer Protocol. insecure UDP file transfer tool. [Source: Microsoft](https://learn.microsoft.com/windows-server/administration/windows-commands/tftp)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('TIFFIFilter', 'TIFF IFilter. Allows Windows Search to index text inside TIFF images. [Source: Microsoft](https://learn.microsoft.com/windows/win32/search/-search-ifilter-registering-filters)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('SimpleTCP', 'Simple TCP Services (Echo, Daytime, etc.). Obsolete networking tools. [Source: Microsoft](https://learn.microsoft.com/en-us/troubleshoot/windows-server/networking/description-tcp-features)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('WorkFolders-Client', 'Work Folders. Syncs files with corporate servers. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/work-folders/work-folders-overview)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WebServerRole', 'Internet Information Services (IIS). Host a full web server on your PC. [Source: Microsoft](https://learn.microsoft.com/iis/get-started/introduction-to-iis/iis-web-server-overview)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-Container', 'Microsoft Message Queuing. Legacy inter-process communication for enterprise apps. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('LegacyComponents', 'Legacy Components. Enables DirectPlay for pre-2005 games. [Source: Microsoft](https://learn.microsoft.com/en-us/windows/win32/cossdk/legacycomponents)', 1, 'Compatibility');",
		"INSERT INTO AppDefinitions VALUES ('DirectPlay', 'DirectPlay API. Deprecated networking library for old games. [Source: Microsoft](https://learn.microsoft.com/en-us/windows/compatibility/windows-components-installed-on-demand))', 1, 'Compatibility');",
		"INSERT INTO AppDefinitions VALUES ('Printing-Foundation-LPDPrintService', 'LPD Print Service. Allows Windows to act as a Unix print server. [Source: Microsoft](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/lpr)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('Printing-Foundation-LPRPortMonitor', 'LPR Port Monitor. Client for printing to Unix LPD servers. [Source: Microsoft](https://learn.microsoft.com/windows-server/administration/windows-commands/lpr)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('Recall', 'Windows Recall (Copilot+). AI feature that snapshots activity for semantic search. [Source: Microsoft](https://support.microsoft.com/windows/retrace-your-steps-with-recall-aa03f8a0-a78b-4b3e-b0a1-2eb8ac48701c)', 1, 'AI');",

		// IIS & Networking
		"INSERT INTO AppDefinitions VALUES ('WCF-HTTP-Activation', 'WCF HTTP Activation. .NET Framework component for web services. [Source: Microsoft](https://learn.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-configure-an-iis-hosted-wcf-service-with-ssl)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-NonHTTP-Activation', 'WCF Non-HTTP Activation. .NET component for TCP/Pipe activation. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/feature-details/configuring-the-net-tcp-port-sharing-service)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WebServer', 'IIS Web Server. Core web server binaries. [Source: Microsoft](https://learn.microsoft.com/iis/get-started/introduction-to-iis/iis-web-server-overview)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-CommonHttpFeatures', 'Common HTTP Features. Static content, default documents, directory browsing. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/configuration/system.webserver/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpErrors', 'HTTP Errors. Custom error pages for IIS. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httperrors/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpRedirect', 'HTTP Redirection. Module to redirect client requests. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httpredirect/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ApplicationDevelopment', 'App Development. ASP.NET, CGI, and ISAPI extensions. [Source: Microsoft](https://learn.microsoft.com/iis/application-frameworks/)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-Security', 'IIS Security. Request filtering, URL authorization, IP security. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/configuration/system.webserver/security/)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-RequestFiltering', 'Request Filtering. Blocks harmful HTTP requests. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/requestfiltering/)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-NetFxExtensibility', '.NET Extensibility. Allows IIS to host .NET logic. [Source: Microsoft](https://learn.microsoft.com/en-us/exchange/iis-7-net-extensibility-component-is-required-longhorniis7netext-exchange-2013-help)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-NetFxExtensibility45', '.NET 4.5 Extensibility. Updated version of the above. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/get-started/whats-new-in-iis-8/iis-80-using-aspnet-35-and-aspnet-45)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HealthAndDiagnostics', 'Health & Diagnostics. Logging and tracing tools. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httplogging)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpLogging', 'HTTP Logging. Logs web activity to disk. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httplogging)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-LoggingLibraries', 'Logging Libraries. Required for any IIS logging. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httplogging)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-RequestMonitor', 'Request Monitor. Real-time traffic monitoring. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/tracing/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpTracing', 'HTTP Tracing. Traces failed requests for debugging. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/tracing/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-URLAuthorization', 'URL Authorization. Controls user access to URLs. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/authorization/)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-IPSecurity', 'IP Security. Allow/Deny specific IP addresses. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/ipsecurity/)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-Performance', 'Performance Features. Compression and caching. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httpcompression/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpCompressionDynamic', 'Dynamic Compression. Compresses generated content. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httpcompression/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WebServerManagementTools', 'Management Tools. Console and scripting tools for IIS. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/manage/provisioning-and-managing-iis/iis-management-and-administration)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ManagementScriptingTools', 'Management Scripts. VBScript/WMI support for IIS. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/manage/scripting/managing-applications-and-application-pools-on-iis-with-wmi)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('IIS-IIS6ManagementCompatibility', 'IIS 6 Compatibility. For legacy scripts. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/install/installing-iis-7/understanding-setup-in-iis)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-Metabase', 'IIS 6 Metabase. Old configuration storage format. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/install/installing-iis-7/compatibility-and-feature-requirements-for-windows-vista)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('WAS-WindowsActivationService', 'Windows Process Activation Service (WAS). Manages worker processes. [Source: Microsoft](https://learn.microsoft.com/iis/manage/provisioning-and-managing-iis/features-of-the-windows-process-activation-service-was)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('WAS-ProcessModel', 'WAS Process Model. Core process management for WAS. [Source: Microsoft](https://learn.microsoft.com/iis/manage/provisioning-and-managing-iis/features-of-the-windows-process-activation-service-was)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('WAS-NetFxEnvironment', 'WAS .NET Environment. Support for managed code. [Source: Microsoft](https://learn.microsoft.com/iis/manage/provisioning-and-managing-iis/features-of-the-windows-process-activation-service-was)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WAS-ConfigurationAPI', 'WAS Config API. [Source: Microsoft](https://learn.microsoft.com/iis/manage/provisioning-and-managing-iis/features-of-the-windows-process-activation-service-was)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HostableWebCore', 'Hostable Web Core. Allows apps to host their own IIS instance. [Source: Microsoft](https://learn.microsoft.com/iis/get-started/introduction-to-iis/iis-web-server-overview)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-Services45', 'WCF Services 4.5. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/whats-new)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-HTTP-Activation45', 'WCF HTTP Activation 4.5. [Source: Microsoft](https://learn.microsoft.com/en-us/dotnet/framework/wcf/feature-details/how-to-install-and-configure-wcf-activation-components)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-TCP-Activation45', 'WCF TCP Activation 4.5. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/feature-details/configuring-the-net-tcp-port-sharing-service)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-Pipe-Activation45', 'WCF Pipe Activation 4.5. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/feature-details/configuring-the-net-tcp-port-sharing-service)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-MSMQ-Activation45', 'WCF MSMQ Activation 4.5. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/feature-details/configuring-the-net-tcp-port-sharing-service)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('WCF-TCP-PortSharing45', 'WCF TCP Port Sharing. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/wcf/feature-details/net-tcp-port-sharing)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-StaticContent', 'Static Content. Serves HTML, images, and CSS. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/staticcontent/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-DefaultDocument', 'Default Document. Handles index.html. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/defaultdocument/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-DirectoryBrowsing', 'Directory Browsing. Lists files in browser (insecure). [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/directorybrowse/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WebDAV', 'WebDAV. Remote file editing over HTTP. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/webdav/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WebSockets', 'WebSocket Protocol. Real-time communication support. [Source: Microsoft](https://learn.microsoft.com/iis/get-started/whats-new-in-iis-8/iis-80-websocket-protocol-support)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ApplicationInit', 'Application Initialization. Pre-loads web apps. [Source: Microsoft](https://learn.microsoft.com/iis/get-started/whats-new-in-iis-8/iis-80-application-initialization)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ISAPIFilter', 'ISAPI Filters. Legacy plugin support. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/isapifilters/)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ISAPIExtensions', 'ISAPI Extensions. Legacy app support. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/iis/6.0-sdk/ms525282%28v=vs.90%29)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ASPNET', 'ASP.NET 3.5. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/get-started/whats-new-in-iis-8/iis-80-using-aspnet-35-and-aspnet-45))', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ASPNET45', 'ASP.NET 4.5. [Source: Microsoft](https://learn.microsoft.com/en-us/aspnet/web-forms/overview/getting-started/getting-started-with-aspnet-45-web-forms/introduction-and-overview)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ASP', 'Classic ASP. Ancient scripting tech. [Source: Microsoft](https://learn.microsoft.com/iis/application-frameworks/running-classic-asp-applications-on-iis-7-and-iis-8/classic-asp-not-installed-by-default-on-iis)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-CGI', 'CGI. Common Gateway Interface. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/cgi)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ServerSideIncludes', 'Server Side Includes (SSI). Legacy dynamic HTML. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/serversideinclude)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-CustomLogging', 'Custom Logging. [Source: Microsoft](https://learn.microsoft.com/iis/extensions/advanced-logging-module/advanced-logging-readme)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-BasicAuthentication', 'Basic Authentication. Transmits passwords in plain text. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/authentication/basicauthentication)', 1, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-HttpCompressionStatic', 'Static Compression. Compresses files. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/httpcompression/)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ManagementConsole', 'IIS Management Console. GUI tool. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/manage/provisioning-and-managing-iis/iis-management-and-administration)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ManagementService', 'IIS Management Service. Remote management. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/manage/remote-administration/remote-administration-for-iis-manager)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WMICompatibility', 'IIS WMI Compatibility. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/manage/scripting/managing-sites-with-the-iis-wmi-provider)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('IIS-LegacyScripts', 'IIS Legacy Scripts. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/iis/6.0-sdk/ms524713%28v=vs.90%29)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('IIS-FTPServer', 'FTP Server. File Transfer Protocol. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/configuration/system.ftpserver/)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('IIS-FTPSvc', 'FTP Service. [Source: Microsoft](https://learn.microsoft.com/en-us/troubleshoot/windows-server/high-availability/configure-internet-information-services-ftp)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('IIS-FTPExtensibility', 'FTP Extensibility. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/publish/using-the-ftp-service/configure-ftp-with-iis-manager-authentication-in-iis-7)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-DCOMProxy', 'MSMQ DCOM Proxy. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-Server', 'MSMQ Server. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-HTTP', 'MSMQ HTTP Support. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-Multicast', 'MSMQ Multicast. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-Triggers', 'MSMQ Triggers. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('SMB1Protocol-Deprecation', 'SMB1 Deprecation. Metadata about SMB1 removal. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/file-server/troubleshoot/detect-enable-and-disable-smbv1-v2-v3)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('WindowsMediaPlayer', 'Windows Media Player Executable. [Source: Microsoft](https://support.microsoft.com/windows/get-windows-media-player-81718e0d-cfce-25b1-aee3-94596b658287)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('NetFx4Extended-ASPNET45', 'ASP.NET 4.5. [Source: Microsoft](https://learn.microsoft.com/en-us/dotnet/framework/install/guide-for-developers)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Windows-Defender-Default-Definitions', 'Windows Defender Definitions. Core antivirus database. [Source: Microsoft](https://learn.microsoft.com/microsoft-365/security/defender-endpoint/microsoft-defender-antivirus-windows)', 3, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('Windows-Identity-Foundation', 'Windows Identity Foundation. Framework for identity-aware apps. [Source: Microsoft](https://www.microsoft.com/en-us/download/details.aspx?id=17331)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('SMB1Protocol', 'SMB 1.0/CIFS Support. Highly insecure legacy protocol. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/file-server/troubleshoot/detect-enable-and-disable-smbv1-v2-v3)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('SMB1Protocol-Client', 'SMB 1.0 Client. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/file-server/troubleshoot/detect-enable-and-disable-smbv1-v2-v3)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('SMB1Protocol-Server', 'SMB 1.0 Server. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/file-server/troubleshoot/detect-enable-and-disable-smbv1-v2-v3)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-RemoteDesktopConnection', 'Remote Desktop Connection (mstsc). [Source: Microsoft](https://learn.microsoft.com/windows-server/remote/remote-desktop-services/clients/remote-desktop-allow-access)', 2, 'Networking');",

		// Hardware Drivers & Control Panels
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Ethernet-Client-Intel', 'Intel Ethernet Drivers. Required for Intel LAN. [Source: Intel](https://www.intel.com/content/www/us/en/products/details/ethernet.html)', 3, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Ethernet-Client-Realtek', 'Realtek Ethernet Drivers. Required for Realtek LAN. [Source: Microsoft Update Catalog](https://www.catalog.update.microsoft.com/Search.aspx?q=Realtek%20PCIe%20GbE%20Family%20Controller)', 3, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Ethernet-Client-Vmware', 'VMware Ethernet Drivers. Required for networking in VMs. [Source: VMware](https://docs.vmware.com/en/VMware-Tools/11.3.0/com.vmware.vsphere.vmwaretools.doc/GUID-8822003C-F51D-40C6-92F6-963C8157A42A.html)', 3, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Wifi-Client', 'Wi-Fi Core. Needed for wireless networking. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/drivers/network/wifi-universal-driver-model)', 3, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('9426MICRO-STARINTERNATION.MSICenter', 'MSI Center. Controls your laptop fans, RGB, and performance profiles. Bloated, but necessary for MSI hardware. [Source: MSI](https://www.msi.com/Landing/MSI-Center)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('AdvancedMicroDevicesInc-RSXCM', 'AMD Radeon Software. The control panel for your AMD graphics card. [Source: AMD](https://www.amd.com/en/technologies/software)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('AppUp.IntelArcSoftware', 'Intel Arc Control. You need this to control your Intel GPU. [Source: Intel](https://www.intel.com/content/www/us/en/products/docs/discrete-gpus/arc/software/arc-control.html)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('AppUp.IntelGraphicsExperience', 'Intel Graphics Command Center. Lets you change resolution and color settings for Intel integrated graphics. [Source: Intel](https://www.intel.com/content/www/us/en/support/articles/000055840/graphics.html)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('NVIDIACorp.NVIDIAControlPanel', 'NVIDIA Control Panel. For changing settings on your GeForce card. [Source: NVIDIA](https://www.nvidia.com/en-us/geforce/drivers/)', 2, 'Drivers');",
		"INSERT INTO AppDefinitions VALUES ('RealtekSemiconductorCorp.RealtekAudioControl', 'Realtek Audio Console. The app that pops up when you plug in headphones. You need it to switch audio ports. [Source: Realtek](https://www.microsoft.com/store/productId/9P2B8MCSVPLN)', 2, 'Drivers');",

		// System Packages
		"INSERT INTO AppDefinitions VALUES ('Package_for_ServicingStack', 'Servicing Stack. Critical component that handles the installation of Windows updates. [Source: Microsoft](https://learn.microsoft.com/windows/deployment/update/servicing-stack-updates)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Package_for_RollupFix', 'Cumulative Update. Monthly security and quality patches. [Source: Microsoft](https://learn.microsoft.com/windows/deployment/update/quality-updates)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Package_for_KB', 'Specific Knowledge Base (KB) update. [Source: Microsoft](https://www.catalog.update.microsoft.com/Faq.aspx)', 3, 'System Core');",
		"INSERT INTO AppDefinitions VALUES ('Package_for_DotNetRollup', '.NET Framework Update. Patches for the .NET runtime. [Source: Microsoft](https://learn.microsoft.com/dotnet/framework/install/)', 3, 'System Core');",

		// Store Apps
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsStore', 'Microsoft Store. Required for app installs and updates. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrfjbmp)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.SecHealthUI', 'Windows Security. Dashboard for Defender Antivirus. [Source: Microsoft](https://support.microsoft.com/windows/device-security-in-windows-security-afa11526-de57-b1c5-599f-3a4c6a61c5e2)', 3, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.DesktopAppInstaller', 'App Installer. Enables winget (Windows built-in command-line package manager) and side loading .appx files. [Source: Microsoft](https://apps.microsoft.com/store/detail/app-installer/9NBLGGH4NNS1)', 3, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.VCLibs', 'Visual C++ Runtime Libraries (UWP). Dependency for most Store apps. [Source: Microsoft](https://learn.microsoft.com/cpp/windows/latest-supported-vc-redist)', 3, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.StorePurchaseApp', 'Store Purchase App. Handles licensing and payment flows. [Source: Microsoft](https://learn.microsoft.com/windows/uwp/monetize/in-app-purchases-and-trials)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.ApplicationCompatibilityEnhancements', 'OpenCL/OpenGL Compatibility Pack. Compatibility layer for legacy graphics apps. [Source: Microsoft](https://apps.microsoft.com/detail/9nqpsl29bfff)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftWindows.Client.WebExperience', 'Windows Web Experience Pack. Powers Widgets and Taskbar feeds. [Source: Microsoft](https://apps.microsoft.com/detail/9mssgkg348sp)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftWindows.CrossDevice', 'Cross Device Experience. Backend for Phone Link and shared clipboard. [Source: Microsoft](https://learn.microsoft.com/windows/uwp/launch-resume/connected-apps-and-devices)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.M365Companions', 'Microsoft 365 Companions. Integrations for Office and mobile. [Source: Microsoft](https://www.microsoft.com/microsoft-365)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('aimgr', 'AI Manager. Component for NPU/AI hardware acceleration. [Source: Microsoft](https://learn.microsoft.com/windows/ai/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.549981C3F5F10', 'Cortana. Legacy voice assistant. [Source: Microsoft](https://support.microsoft.com/topic/end-of-support-for-cortana-in-windows-d025b39f-ee5b-4836-a954-0ab646ee1efa)', 1, 'AI');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Copilot', 'Microsoft Copilot. AI assistant integration. [Source: Microsoft](https://copilot.microsoft.com/)', 1, 'AI');",

		// Store Apps - Media Codecs
		"INSERT INTO AppDefinitions VALUES ('Microsoft.AV1VideoExtension', 'AV1 Video Extension. High efficiency video codec support. [Source: Microsoft](https://apps.microsoft.com/store/detail/av1-video-extension/9MVZQVXJBQ9V)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.AVCEncoderVideoExtension', 'AVC Encoder. Hardware acceleration for H.264 video encoding. [Source: Microsoft](https://apps.microsoft.com/detail/9pb0trcnrhfx)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.HEIFImageExtension', 'HEIF Image Extensions. Support for Apple .HEIC photos. [Source: Microsoft](https://apps.microsoft.com/store/detail/heif-image-extensions/9PMMSR1CGPWG)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.HEVCVideoExtension', 'HEVC Video Extensions. Support for H.265 (4K/High-Res) video. [Source: Microsoft](https://apps.microsoft.com/store/detail/hevc-video-extensions/9NMZLZ57R3T7)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MPEG2VideoExtension', 'MPEG-2 Video Extension. Support for DVD and legacy video formats. [Source: Microsoft](https://apps.microsoft.com/detail/9n95q1zzpmh4)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.RawImageExtension', 'Raw Image Extension. Support for professional camera RAW formats. [Source: Microsoft](https://apps.microsoft.com/store/detail/raw-image-extension/9NCTDW2W1BH8)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.VP9VideoExtensions', 'VP9 Video Extensions. Codec required for many 4K YouTube videos. [Source: Microsoft](https://apps.microsoft.com/detail/9n4d0msmp0pt)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WebMediaExtensions', 'Web Media Extensions. Support for OGG, Vorbis, and Theora. [Source: Microsoft](https://apps.microsoft.com/store/detail/web-media-extensions/9N5TDP8VCMHS)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WebpImageExtension', 'WebP Image Extensions. Support for Google WebP images. [Source: Microsoft](https://apps.microsoft.com/detail/9pg2dk419drg?hl=en-US&gl=US)', 2, 'Media');",

		// Store Apps - Utilities
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsCalculator', 'Windows Calculator. [Source: Microsoft](https://apps.microsoft.com/store/detail/windows-calculator/9WZDNCRFHVN5)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Windows.Photos', 'Microsoft Photos. Default image viewer and editor. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-photos/9WZDNCRFJBH4)', 2, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsNotepad', 'Windows Notepad (Store App). Modern text editor with tabs. [Source: Microsoft](https://apps.microsoft.com/detail/9msmlrh6lzf3)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsTerminal', 'Windows Terminal. Multi-tabbed command line interface. [Source: Microsoft](https://apps.microsoft.com/detail/9n0dx20hk701)', 2, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.ScreenSketch', 'Snipping Tool. Screen capture and annotation utility. [Source: Microsoft](https://apps.microsoft.com/store/detail/snipping-tool/9MZ95KL8MR0L)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MSPaint', 'Microsoft Paint. The classic art creation tool, updated for modern Windows. [Source: Microsoft](https://www.microsoft.com/en-us/windows/paint)', 2, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsMaps', 'Windows Maps. Offline maps and navigation. [Source: Microsoft](https://support.microsoft.com/en-us/windows/find-your-way-with-maps-51ece9fb-a0f2-9853-4164-6940865085c8)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsSoundRecorder', 'Sound Recorder. Simple voice recording app. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrfhwkn)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsAlarms', 'Windows Clock. Alarms, timers, and stopwatches. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrfj3pr)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MicrosoftStickyNotes', 'Microsoft Sticky Notes. Desktop notes synced to OneNote. [Source: Microsoft](https://support.microsoft.com/topic/get-started-with-sticky-notes-86b36182-fdf5-4f9b-af7a-2846f83263f5)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Todos', 'Microsoft To Do. Task management app. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-to-do/9NBLGGH5R558)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.PowerAutomateDesktop', 'Power Automate. Workflow automation tool. [Source: Microsoft](https://powerautomate.microsoft.com/desktop/)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftCorporationII.QuickAssist', 'Quick Assist. Remote support tool for helping others. [Source: Microsoft](https://apps.microsoft.com/store/detail/quick-assist/9P7BP5VNWKX5)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsFeedbackHub', 'Feedback Hub. Submit bugs and suggestions to Microsoft. [Source: Microsoft](https://apps.microsoft.com/store/detail/feedback-hub/9NBLGGH4R32N)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.GetHelp', 'Get Help. Support portal and chatbot. [Source: Microsoft](https://support.microsoft.com/windows/about-get-help-717c7ae9-5c45-45f0-b637-900b7437a395)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.YourPhone', 'Phone Link. Connect Android/iPhone to PC. [Source: Microsoft](https://apps.microsoft.com/store/detail/phone-link/9NMPJ99VJBWV)', 1, 'Utility');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftCorporationII.WindowsSubsystemForLinux', 'Windows Subsystem for Linux (Store App). [Source: Microsoft](https://learn.microsoft.com/en-us/windows/wsl/install)', 2, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Office.ActionsServer', 'Office Actions. Background service for Office integration. [Source: Microsoft](https://www.microsoft.com/microsoft-365)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.OfficePushNotificationUtility', 'Office Push Notifications. [Source: Microsoft](https://www.microsoft.com/microsoft-365)', 2, 'System');",

		// Store Apps - Gaming
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MicrosoftSolitaireCollection', 'Solitaire Collection. Classic card games. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-solitaire-collection/9WZDNCRFHWD2)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.XboxApp', 'Xbox App. Game Pass launcher and social hub. [Source: Microsoft](https://apps.microsoft.com/detail/9mv0b5hzvk9z)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.GamingApp', 'Gaming Services. Background services for Xbox games and Game Pass. [Source: Microsoft](https://apps.microsoft.com/detail/9mwpm2cqnlhn?hl=en-US&gl=US)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Xbox.TCUI', 'Xbox TCUI. User Interface for party chat and invites. [Source: Microsoft](https://support.xbox.com/)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.XboxGameOverlay', 'Xbox Game Overlay. Part of the Game Bar overlay (Win+G). [Source: Microsoft](https://support.xbox.com/en-US/help/games-apps/game-setup-and-play/get-to-know-game-bar-on-windows-10)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.XboxGamingOverlay', 'Xbox Game Bar. Overlay for recording and audio control (Win+G). [Source: Microsoft](https://apps.microsoft.com/store/detail/xbox-game-bar/9NZKPSTSNW4P)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.XboxIdentityProvider', 'Xbox Identity Provider. Authentication for Xbox Live games. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrd1hkw?hl=en-US&gl=US)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.XboxSpeechToTextOverlay', 'Xbox Speech-to-Text. Accessibility overlay for gaming. [Source: Microsoft](https://www.xbox.com/community/for-everyone/accessibility)', 1, 'Gaming');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Edge.GameAssist', 'Edge Game Assist. Browser sidebar for gaming tips. [Source: Microsoft](https://www.microsoft.com/edge)', 1, 'Gaming');",

		// Store Apps - Web & Legacy
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-InternetExplorer-Optional-Package', 'Internet Explorer 11. Legacy browser for compatibility. [Source: Microsoft](https://learn.microsoft.com/lifecycle/products/internet-explorer-11)', 1, 'Legacy');",
		"INSERT INTO AppDefinitions VALUES ('OpenSSH-Client-Package', 'OpenSSH Client. Secure shell tools for remote command line. [Source: Microsoft](https://learn.microsoft.com/windows-server/administration/openssh/openssh_overview)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Lxss-Optional-Package', 'WSL Kernel. Core component for Linux subsystem. [Source: Microsoft](https://learn.microsoft.com/windows/wsl/install)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MicrosoftEdge.Stable', 'Microsoft Edge. Default Chromium-based web browser. [Source: Microsoft](https://www.microsoft.com/edge)', 2, 'System');",

		// Store Apps - Bloatware/Optional
		"INSERT INTO AppDefinitions VALUES ('Clipchamp.Clipchamp', 'Microsoft Clipchamp. Web-based video editor. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-clipchamp/9P1J8S7CCWWT)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.BingWeather', 'MSN Weather. Weather forecasts. [Source: Microsoft](https://apps.microsoft.com/store/detail/msn-weather/9WZDNCRFJ3Q2)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.BingNews', 'Microsoft News. News aggregator. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-news/9WZDNCRFHVFW)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.BingSearch', 'Bing Web Search. Dashboard component that opens Bing in the browser. [Source: Microsoft](https://www.bing.com)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Getstarted', 'Tips. Introductory tutorials for Windows. [Source: Microsoft](https://www.microsoft.com/en-us/windows/tips)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Microsoft3DViewer', '3D Viewer. Viewer for 3D models. [Source: Microsoft](https://apps.microsoft.com/store/detail/3d-viewer/9NBLGGH42THS)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MicrosoftOfficeHub', 'Microsoft 365 (Office). Launcher for Office apps. [Source: Microsoft](https://apps.microsoft.com/store/detail/microsoft-365-office/9WZDNCRD29V9)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.MixedReality.Portal', 'Mixed Reality Portal. VR headset setup tool. [Source: Microsoft](https://apps.microsoft.com/store/detail/mixed-reality-portal/9NG1H8B3ZC7M)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Office.OneNote', 'OneNote for Windows 10. Digital notebook. [Source: Microsoft](https://support.microsoft.com/en-us/office/moving-to-onenote-on-windows-4ba7b498-aafc-44b1-8326-a582a6c71196)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.OutlookForWindows', 'Outlook (New). Web-based email client replacing Mail app. [Source: Microsoft](https://apps.microsoft.com/store/detail/outlook-for-windows/9NRX63209R7B)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Paint', 'Paint 3D (Deprecated). 3D modeling tool. Replaced by the new Paint app. [Source: Microsoft](https://learn.microsoft.com/en-us/windows/whats-new/deprecated-features)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.People', 'Microsoft People. Contact management. [Source: Microsoft](https://support.microsoft.com/en-us/windows/people-app-help-144c4373-fa32-da81-f8b4-ee87589c509c)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.SkypeApp', 'Skype. Video calling and messaging. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrfj364?hl=en-US&gl=US)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Wallet', 'Microsoft Wallet. Payment and pass management. [Source: Microsoft](https://support.microsoft.com/en-us/microsoft-edge/wallet-is-retired-in-microsoft-edge-bc52418a-4d35-444e-845c-e09df6033b81)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Windows.DevHome', 'Dev Home. Dashboard for developer workflows. [Source: Microsoft](https://apps.microsoft.com/store/detail/dev-home/9N8MHTPHNGVV)', 1, 'Developer');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsCamera', 'Windows Camera. Webcam application. [Source: Microsoft](https://apps.microsoft.com/store/detail/windows-camera/9WZDNCRFJBBG)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('microsoft.windowscommunicationsapps', 'Mail and Calendar. Legacy email apps. [Source: Microsoft](https://apps.microsoft.com/detail/9nrx63209r7b?hl=en-US&gl=US)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.ZuneMusic', 'Windows Media Player (Modern). Music library and player. [Source: Microsoft](https://apps.microsoft.com/detail/9wzdncrfj3pt?hl=en-US&gl=US)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.ZuneVideo', 'Movies & TV. Video player. [Source: Microsoft](https://apps.microsoft.com/store/detail/movies-tv/9WZDNCRFJ3P2)', 1, 'Media');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftCorporationII.MicrosoftFamily', 'Microsoft Family Safety. Parental controls. [Source: Microsoft](https://family.microsoft.com/)', 1, 'Bloatware');",
		"INSERT INTO AppDefinitions VALUES ('MicrosoftTeams', 'Microsoft Teams (Work/School). Enterprise communication. [Source: Microsoft](https://www.microsoft.com/microsoft-teams/download-app)', 1, 'Social');",
		"INSERT INTO AppDefinitions VALUES ('MSTeams', 'Microsoft Teams (Personal). Consumer version of Teams. [Source: Microsoft](https://www.microsoft.com/microsoft-teams/download-app)', 1, 'Social');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.NET.Native.Framework.2.2', '.NET Native Framework 2.2. Runtime support for UWP apps. [Source: Microsoft](https://devblogs.microsoft.com/dotnet/the-net-native-tool-chain/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.NET.Native.Runtime.2.2', '.NET Native Runtime 2.2. Core runtime for UWP apps. [Source: Microsoft](https://devblogs.microsoft.com/dotnet/the-net-native-tool-chain/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.Services.Store.Engagement', 'Microsoft Store Engagement. Background service for Store notifications and updates. [Source: Microsoft](https://learn.microsoft.com/uwp/api/microsoft.services.store.engagement)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.UI.Xaml.2.7', 'WinUI 2.7. XAML UI controls library. [Source: Microsoft](https://learn.microsoft.com/windows/apps/winui/winui2/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.UI.Xaml.2.8', 'WinUI 2.8. XAML UI controls library. [Source: Microsoft](https://learn.microsoft.com/windows/apps/winui/winui2/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsAppRuntime.1.3', 'Windows App SDK 1.3. Runtime for modern Windows apps. [Source: Microsoft](https://learn.microsoft.com/windows/apps/windows-app-sdk/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft.WindowsAppRuntime.1.4', 'Windows App SDK 1.4. Runtime for modern Windows apps. [Source: Microsoft](https://learn.microsoft.com/windows/apps/windows-app-sdk/)', 2, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-Printing-PMCPPC-FoD-Package', 'Print Management Console. Advanced print management tools. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/manufacture/desktop/features-on-demand-non-language-fod)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Windows-SenseClient-FoD-Package', 'Windows Defender Advanced Threat Protection (Sense). Enterprise security feature. [Source: Microsoft](https://learn.microsoft.com/microsoft-365/security/defender-endpoint/microsoft-defender-endpoint)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V', 'Hyper-V Hypervisor. Runs virtual machines. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-All', 'Hyper-V Platform. Complete virtualization platform. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-Hypervisor', 'Hyper-V Hypervisor Core. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/reference/hypervisor-platform)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-Management-Clients', 'Hyper-V Management Tools (GUI). Hyper-V Manager. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-Management-PowerShell', 'Hyper-V PowerShell Module. Scripting for Hyper-V. [Source: Microsoft](https://learn.microsoft.com/powershell/module/hyper-v/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-Services', 'Hyper-V Services. Background services for VMs. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Microsoft-Hyper-V-Tools-All', 'Hyper-V Management Tools. GUI and PowerShell tools. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('MSMQ-ADIntegration', 'MSMQ Active Directory Integration. [Source: Microsoft](https://learn.microsoft.com/openspecs/windows_protocols/mc-mqsrm/f5d86382-fba8-4e1c-848f-a74344f230fb)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('IIS-CertProvider', 'IIS Centralized Certificate Provider. [Source: Microsoft](https://learn.microsoft.com/iis-administration/api/centralized-certificates)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('IIS-WindowsAuthentication', 'IIS Windows Authentication. Authenticate using Windows accounts. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/authentication/windowsauthentication/)', 1, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-DigestAuthentication', 'IIS Digest Authentication. Challenge-response authentication. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/authentication/digestauthentication)', 1, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ClientCertificateMappingAuthentication', 'IIS Client Certificate Mapping. Map certs to user accounts. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/security/authentication/clientcertificatemappingauthentication)', 1, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-IISCertificateMappingAuthentication', 'IIS Certificate Mapping. Map certs to Windows accounts. [Source: Microsoft](https://learn.microsoft.com/en-us/iis/configuration/system.webserver/security/authentication/iisclientcertificatemappingauthentication/)', 1, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('IIS-ODBCLogging', 'IIS ODBC Logging. Log to a database via ODBC. [Source: Microsoft](https://learn.microsoft.com/iis/configuration/system.webserver/odbclogging)', 1, 'Server');",
		"INSERT INTO AppDefinitions VALUES ('DirectoryServices-ADAM-Client', 'AD LDS (ADAM) Client. Lightweight Directory Services tools. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/windows/desktop/adam/active-directory-lightweight-directory-services))', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('SmbDirect', 'SMB Direct. High performance file sharing over RDMA. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/file-server/smb-direct)', 2, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('AppServerClient', 'Application Server Client. Support for specialized app servers. [Source: Microsoft](https://learn.microsoft.com/en-us/microsoft-desktop-optimization-pack/#application-virtualization-app-v))', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('HostGuardian', 'Host Guardian Service. Shielded VMs protection. [Source: Microsoft](https://learn.microsoft.com/windows-server/security/guarded-fabric-shielded-vm/guarded-fabric-and-shielded-vms)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('Client-DeviceLockdown', 'Device Lockdown. Features for kiosk mode and restricted access. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/customize/enterprise/assigned-access)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Client-EmbeddedShellLauncher', 'Shell Launcher. Replace Explorer shell with custom app. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/customize/enterprise/shell-launcher)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Client-EmbeddedBootExp', 'Unbranded Boot. Remove Windows boot logos. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/customize/enterprise/unbranded-boot)', 1, 'Cosmetic');",
		"INSERT INTO AppDefinitions VALUES ('Client-EmbeddedLogon', 'Custom Logon. Suppress Windows UI elements during logon. [Source: Microsoft](https://learn.microsoft.com/windows-hardware/customize/enterprise/custom-logon)', 1, 'Cosmetic');",
		"INSERT INTO AppDefinitions VALUES ('Client-KeyboardFilter', 'Keyboard Filter. Block specific key combinations (e.g. Ctrl+Alt+Del). [Source: Microsoft](https://learn.microsoft.com/windows/iot/iot-enterprise/customize/keyboard-filter)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Client-UnifiedWriteFilter', 'Unified Write Filter (UWF). Protects drive from writes (Kiosk mode). [Source: Microsoft](https://learn.microsoft.com/windows-hardware/customize/enterprise/unified-write-filter)', 1, 'System');",
		"INSERT INTO AppDefinitions VALUES ('Containers-DisposableClientVM', 'Windows Sandbox. Run a temporary desktop in a VM. [Source: Microsoft](https://learn.microsoft.com/windows/security/application-security/application-isolation/windows-sandbox/windows-sandbox-overview)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Containers-Server-For-Application-Guard', 'Application Guard. Isolate Edge browser in a container. [Source: Microsoft](https://learn.microsoft.com/windows/security/application-security/application-isolation/microsoft-defender-application-guard/md-app-guard-overview)', 2, 'Security');",
		"INSERT INTO AppDefinitions VALUES ('HyperV-KernelInt-VirtualDevice', 'Hyper-V Kernel Interface. Internal virtualization component. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('HyperV-Guest-KernelInt', 'Hyper-V Guest Kernel Interface. Internal virtualization component. [Source: Microsoft](https://learn.microsoft.com/virtualization/hyper-v-on-windows/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('DataCenterBridging', 'Data Center Bridging. Converged network traffic management. [Source: Microsoft](https://learn.microsoft.com/en-us/windows-server/networking/technologies/hpn/hpn-top)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('ServicesForNFS-ClientOnly', 'Services for NFS (Client). Access NFS file shares. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/nfs/nfs-overview)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('ClientForNFS-Infrastructure', 'Client for NFS Infrastructure. Core NFS support. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/nfs/nfs-overview)', 1, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('NFS-Administration', 'NFS Administration Tools. [Source: Microsoft](https://learn.microsoft.com/windows-server/storage/nfs/nfs-overview)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('Containers', 'Containers. Support for Docker and Windows Containers. [Source: Microsoft](https://learn.microsoft.com/virtualization/windowscontainers/about/)', 2, 'Virtualization');",
		"INSERT INTO AppDefinitions VALUES ('Containers-HNS', 'Host Network Service. Networking for containers. [Source: Microsoft](https://learn.microsoft.com/virtualization/windowscontainers/container-networking/architecture)', 2, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('Containers-SDN', 'Software Defined Networking. Advanced container networking. [Source: Microsoft](https://learn.microsoft.com/virtualization/windowscontainers/container-networking/architecture)', 2, 'Networking');",
		"INSERT INTO AppDefinitions VALUES ('MultiPoint-Connector', 'MultiPoint Connector. Monitor and manage MultiPoint stations. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/windows-server/it-pro/windows-server-2019/remote/multipoint-services/multipoint-services-stations)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('MultiPoint-Connector-Services', 'MultiPoint Connector Services. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/windows-server/it-pro/windows-server-2019/remote/multipoint-services/introducing-multipoint-services)', 1, 'Management');",
		"INSERT INTO AppDefinitions VALUES ('MultiPoint-Tools', 'MultiPoint Tools. Dashboard and manager. [Source: Microsoft](https://learn.microsoft.com/en-us/previous-versions/windows-server/it-pro/windows-server-2019/remote/multipoint-services/managing-your-multipoint-services-system)', 1, 'Management');"
	};

	// Loops through the defaults list and runs each insert command
	for (const auto& sql : defaults) {
		char* execErrMsg = 0;
		int rc = sqlite3_exec(db, sql.c_str(), NULL, NULL, &execErrMsg);
		if (rc != SQLITE_OK) {
			sqlite3_free(execErrMsg);
		}
	}

	// Saves all the changes to the disk
	sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
}

AppKnowledge DatabaseManager::GetAppInfo(std::string packageName) {
	// Sets default values in case nothing is found
	AppKnowledge info = { "No description available.", 0, "Unknown" };

	if (!isConnected) return info;

	// Creates a clean version of the name by removing version numbers
	std::string sysBase = packageName;
	size_t tilde = sysBase.find('~');
	if (tilde != std::string::npos) sysBase = sysBase.substr(0, tilde);

	// Creates another clean version by removing Appx underscores
	std::string appxBase = packageName;
	size_t underscore = appxBase.find('_');
	if (underscore != std::string::npos) appxBase = appxBase.substr(0, underscore);

	// Selects the first matching record from the database
	const char* sql = "SELECT Description, SafetyRating, Category FROM AppDefinitions "
		"WHERE PackageName = ? "
		"OR PackageName = ? "
		"OR PackageName = ? "
		"OR INSTR(?, PackageName) > 0 LIMIT 1;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
		// Fills in the question marks with the actual names
		sqlite3_bind_text(stmt, 1, packageName.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, sysBase.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 3, appxBase.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 4, packageName.c_str(), -1, SQLITE_STATIC);

		// Steps through the result if a row was returned
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			// Extracts data from the columns
			const unsigned char* desc = sqlite3_column_text(stmt, 0);
			if (desc) info.Description = reinterpret_cast<const char*>(desc);
			info.SafetyRating = sqlite3_column_int(stmt, 1);
			const unsigned char* cat = sqlite3_column_text(stmt, 2);
			if (cat) info.Category = reinterpret_cast<const char*>(cat);
		}
		// Cleans up the memory used by the query
		sqlite3_finalize(stmt);
	}

	// KB Search Links
	if (info.Description.find("catalog.update.microsoft.com") != std::string::npos) {
		// Try to extract KB number from the actual package name 
		std::string kbStr;
		size_t kbPos = packageName.find("KB");
		if (kbPos != std::string::npos) {
			size_t endPos = kbPos + 2;
			while (endPos < packageName.length() && isdigit(packageName[endPos])) {
				endPos++;
			}
			kbStr = packageName.substr(kbPos, endPos - kbPos);
		}

		// If a KB number is found and the URL ends with 'q=', append it
		if (!kbStr.empty()) {
			size_t qPos = info.Description.find("q=");
			if (qPos != std::string::npos) {
				// Check if it's already populated to avoid duplicates
				if (info.Description[qPos + 2] == ')') {
					info.Description.insert(qPos + 2, kbStr);
				}
			}
		}
	}

	return info;
}

void DatabaseManager::LogRemoval(std::string packageName, std::string type) {
	if (!isConnected) return;

	// Inserts a new history record
	const char* sql = "INSERT INTO RemovalHistory (PackageName, Type) VALUES (?, ?);";
	sqlite3_stmt* stmt;

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
		// Binds the package name and type to the query
		sqlite3_bind_text(stmt, 1, packageName.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, type.c_str(), -1, SQLITE_STATIC);
		// Executes the insert
		sqlite3_step(stmt);
		// Cleans up
		sqlite3_finalize(stmt);
	}
}