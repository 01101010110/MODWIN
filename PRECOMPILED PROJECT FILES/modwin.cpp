#define NOMINMAX  // Prevents the Windows headers from defining the min() and max() macros (conflicts with std::min and std::max)
#include <windows.h> // Includes the Windows API header for Windows-specific functions and macros
#include <iostream> // Includes the IO stream library for standard input/output operations (e.g., std::cout, std::cin)
#include <string> // Includes the standard string class library (std::string) for features like handling and manipulating text, concatenation, searching, and more.
#include <cstdlib> // Includes the C standard library for general-purpose functions like memory allocation, process control, conversions, etc.
#include <fstream> // Includes the file stream library for file input/output operations (e.g., std::ifstream, std::ofstream)
#include <limits> // Includes the limits library which provides ways to query properties of fundamental arithmetic types (e.g., min/max values)
#include <vector>    // Includes the standard vector class, a dynamic array that allows elements to be added or removed and automatically manages storage
#include <filesystem>// Includes the filesystem library to work with files and directories, such as file manipulation and path information
#include <regex>     // Includes the regular expression library for pattern matching and text searching/manipulating using regex patterns
#include <set>
// Includes generated header files for the xorriso binary to be able to be unpacked to user's system. 
#include "gitignore.h" // Xorriso is our iso builder
#include "LICENSE.h" // these files were converted from file format to arrays
#include "README_md.h" // so that we can build the files on user's computer using only code
#include "cygiconv_2_dll.h"
#include "cygwin1_dll.h"
#include "resources_rc.h"
#include "xorriso_exe.h"

namespace fs = std::filesystem;

// Function declarations to help compilers, as well as the code in this script is constructed as ordered below
int main();
bool IsUserAdmin();
bool FileExists(const std::string& filename);
bool DirectoryExists(const std::string& dirName);
void BuildModwinFolder(const fs::path& exePath);
void ShowMenu();
void SourceWIM();
void HandleWIM();
void HandleESD();
void MountWIM();
void Apps();
void AddApp();
void RemoveApp();
void RemoveAllApps();
void Packages();
void RemovePackage();
void RemoveAllPackages();
void AddPackage();
void Features();
void RemoveFeature();
void RemoveAllFeatures();
void EnableFeature();
void MountWIMRegistry();
void OpenRegistryHive(const std::string& hiveName);
void UnloadRegistryHive();
void PushUserFolderToWIM();
void BuildOptions();
void SaveChanges();
void CompressWIM();
void BuildISO();
void DiscardChanges();
void UnmountWIM();
void Credits();

// Main Function for MODWIN
int main() {
    // Checks if user is Admin
    if (!IsUserAdmin()) {
        std::cout << "Administrative privileges required.\n";
        SHELLEXECUTEINFO sei = { sizeof(sei) };
        sei.lpVerb = "runas";
        sei.lpFile =                                 "MODWIN.exe"; 
        sei.hwnd = NULL;
        sei.nShow = SW_NORMAL;
        if (!ShellExecuteEx(&sei)) {
            std::cerr << "Failed to gain administrative privileges.\n";
        }
        return 1;
    }

    // Obtain the absolute path of the executable
    fs::path exePath = fs::absolute(fs::current_path() / "MODWIN.exe"); 

    // Checks if the C:/MODWIN directory exists
    if (!DirectoryExists("C:\\MODWIN")) {
        BuildModwinFolder(exePath); // Pass the executable path to BuildModwinFolder
    }
    else {
        system("explorer C:\\MODWIN\\ISO"); // Open File Explorer to C:\MODWIN\ISO
        ShowMenu(); // Takes user to the main menu
    }
    return 0;
}

// Function to check for administrative privileges
bool IsUserAdmin() {
    BOOL isAdmin = FALSE; // Declare a BOOL variable to store the admin status, initially set to FALSE
    PSID adminGroup; // Declare a pointer to a security identifier (SID) for the admin group
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY; // Initialize a SID_IDENTIFIER_AUTHORITY structure with the value for the NT authority system
    // Allocate and initialize a SID for the administrators group
    if (!AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        return false; // If SID allocation fails, return false
    }
    // Check if the calling thread's access token has the specified SID
    if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) { // If check fails
        isAdmin = FALSE; // Set isAdmin to FALSE
    }
    FreeSid(adminGroup); // Free the memory allocated for the SID
    return isAdmin == TRUE; // Return true if the user is an admin
}

// Function to check if a file exists and is accessible
bool FileExists(const std::string& filename) {
    std::ifstream file(filename); // Creates an ifstream object 'file' and attempts to open the file with the name provided in 'filename'.
    return file.good(); // Returns true if the file stream is in a good state (i.e., the file exists and is accessible), otherwise returns false.
}

// Function to check if a directory exists
bool DirectoryExists(const std::string& dirName) {
    DWORD ftyp = GetFileAttributesA(dirName.c_str()); // Use GetFileAttributesA to get attributes of the file/directory
    if (ftyp == INVALID_FILE_ATTRIBUTES)  // Check if the returned attributes indicate an error or invalid file path
        return false; // The path is invalid or an error occurred
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY) // Check if the attributes include FILE_ATTRIBUTE_DIRECTORY
        return true; // The path points to a directory
    return false; // The path does not point to a directory
}

// Function to build MODWIN's directory structure on user PC
void BuildModwinFolder(const std::filesystem::path& exePath) {
    // Creates the Directories as listed below
    std::filesystem::create_directory("C:\\MODWIN");
    std::filesystem::create_directory("C:\\MODWIN\\APPS");
    std::filesystem::create_directory("C:\\MODWIN\\BIN");
    std::filesystem::create_directory("C:\\MODWIN\\BIN\\xorriso");  
    std::filesystem::create_directory("C:\\MODWIN\\ISO");
    std::filesystem::create_directory("C:\\MODWIN\\MOD");
    std::filesystem::create_directory("C:\\MODWIN\\PACKAGES");
    std::filesystem::create_directory("C:\\MODWIN\\PATH");
    std::filesystem::create_directory("C:\\MODWIN\\USER");
    // Defines the output directory as the new xorriso directory
    const std::filesystem::path outputDir = "C:\\MODWIN\\BIN\\xorriso";
    // List of all header files with arrays to unpack
    std::vector<std::pair<const unsigned char*, size_t>> files = {
     {_gitignore, _gitignore_len},
     {LICENSE, LICENSE_len},
     {README_md, README_md_len},
     {cygiconv_2_dll, cygiconv_2_dll_len},
     {cygwin1_dll, cygwin1_dll_len},
     {resources_rc, resources_rc_len},
     {xorriso_exe, xorriso_exe_len}
    };
    // The names of the files to be created
    std::vector<std::string> filenames = {
        ".gitignore",
        "LICENSE",
        "README.md",
        "cygiconv-2.dll",
        "cygwin1.dll",
        "resources.rc",
        "xorriso.exe"
    };
    // Iterate over the arrays and write them to files in the xorriso directory
    for (size_t i = 0; i < files.size(); ++i) {
        std::ofstream outFile(outputDir / filenames[i], std::ios::binary);
        outFile.write(reinterpret_cast<const char*>(files[i].first), files[i].second);
    }
    system("explorer C:\\MODWIN\\ISO"); // Opens file explorer to the ISO folder of MODWIN so they can paste their files in
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes user to Main Menu
}


// Function for MODWIN's Main Menu
void ShowMenu() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); // Get the console handle
    // Banner for MODWIN ASCII art and version
    std::string modwinBanner = R"( 
=========================================================================
||                                                                     ||
||  1111   1111  000000   111111     00  00  00  111111   00      00   ||
||  11 11 11 11 00    00  111  111   00  00  00    11     0000    00   ||
||  11  111  11 00    00  111   111  00  00  00    11     00 00   00   ||
||  11   11  11 00    00  111   111  00  00  00    11     00  00  00   ||
||  11       11 00    00  111   111  00  00  00    11     00   00 00   ||
||  11       11  000000   11111111   0000000000  111111   00    0000   ||
||                                                                  V5 ||
=========================================================================                      
Main Menu Options: 
==================
)";
    // Banner for the menu options
    std::string menuBanner = R"(
1. Extract WIM
2. Mount WIM
3. WIM's Application Package Menu
4. WIM's Package Menu
5. WIM's Feature Menu
6. WIM's Registry Options
7. Push USER folder to WIM
8. Unmount WIM and Build ISO Options
9. Exit
0. Credits and Thanks to Our Community!
)";
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY); // Change text color to green 
    std::cout << modwinBanner; // Output the MODWIN banner
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset to default color
    std::cout << menuBanner << std::endl; // Output the menu banner
    int choice; // Variable to store the user's menu selection
    int validChoices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }; // Allowed choices
    int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
    while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
        std::cout << "Type a number above and press enter: "; // Prints message to screen
        if (!(std::cin >> choice)) { // Check if the input is not an integer
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear(); // Clear the error state of the cin object
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
            continue; // Continue to the next iteration of the loop
        }
        bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
        for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
            if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                isValidChoice = true; // If it matches, set isValidChoice to true
                break; // Break out of the loop as we've found a valid choice
            }
        }
        if (!isValidChoice) { // Not a valid choice
            std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
        }
        else {
            break; // Exit the loop if a valid choice is made
        }
    }
    switch (choice) { // Switch cases for user choice, takes user to selected menu.
    case 1:
        SourceWIM();
        break;
    case 2:
        MountWIM();
        break;
    case 3:
        Apps();
        break;
    case 4:
        Packages();
        break;
    case 5:
        Features();
        break;
    case 6:
        MountWIMRegistry();
        break;
    case 7:
        PushUserFolderToWIM();
        break;
    case 8:
        BuildOptions();
        break;
    case 9:
        std::cout << "Exiting program.\n";
        exit(0); // Closes the program
    case 0:
        Credits();
        break;
    default:
        std::cout << "Invalid option. Please try again.\n";
    }
}

// Function to check if our unpacked iso contains a wim or an esd install file
void SourceWIM() {
    std::string wimPath = "C:\\MODWIN\\ISO\\sources\\install.wim"; // Sets the 'wimPath' string to point to 'C:\MODWIN\ISO\sources\install.wim'
    std::string esdPath = "C:\\MODWIN\\ISO\\sources\\install.esd"; // Sets the 'esdPath' string to point to 'C:\MODWIN\\ISO\sources\install.esd'
    system("cls"); // Clear the console screen
    // Checks if a wimPath or an esdPath is detected:
    if (FileExists(wimPath)) { // If a wimPath file is detected
        std::cout << "==================\n"; // Prints message to the screen
        std::cout << "Detected WIM file.\n"; // Prints message to the screen
        std::cout << "==================\n"; // Prints message to the screen
        HandleWIM(); // Hops over to the WIM extraction menu
    }
    else if (FileExists(esdPath)) { // If an esdPath file is detected
        std::cout << "==================\n"; // Prints message to the screen
        std::cout << "Detected ESD file.\n"; // Prints message to the screen
        std::cout << "==================\n"; // Prints message to the screen
        HandleESD(); // Hops over to the ESD extraction menu
    }
    else { // If no esdPath or wimPath is detected
        std::cout << "Error: No WIM or ESD file found in the source directory.\n"; // Prints message to the screen
        std::cout << "Press any key to return to the Main Menu.\n"; // Prints message to the screen
        system("pause>nul"); // Pause the program
        system("cls"); // Clear the console screen
        ShowMenu(); // Return to the main menu
    }
}

// Function for the WIM extraction menu, for extracting a WIM from a WIM of the user's choosing
void HandleWIM() {
    system("Dism /Get-WimInfo /WimFile:\"C:\\MODWIN\\ISO\\sources\\install.wim\""); // Runs the DISM command to retrieve the sources in the WIM for extraction.
    int sourceIndex; // Declares an integer variable 'sourceIndex' to store the user's choice of image index for extraction.
    std::cout << "\nType an Index Number and press enter: ";  // Prints message to the screen, prompting the user to enter the index number of the source image they want to extract.
    std::cin >> sourceIndex; // Reads the user's input into sourceIndex
    system("cls"); // Clear the console screen
    std::cout << "============================\n"; // Prints message to the screen
    std::cout << "Preparing to extract the WIM\n"; // Prints message to the screen
    std::cout << "============================\n"; // Prints message to the screen
    // Constructs a DISM command to export a specific image from the WIM file into another WIM file with maximum compression
    std::string dismExportCommand = "dism /export-image /SourceImageFile:\"C:\\MODWIN\\ISO\\sources\\install.wim\" /SourceIndex:" + std::to_string(sourceIndex) + " /DestinationImageFile:\"C:\\MODWIN\\ISO\\sources\\install1.wim\" /Compress:max /CheckIntegrity";
    system(dismExportCommand.c_str()); // Executes the constructed DISM command
    std::cout << "\nPress any key to continue.\n"; // Prints message to the screen, informing the user that the extraction process is complete
    system("pause>nul"); // Pause the program
    system("del \"C:\\MODWIN\\ISO\\sources\\install.wim\""); // Deletes the original WIM file
    system("ren \"C:\\MODWIN\\ISO\\sources\\install1.wim\" install.wim"); // Renames the extracted WIM file from 'install1.wim' to 'install.wim'
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes the user back to the main menu
}

// Function for the ESD extraction menu, for extracting a WIM from the ESD of the user's choosing
void HandleESD() {
    system("Dism /Get-WimInfo /WimFile:\"C:\\MODWIN\\ISO\\sources\\install.esd\""); // Runs the DISM command to retrieve the sources in the ESD for extraction to WIM format.
    int sourceIndex; // Declares an integer variable 'sourceIndex' to store the user's choice of image index for extraction.
    std::cout << "\nType an Index Number and Press Enter: "; // Prints message to the screen, prompting the user to enter the index number of the source image they want to extract.
    std::cin >> sourceIndex; // Reads the user's input into sourceIndex
    system("cls"); // Clear the console screen
    std::cout << "============================\n"; // Prints message to the screen
    std::cout << "Preparing to extract the WIM\n"; // Prints message to the screen
    std::cout << "============================\n"; // Prints message to the screen
    // Constructs a DISM command to export a specific image from the ESD file to a WIM (Windows Imaging Format) file with maximum compression
    std::string dismCommand = "dism /export-image /SourceImageFile:\"C:\\MODWIN\\ISO\\sources\\install.esd\" /SourceIndex:" + std::to_string(sourceIndex) + " /DestinationImageFile:\"C:\\MODWIN\\ISO\\sources\\install.wim\" /Compress:max /CheckIntegrity";
    system(dismCommand.c_str()); // Executes the constructed DISM command
    std::cout << "\nPress any key to continue.\n"; // Prints message to the screen, informing the user that the extraction process is complete
    system("pause>nul"); // Pause the program
    system("del \"C:\\MODWIN\\ISO\\sources\\install.esd\""); // Deletes the original ESD file
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes the user back to the Main Menu
}

// Function to mount the WIM using DISM
void MountWIM() {
    system("cls"); // Clear the console screen
    std::string mountWimPath = "C:\\MODWIN\\ISO\\sources\\install.wim"; // Sets the 'mountWimPath' string to point to 'C:\MODWIN\ISO\sources\install.wim'
    //Then checks if the WIM file exists to save user time and prevent further errors  
    if (FileExists(mountWimPath)) { // If the WIM file Does exist
        std::cout << "==========================\n"; // Prints message to the screen
        std::cout << "Preparing to mount the WIM\n"; // Prints message to the screen
        std::cout << "==========================\n"; // Prints message to the screen
        system("dism.exe /mount-wim /wimfile:\"C:\\MODWIN\\ISO\\sources\\install.wim\" /mountdir:\"C:\\MODWIN\\PATH\" /index:1"); // Mounts the WIM file and exposes it's contents in the PATH folder of MODWIN
        std::cout << "\nPress any key to continue.\n"; // Prints success message
        system("pause>nul"); // Pause the program
        system("cls"); // Clear the console screen
        ShowMenu(); // Takes the user back to the Main Menu
    }  
    else { // If the WIM file does not exist
        std::cout << "Error: WIM file not found at " << mountWimPath << ". Please ensure the file exists and try again.\n";
        std::cout << "Press any key to return to the Main Menu.\n"; // Prints message to the screen
        system("pause>nul"); // Pause the program
        system("cls"); // Clear the console screen
        ShowMenu(); // Return to the main menu  
    }
}

// Function for the WIM Application Package Menu
void Apps() {
    system("cls"); // Clear the console screen
    std::cout << "========================\n"; // Prints message to screen
    std::cout << "Application Package Menu\n"; // Prints message to screen
    std::cout << "========================\n"; // Prints message to screen
    std::cout << "\n1. Remove Applications \n"; // Prints message to screen
    std::cout << "2. Remove All Applications \n"; // Prints message to screen
    std::cout << "3. Install Applications \n"; // Prints message to screen
    std::cout << "4. Return to Main Menu\n"; // Prints message to screen
    int choice; // Variable to store the user's menu selection
    int validChoices[] = { 1, 2, 3, 4 }; // Allowed choices
    int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
    while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
        std::cout << "\nType a number above and press enter: ";
        if (!(std::cin >> choice)) { // Check if the input is not an integer
            std::cout << "Invalid input. Please enter a number.\n";
            std::cin.clear(); // Clear the error state of the cin object
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
            continue; // Continue to the next iteration of the loop
        }
        bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
        for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
            if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                isValidChoice = true; // If it matches, set isValidChoice to true
                break; // Break out of the loop as we've found a valid choice
            }
        }
        if (!isValidChoice) { // Not a valid choice
            std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
        }
        else {
            break; // Exit the loop if a valid choice is made
        }
    }
    switch (choice) {
    case 1:
        RemoveApp(); // Takes user to the RemoveApp menu
        break;
    case 2:
        RemoveAllApps(); // Takes user to Remove All Apps
        break;
    case 3:
        AddApp(); // Takes user to the AddApp menu
        break;
    case 4:
        system("cls"); // Clear the console screen
        ShowMenu(); // Return to the main menu
        break;
    default:
        std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
    }
}

// Function for the Remove Application menu
void RemoveApp() {
    system("cls"); // Clear the console screen
    // Dism command to get and list provisioned application packages, who's output is saved to a text file called apps.txt
    system("C:\\Windows\\System32\\dism.exe /Image:C:\\MODWIN\\PATH /Get-ProvisionedAppxPackages > C:\\MODWIN\\apps.txt");
    system("find \"PackageName : \" C:\\MODWIN\\apps.txt > C:\\MODWIN\\newapps.txt"); // Searchs for "PackageName : " and prints only those lines to newapps.txt
    std::ifstream inFile("C:\\MODWIN\\newapps.txt");// Open the newapps.txt file
    if (!inFile) { // if file not available
        std::cerr << "Failed to open C:\\MODWIN\\newapps.txt\n"; // Prints message to screen
        Apps(); // Exit the function if file opening fails
    }
    std::string line; // Declare a string to hold each line read from the file
    while (std::getline(inFile, line)) { // Read lines from inFile one by one into 'line' until the end of the file is reached
        size_t startPos = line.find("PackageName : "); // Find the start position of "PackageName : " in the line
        if (startPos != std::string::npos) { // Check if the substring is found
            // Print the package name, excluding the "PackageName : " part
            std::cout << line.substr(startPos + std::string("PackageName : ").length()) << '\n';
        }
    }
    inFile.close(); // Close the file after reading
    std::string packageName; // Ask user for the package name to remove
    std::cout << "\nCopy and paste an application from above to remove: "; // Prints message to screen
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer
    std::getline(std::cin, packageName); // Read the full line of input for the package name
    // Constructs the Dism command to remove the selected application package
    std::string removeCommand = "dism /Image:C:\\MODWIN\\PATH /Remove-ProvisionedAppxPackage /PackageName:\"" + packageName + "\"";
    system(removeCommand.c_str()); // Execute the command
    std::cout << "Press any key to continue.\n"; // Prints message to the screen
    system("pause>nul"); // Pauses so the user can verify 
    system("cls"); // Clear the console screen
    system("del C:\\MODWIN\\apps.txt"); // Removes leftover apps text file
    system("del C:\\MODWIN\\newapps.txt"); // Removes leftover newapps text file
    Apps(); // Takes the user back to apps
}

// Function to remove all applications listed in newapps.txt
void RemoveAllApps() {
    system("cls"); // Clear the console screen
    // Dism command to get and list provisioned application packages, who's output is saved to a text file called apps.txt
    system("C:\\Windows\\System32\\dism.exe /Image:C:\\MODWIN\\PATH /Get-ProvisionedAppxPackages > C:\\MODWIN\\apps.txt");
    system("find \"PackageName : \" C:\\MODWIN\\apps.txt > C:\\MODWIN\\newapps.txt"); // Searchs for "PackageName : " and prints only those lines to newapps.txt
    std::ifstream inFile("C:\\MODWIN\\newapps.txt"); // Open the newapps.txt file
    if (!inFile) { // if file not available
        std::cerr << "Failed to open C:\\MODWIN\\newapps.txt\n"; // Prints message to screen
        return; // Exit the function if file opening fails
    }
    std::string line; // Variable to hold each line
    while (std::getline(inFile, line)) { // Read lines from inFile one by one
        size_t startPos = line.find("PackageName : "); // Find the start position of "PackageName : " in the line
        if (startPos != std::string::npos) {
            std::string packageName = line.substr(startPos + std::string("PackageName : ").length()); // Extract the package name
            // Constructs the DISM command to remove the application package
            std::string removeCommand = "C:\\Windows\\System32\\dism.exe /Image:C:\\MODWIN\\PATH /Remove-ProvisionedAppxPackage /PackageName:\"" + packageName + "\"";
            system(removeCommand.c_str()); // Execute the command
            std::cout << packageName << " removed.\n"; // Inform user of removal
        }
    }
    inFile.close(); // Close the file after processing

    std::cout << "\nAll applications removed. Press any key to continue.\n"; // Prints message to the screen
    system("pause>nul"); // Pauses so the user can see the output
    system("cls"); // Clear the console screen
    system("del C:\\MODWIN\\apps.txt"); // Removes leftover apps text file
    system("del C:\\MODWIN\\newapps.txt"); // Removes leftover newapps text file
    ShowMenu();
}

// Function for the Add Application menu
void AddApp() {
    system("cls"); // Clear the console screen
    namespace fs = std::filesystem; // Alias for the filesystem namespace
    std::string appsDirectory = "C:\\MODWIN\\APPS"; // Path to the apps directory
    std::vector<std::string> appFiles; // Vector to store app file paths
    std::cout << "Available apps in " << appsDirectory << ":\n"; // List the contents of the folder
    for (const auto& entry : fs::directory_iterator(appsDirectory)) {
        if (fs::is_regular_file(entry)) { // Check if the entry is a regular file
            std::cout << entry.path().filename() << '\n'; // Print the file name
            appFiles.push_back(entry.path().string()); // Add the file path to the vector
        }
    }
    char choice;
    std::cout << "\nDo you want to install all items? (Y/N): "; // Print message to screen
    std::cin >> choice; // Read user's choice
    if (tolower(choice) == 'y') { // If user selects the yes option
        for (const auto& appPath : appFiles) { // Install all items
            // Constructs Dism command to add application packages to the WIM
            std::string addCommand = "dism /Image:C:\\MODWIN\\PATH /Add-ProvisionedAppxPackage /PackagePath:" + appPath + " /SkipLicense";
            system(addCommand.c_str()); // Execute the command for each app
        }
    }
    else { // If user selects anything other than 'y' or 'Y' 
        Apps(); // Takes user back to the Application Menu
    }
    std::cout << "App(s) added. Press any key to continue.\n"; // Prints message to screen
    system("pause>nul"); // Pause the program
    system("cls"); // Clear the console screen
    Apps(); // Takes user back to the Application Package Menu
}

// Function for the Packages Menu
void Packages() {
    system("cls"); // Clear the console screen
    std::cout << "====================\n"; // Prints message to screen
    std::cout << "Package Manager Menu\n"; // Prints message to screen
    std::cout << "====================\n"; // Prints message to screen
    std::cout << "\n1. Remove installed packages\n"; // Prints message to screen
    std::cout << "2. Remove all packages\n"; // Prints message to screen
    std::cout << "3. Install custom packages\n"; // Prints message to screen
    std::cout << "4. Return to Main Menu\n"; // Prints message to screen
    int choice; // Variable to store the user's menu selection
    int validChoices[] = { 1, 2, 3, 4 }; // Allowed choices
    int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
    while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
        std::cout << "\nType a number above and press enter: "; // Prints message to screen
        if (!(std::cin >> choice)) { // Check if the input is not an integer
            std::cout << "Invalid input. Please enter a number.\n"; // Prints message to screen
            std::cin.clear(); // Clear the error state of the cin object
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
            continue; // Continue to the next iteration of the loop
        }
        bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
        for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
            if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                isValidChoice = true; // If it matches, set isValidChoice to true
                break; // Break out of the loop as we've found a valid choice
            }
        }
        if (!isValidChoice) { // Not a valid choice
            std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
        }
        else {
            break; // Exit the loop if a valid choice is made
        }
    }
    switch (choice) { // Switch cases for the user selection, takes user to selected option
    case 1:
        RemovePackage();
        break;
    case 2:
        RemoveAllPackages();
        break;
    case 3:
        AddPackage();
        break;
    case 4:
        system("cls"); // Clear the console screen
        ShowMenu(); // Return to the main menu
        break;
    default:
        std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
    }
}

// Function for the Remove Package menu
void RemovePackage() {
    system("cls"); // Clear the console screen
    // Dism command to get the list of packages installed on the wim and save it to a text file
    system("dism /Image:C:\\MODWIN\\PATH /Get-Packages > C:\\MODWIN\\packages.txt");
    // Command to search packages.txt for lines containing 'Package Identity : ' and prints those to a new text file.
    system("find \"Package Identity : \" C:\\MODWIN\\packages.txt > C:\\MODWIN\\newpackages.txt");
    std::ifstream inFile("C:\\MODWIN\\newpackages.txt"); // Open the newpackages.txt file
    if (!inFile) { // If the file fails to open
        std::cerr << "Failed to open C:\\MODWIN\\newpackages.txt\n"; // Display error message
        Packages(); // Take user back to packages
    }
    std::string line; // Declare a string to hold each line read from the file
    while (std::getline(inFile, line)) { // Read lines from inFile one by one into 'line' until the end of the file is reached
        size_t startPos = line.find("Package Identity : "); // Search for the start position of the substring "Package Identity : " in the line
        if (startPos != std::string::npos) {
            // If the substring is found, extract and print the part of the line following it
            std::cout << line.substr(startPos + std::string("Package Identity : ").length()) << '\n';
        }
    }
    inFile.close(); // Close the file after reading all lines
    std::string packageName; // Declare a string to store the package name entered by the user
    std::cout << "\nCopy and paste a package from above to remove: "; // Prompt user to enter a package name
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer before reading new input
    std::getline(std::cin, packageName); // Use getline to handle spaces in package names
    // Constructs the Dism command to remove packages
    std::string removeCommand = "dism /Image:C:\\MODWIN\\PATH /Remove-Package /PackageName:" + packageName;
    system(removeCommand.c_str()); // Executes the command
    std::cout << "Package removed. Press any key to continue.\n"; // Prints message to screen
    system("pause>nul"); // Pause the program
    system("cls"); // Clear the console screen
    system("del C:\\MODWIN\\newpackages.txt"); // Removes leftover newpackages text file
    system("del C:\\MODWIN\\packages.txt"); // Removes leftover packages text file
    Packages(); // Takes user back to the Packages Menu
}

// Function to remove all "safe" packages listed in newpackages.txt
void RemoveAllPackages() {
    system("cls"); // Clear the console screen
    system("dism /Image:C:\\MODWIN\\PATH /Get-Packages > C:\\MODWIN\\packages.txt");
    system("find \"Package Identity : \" C:\\MODWIN\\packages.txt > C:\\MODWIN\\newpackages.txt");

    std::ifstream inFile("C:\\MODWIN\\newpackages.txt");
    std::ofstream safeFile("C:\\MODWIN\\safe.txt");
    if (!inFile) {
        std::cerr << "Failed to open C:\\MODWIN\\newpackages.txt\n";
        Packages();
        return;
    }
    if (!safeFile) {
        std::cerr << "Failed to open or create C:\\MODWIN\\safe.txt\n";
        Packages();
        return;
    }

    std::vector<std::string> safePackagePrefixes = {
        "Microsoft-OneCore-ApplicationModel",
        "Microsoft-OneCore-DirectX",
        "Microsoft-Windows-Hello",
        "Microsoft-Windows-InternetExplorer",
        "Microsoft-Windows-LanguageFeatures-Handwriting",
        "Microsoft-Windows-LanguageFeatures-OCR",
        "Microsoft-Windows-LanguageFeatures-Speech",
        "Microsoft-Windows-LanguageFeatures-TextToSpeech",
        "Microsoft-Windows-MSPaint",
        "Microsoft-Windows-MediaPlayer",
        "Microsoft-Windows-PowerShell",
        "Microsoft-Windows-Printing",
        "Microsoft-Windows-QuickAssist",
        "Microsoft-Windows-StepsRecorder",
        "Microsoft-Windows-TabletPCMath",
        "Microsoft-Windows-WMIC",
        "Microsoft-Windows-Wallpaper",
        "Microsoft-Windows-WordPad",
        "OpenSSH-Client-Package"
    };

    std::string line;
    std::cout << "Identifying safe packages to remove...\n";

    while (std::getline(inFile, line)) {
        for (const auto& prefix : safePackagePrefixes) {
            if (line.find(prefix) != std::string::npos) {
                safeFile << line << '\n';
                break;
            }
        }
    }

    inFile.close();
    safeFile.close();

    std::ifstream safePackagesFile("C:\\MODWIN\\safe.txt");
    std::cout << "Removing safe packages...\n";
    while (std::getline(safePackagesFile, line)) {
        size_t startPos = line.find("Package Identity : ");
        if (startPos != std::string::npos) {
            std::string packageIdentity = line.substr(startPos + std::string("Package Identity : ").length());
            std::string removeCommand = "dism /Image:C:\\MODWIN\\PATH /Remove-Package /PackageName:" + packageIdentity;
            if (system(removeCommand.c_str()) != 0) {
                std::cerr << "Failed to remove package: " << packageIdentity << '\n';
            }
            else {
                std::cout << "Removed package: " << packageIdentity << '\n';
            }
        }
    }

    safePackagesFile.close();
    std::cout << "\nAll safe packages have been removed. Press any key to continue.\n";
    system("pause>nul");
    system("cls");

    system("del C:\\MODWIN\\newpackages.txt");
    system("del C:\\MODWIN\\packages.txt");
    system("del C:\\MODWIN\\safe.txt");

    Packages();
}

void AddPackage() {
    system("cls"); // Clear the console screen
    namespace fs = std::filesystem; // Alias for the filesystem namespace
    std::string packagesDirectory = "C:\\MODWIN\\PACKAGES"; // Path to the packages directory
    std::vector<std::string> packageFiles; // Vector to store package file paths

    std::cout << "Available packages in " << packagesDirectory << ":\n"; // List the contents of the folder
    for (const auto& entry : fs::directory_iterator(packagesDirectory)) {
        if (fs::is_regular_file(entry)) { // Check if the entry is a regular file
            std::cout << entry.path().filename() << '\n'; // Print the file name
            packageFiles.push_back(entry.path().filename().string()); // Add the file name to the vector
        }
    }

    char choice;
    std::cout << "\nDo you want to install all items? (Y/N): "; // Print message to screen
    std::cin >> choice; // Read user's choice
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the newline character in the input stream

    if (tolower(choice) == 'y') { // If user selects the yes option
        for (const auto& packageName : packageFiles) { // Install all items
            std::string fullPath = packagesDirectory + "\\" + packageName;
            std::ifstream file(fullPath);
            if (file) {
                // Constructs Dism command to add packages to the WIM
                std::string addCommand = "dism /Image:C:\\MODWIN\\PATH /Add-Package /PackagePath:\"" + fullPath + "\"";
                std::cout << "Executing command: " << addCommand << std::endl;
                int result = system(addCommand.c_str()); // Execute the command for each package
                if (result != 0) {
                    std::cout << "Error: DISM command failed with error code " << result << std::endl;
                }
            }
            else {
                std::cout << "Error: Cannot access file " << fullPath << std::endl;
            }
            file.close();
        }
    }
    else { // If user selects anything other than 'y' or 'Y'
        Packages(); // Takes the user back to the Package Menu
        return; // Exit the function
    }

    std::cout << "\nPackage(s) added. Press any key to continue.\n"; // Prints message to screen
    system("pause>nul"); // Pause the program
    system("cls"); // Clear the console screen
    Packages(); // Takes the user back to the Package Menu
}


    // Function for the Features Menu
    void Features() {
        system("cls"); // Clear the console screen
        std::cout << "=======================\n";
        std::cout << "Feature Management Menu\n";
        std::cout << "=======================\n";
        std::cout << "\n1. Disable Features\n";
        std::cout << "2. Disable All Features\n";
        std::cout << "3. Enable Features\n";
        std::cout << "4. Return to Main Menu\n";
        int choice; // Variable to store the user's menu selection
        int validChoices[] = { 1, 2, 3, 4 }; // Allowed choices
        int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
        while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
            std::cout << "\nType a number above and press enter: "; // Prints message to screen
            if (!(std::cin >> choice)) { // Check if the input is not an integer
                std::cout << "Invalid input. Please enter a number.\n"; // Prints message to screen
                std::cin.clear(); // Clear the error state of the cin object
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
                continue; // Continue to the next iteration of the loop
            }
            bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
            for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
                if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                    isValidChoice = true; // If it matches, set isValidChoice to true
                    break; // Break out of the loop as we've found a valid choice
                }
            }
            if (!isValidChoice) { // Not a valid choice
                std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
            }
            else {
                break; // Exit the loop if a valid choice is made
            }
        }
        switch (choice) { // Switch cases for user selection, takes user to selected menu
        case 1:
            RemoveFeature(); // Takes user to the RemoveFeature menu
            break;
        case 2:
            RemoveAllFeatures(); // Takes user to the RemoveAllFeatures menu
            break;
        case 3:
            EnableFeature(); // Takes user to the EnableFeature menu
            break;
        case 4:
            system("cls"); // Clear the console screen
            ShowMenu(); // Takes user back to Main Menu
            break;
        default:
            std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
        }
    }

// Function that provides a menu to allow user to remove features
    void RemoveFeature() {
        system("cls"); // Clear the console screen
        system("Dism /Image:C:\\MODWIN\\PATH /Get-Features /Format:Table > C:\\MODWIN\\features.txt");
        system("findstr \"Enabled\" C:\\MODWIN\\features.txt > C:\\MODWIN\\enabledFeatures.txt");

        std::ifstream inFile("C:\\MODWIN\\enabledFeatures.txt");
        if (!inFile) {
            std::cerr << "Failed to open C:\\MODWIN\\enabledFeatures.txt\n";
            Features();
            return;
        }

        std::string line;
        std::cout << "=========================\n";
        std::cout << "List of Enabled Features:\n";
        std::cout << "=========================\n";
        while (std::getline(inFile, line)) {
            // Skip header lines
            if (line.find("Version") != std::string::npos ||
                line.find("Features listing for package") != std::string::npos) {
                continue;
            }

            // Remove "| Enabled"
            size_t enabledPos = line.find("| Enabled");
            if (enabledPos != std::string::npos) {
                line.erase(enabledPos, std::string("| Enabled").length());
            }

            // Trim leading and trailing spaces from the line
            line.erase(0, line.find_first_not_of(" \t")); // Leading spaces
            line.erase(line.find_last_not_of(" \t") + 1); // Trailing spaces

            std::cout << line << '\n';
        }

        inFile.close();

        std::string featureName;
        std::cout << "\nEnter the feature name to disable: ";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore characters in the input buffer up to the maximum stream size or until a newline character is encountered.
        std::getline(std::cin, featureName); // Use getline to handle spaces in feature names
        // Construct and execute the Dism command to disable the feature
        std::string removeCommand = "Dism /Image:C:\\MODWIN\\PATH /Disable-Feature /FeatureName:" + featureName;
        system(removeCommand.c_str()); // Executes the command
        std::cout << "\nFeature disabled. Press any key to continue.\n"; // Prints to screen
        system("pause>nul"); // Pauses
        system("cls"); // Clear the console screen
        std::remove("C:\\MODWIN\\features.txt"); // Removes the features text file
        std::remove("C:\\MODWIN\\enabledFeatures.txt"); // Removes the enabledFeatures text file
        Features(); // Returns user to Features menu
    }
    // Function that provides a menu to allow user to remove all features
    void RemoveAllFeatures() {
        system("cls"); // Clear the console screen
        // Run DISM command and output to a text file
        system("Dism /Image:C:\\MODWIN\\PATH /Get-Features /Format:Table > C:\\MODWIN\\features.txt");
        // Extract only the lines that contain "Enabled" to a new file
        system("findstr /c:\"Enabled\" C:\\MODWIN\\features.txt > C:\\MODWIN\\enabledFeatures.txt");

        std::ifstream inFile("C:\\MODWIN\\enabledFeatures.txt");
        if (!inFile) {
            std::cerr << "Failed to open C:\\MODWIN\\enabledFeatures.txt\n";
            Features(); // Return to the features menu if the file can't be opened
            return;
        }

        std::string line;
        std::cout << "Disabling all enabled features...\n";

        // Read each line from the file
        while (std::getline(inFile, line)) {
            // Skip header lines
            if (line.find("Feature Name") != std::string::npos ||
                line.find("State") != std::string::npos ||
                line.find("----") != std::string::npos ||
                line.find("Deployment Image Servicing and Management tool") != std::string::npos) {
                continue; // Skip the current iteration
            }

            // Remove all occurrences of the vertical bar and "Enabled"
            line.erase(std::remove(line.begin(), line.end(), '|'), line.end());
            size_t enabledPos = line.find("Enabled");
            if (enabledPos != std::string::npos) {
                line.erase(enabledPos, std::string("Enabled").length());
            }

            // Trim leading and trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (!line.empty()) {
                // Construct and execute the DISM command to disable the feature
                std::string disableCommand = "Dism /Image:C:\\MODWIN\\PATH /Disable-Feature /FeatureName:" + line;
                system(disableCommand.c_str());
                std::cout << "Disabled feature: " << line << '\n';
            }
        }

        inFile.close(); // Close the file stream

        std::cout << "\nAll features have been disabled. Press any key to continue.\n";
        system("pause>nul"); // Pause the console without displaying any message
        system("cls"); // Clear the console screen after resuming

        // Cleanup by deleting the temporary text files
        std::remove("C:\\MODWIN\\features.txt");
        std::remove("C:\\MODWIN\\enabledFeatures.txt");

        Features(); // Return to the features menu
    }


    // Function to Enable Features on the WIM
    void EnableFeature() {
        system("cls"); // Clear the console screen
        // Run DISM command and output to a text file
        system("Dism /Image:C:\\MODWIN\\PATH /Get-Features /Format:Table > C:\\MODWIN\\features.txt");
        // Extract only the lines that contain "Disabled" to a new file
        system("findstr /c:\"Disabled\" C:\\MODWIN\\features.txt > C:\\MODWIN\\disabledFeatures.txt");

        std::ifstream inFile("C:\\MODWIN\\disabledFeatures.txt");
        if (!inFile) {
            std::cerr << "Failed to open C:\\MODWIN\\disabledFeatures.txt\n";
            ShowMenu(); // Return to the menu if the file can't be opened
            return;
        }

        std::string line;
        std::cout << "==========================\n";
        std::cout << "List of Disabled Features:\n";
        std::cout << "==========================\n";

        while (std::getline(inFile, line)) {
            // Check for unwanted lines and phrases and skip them
            if (line.find("Feature Name") != std::string::npos ||
                line.find("State") != std::string::npos ||
                line.find("----") != std::string::npos ||
                line.find("Deployment Image Servicing and Management tool") != std::string::npos ||
                line.find("with Payload Removed") != std::string::npos) {
                continue; // Skip the current iteration
            }

            // Remove all occurrences of the vertical bar
            line.erase(std::remove(line.begin(), line.end(), '|'), line.end());

            // Remove the word "Disabled" from the line if present
            size_t disabledPos = line.find("Disabled");
            if (disabledPos != std::string::npos) {
                line.replace(disabledPos, std::string("Disabled").length(), ""); // Replace with empty string
            }

            // Trim leading and trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (!line.empty()) {
                std::cout << line << '\n'; // Print the line if it's not empty
            }
        }

        inFile.close(); // Close the file stream

        std::string featureName;
        std::cout << "\nEnter the feature name to enable: ";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear the input buffer
        std::getline(std::cin, featureName); // Get the user input for the feature name

        // Construct and execute the DISM command to enable the feature
        std::string enableCommand = "Dism /Image:C:\\MODWIN\\PATH /Enable-Feature /FeatureName:" + featureName;
        system(enableCommand.c_str()); // Execute the command

        std::cout << "\nFeature enabled. Press any key to continue.\n";
        system("pause>nul"); // Pause the console without displaying any message
        system("cls"); // Clear the console screen after resuming

        // Cleanup by deleting the temporary text files
        std::remove("C:\\MODWIN\\features.txt");
        std::remove("C:\\MODWIN\\disabledFeatures.txt");

        Features(); // Return to the features menu
    }



// Function for the WIM Registry Hive Menu
void MountWIMRegistry() {
    // Check if C:\MODWIN\PATH\Windows exists
    if (!DirectoryExists("C:\\MODWIN\\PATH\\Windows")) { // If PATH/Windows does Not exist
        system("cls"); // Clear the console screen
        std::cout << "Error: 'C:\\MODWIN\\PATH\\Windows' does not exist. Make sure your WIM is mounted before proceeding.\n"; // Prints message to screen
        std::cout << "Press any key to return to the main menu.\n"; // Prints message to screen
        system("pause>nul"); // Pause the program
        system("cls"); // Clear the console screen
        ShowMenu(); // Returns user to Main Menu
    }
    system("cls"); // Clear the console screen
    std::cout << "======================\n"; // Prints message to screen
    std::cout << "WIM Registry Hive Menu\n"; // Prints message to screen
    std::cout << "======================\n"; // Prints message to screen
    std::cout << "\nPress 1 to open the SYSTEM Registry Hive\n"; // Prints message to screen
    std::cout << "Press 2 to open the SOFTWARE Registry Hive\n"; // Prints message to screen
    std::cout << "Press 3 to open the DEFAULT Registry Hive\n"; // Prints message to screen
    std::cout << "Press 4 to open the DRIVERS Registry Hive\n"; // Prints message to screen
    std::cout << "Press 5 to open the SAM Registry Hive\n"; // Prints message to screen
    std::cout << "Press 6 to return to the main MENU\n"; // Prints message to screen
    std::cout << "\nType a number and press enter: "; // Prints message to screen
    int choice; // Variable to store the user's menu selection
    int validChoices[] = { 1, 2, 3, 4, 5, 6 }; // Allowed choices
    int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
    while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
        std::cout << "Type a number above and press enter: "; // Prints message to screen
        if (!(std::cin >> choice)) { // Check if the input is not an integer
            std::cout << "Invalid input. Please enter a number.\n"; // Prints message to screen
            std::cin.clear(); // Clear the error state of the cin object
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
            continue; // Continue to the next iteration of the loop
        }
        bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
        for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
            if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                isValidChoice = true; // If it matches, set isValidChoice to true
                break; // Break out of the loop as we've found a valid choice
            }
        }
        if (!isValidChoice) { // Not a valid choice
            std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
        }
        else {
            break; // Exit the loop if a valid choice is made
        }
    }
    switch (choice) { // Switch case for user selection, takes user to selected menu
    case 1:
        OpenRegistryHive("SYSTEM");
        break;
    case 2:
        OpenRegistryHive("SOFTWARE");
        break;
    case 3:
        OpenRegistryHive("DEFAULT");
        break;
    case 4:
        OpenRegistryHive("DRIVERS");
        break;
    case 5:
        OpenRegistryHive("SAM");
        break;
    case 6:
        system("cls"); // Clear the console screen
        ShowMenu(); // Return to the main menu
    default:
        std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
    }
}

// Function for the WIM Registry Hive Menu
void OpenRegistryHive(const std::string& hiveName) {
    // Load the registry hive
    std::string command = "reg load HKLM\\OFFLINE C:\\MODWIN\\PATH\\Windows\\System32\\Config\\" + hiveName;
    system(command.c_str());
    system("cls"); // Clear the console screen
    std::cout << "Opening Registry Editor. Go to HKEY_LOCAL_MACHINE\\OFFLINE to see the loaded hive. \n"; // Print to the screen
    std::cout << "Press any key to continue.\n"; // Print to the screen
    system("pause>nul"); // Pause the program before opening regedit
    system("start /wait regedit"); // Open Registry Editor and wait for it to close
    system("cls"); // Clear the console screen
    UnloadRegistryHive(); // Starts the UnloadRegistryHive function
}

// Function to Unload the WIM's registry hive when user closes regedit
void UnloadRegistryHive() {
    system("reg unload HKLM\\OFFLINE"); // Command to execute the registry unload
    system("cls"); // Clear the console screen
    MountWIMRegistry(); // Returns user to the WIM Registry menu
}

// Function to push the /Contents/ of the USER folder in MODWIN to the C:\ on the WIM. So pack USER like it is the C folder
void PushUserFolderToWIM() {
    system("cls"); // Clear the console screen
    std::cout << "=================================\n"; // Prints message to screen
    std::cout << "Copying the USER folder to WIM...\n"; // Print message to screen
    std::cout << "=================================\n"; // Prints message to screen
    system("xcopy C:\\MODWIN\\USER C:\\MODWIN\\PATH /h /i /c /k /e /r /y"); // Execute the xcopy command to copy the files
    std::cout << "Copy operation completed. Press any key to continue.\n"; // Print message to screen
    system("pause>nul"); // Pause the program
    system("cls"); // Clear the console screen
    ShowMenu(); // Return to the main menu
}

// Function for the Unmount WIM and Build ISO menu
void BuildOptions() {
    system("cls"); // Clears the console screen
    std::cout << "==============================\n"; // Prints message to screen
    std::cout << "Unmount WIM and Build ISO Menu\n"; // Print message to screen
    std::cout << "==============================\n"; // Prints message to screen
    std::cout << "\n1. Unmount WIM, Cleanup, Save Changes, and Build ISO (Compresses WIM to ESD)\n";  // Print message to screen
    std::cout << "2. Unmount WIM and Discard Changes (if you make a mistake)\n";  // Print message to screen
    std::cout << "3. Unmount WIM Only and Save Changes (Keeps WIM in WIM format)\n";  // Print message to screen
    std::cout << "4. Build ISO Only\n";  // Print message to screen
    int choice; // Variable to store the user's menu selection
    int validChoices[] = { 1, 2, 3, 4 }; // Allowed choices
    int numberOfChoices = sizeof(validChoices) / sizeof(validChoices[0]); // Number of valid choices
    while (true) { // Use a while loop to continuously prompt for input until a valid choice is made
        std::cout << "\nType a number above and press enter: ";  // Print message to screen
        if (!(std::cin >> choice)) { // Check if the input is not an integer
            std::cout << "Invalid input. Please enter a number.\n";  // Print message to screen
            std::cin.clear(); // Clear the error state of the cin object
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore the rest of the current input line
            continue; // Continue to the next iteration of the loop
        }
        bool isValidChoice = false; // Initialize a flag to track if the user's choice is valid
        for (int i = 0; i < numberOfChoices; i++) { // Loop through all the valid choices to see if the user's choice matches one of them
            if (choice == validChoices[i]) { // Check if the user's choice matches the current valid choice
                isValidChoice = true; // If it matches, set isValidChoice to true
                break; // Break out of the loop as we've found a valid choice
            }
        }
        if (!isValidChoice) { // Not a valid choice
            std::cout << "Invalid option. Please try again.\n"; // Prints message to the screen
        }
        else {
            break; // Exit the loop if a valid choice is made
        }
    }
    switch (choice) {
    case 1:
        SaveChanges();
        break;
    case 2:
        DiscardChanges();
        break;
    case 3:
        UnmountWIM();
        break;
    case 4:
        BuildISO();
        break;
    default:
        std::cout << "Invalid option. Please try again.\n"; // Prints message to screen
    }
}

// Function to Unmount the WIM while saving all changes made to the WIM
void SaveChanges() {
    system("cls"); // Clear the console screen
    std::cout << "==============================================================\n";  // Print message to screen
    std::cout << "Saving Changes to the WIM, Cleaning Up, and Compressing to ESD\n";  // Print message to screen
    std::cout << "==============================================================\n";  // Print message to screen
    system("dism /Image:\"C:\\MODWIN\\PATH\" /cleanup-image /StartComponentCleanup /ResetBase"); // Used to reduce the size of the component store.
    system("dism /Unmount-Image /MountDir:\"C:\\MODWIN\\PATH\" /Commit"); // Dism command to unmount the WIM and Save the changes
    // Dism command to compress the WIM into an ESD 
    system("dism /export-image /SourceImageFile:\"C:\\MODWIN\\ISO\\sources\\install.wim\" /SourceIndex:1 /DestinationImageFile:\"C:\\MODWIN\\ISO\\sources\\install.esd\" /Compress:recovery /CheckIntegrity");
    system("del \"C:\\MODWIN\\ISO\\sources\\install.wim\""); // Deletes the old install.wim 
    std::cout << "\n"; // Adds a new line for aesthetics
    system("pause"); // Wait for user to return and press any key
    BuildISO(); // Takes user to the build iso menu
}

// Function to build a bootable Windows ISO
void BuildISO() {
    system("cls"); // Clear the console screen
    std::cout << "==============\nISO Build Menu\n==============\n";

    // Prompt the user for the ISO file name
    std::cout << "\nEnter a name for your ISO file (without the .iso extension): ";
    std::string isoFileName;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, isoFileName);

    std::string isoFilePath = "C:\\MODWIN\\MOD\\" + isoFileName + ".iso";

    // Build the xorriso command
    std::string xorrisoCommand = R"(C:\MODWIN\BIN\xorriso\xorriso )";
    xorrisoCommand += "-as mkisofs ";
    xorrisoCommand += "-iso-level 3 "; // Using ISO Level 3 for large file support
    xorrisoCommand += "-R ";
    xorrisoCommand += "-D ";
    xorrisoCommand += "-b efi/microsoft/boot/efisys_noprompt.bin ";
    xorrisoCommand += "-e efi/microsoft/boot/efisys_noprompt.bin ";
    xorrisoCommand += "-no-emul-boot ";
    xorrisoCommand += "-boot-load-size 4 ";
    xorrisoCommand += "-o " + isoFilePath + " ";
    xorrisoCommand += "/cygdrive/c/MODWIN/ISO"; // Adjusted path for Cygwin

    system(xorrisoCommand.c_str());
    std::cout << "============================================\n";
    std::cout << "Check " << isoFilePath << " to find your ISO\n";
    std::cout << "============================================\n";
    system("explorer C:\\MODWIN\\MOD");
    system("pause");
    system("cls");
    ShowMenu(); // Return to the main menu
}



// Function to unmount the WIM, discard changes, and clean-up the mount path
void DiscardChanges() {
    system("cls"); // Clear the console screen
    std::cout << "================================================\n"; // Print message to scree
    std::cout << "Unmounting and discarding the changes to the WIM\n"; // Print message to screen
    std::cout << "================================================\n"; // Print message to screen
    system("dism /Cleanup-mountpoints"); // DISM command to cleanup the mount points
    system("dism /Unmount-Image /MountDir:\"C:\\MODWIN\\PATH\" /discard"); // DISM command to discard the changes to the WIM 
    system("pause"); // Wait for user to press any key
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes user back to the Main Menu
}

// Function to unmount the WIM, Cleanup, and Save
void UnmountWIM() {
    system("cls"); // Clear the console screen
    std::cout << "=======================================================\n"; // Print message to scree
    std::cout << "Unmounting the Wim, Cleaning Up, and Saving the changes\n"; // Print message to screen
    std::cout << "=======================================================\n"; // Print message to screen
    system("dism /Image:\"C:\\MODWIN\\PATH\" /cleanup-image /StartComponentCleanup /ResetBase"); // Used to reduce the size of the component store.
    system("dism /Unmount-Image /MountDir:\"C:\\MODWIN\\PATH\" /Commit"); // Unmounts the WIM and Saves the changes
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes user back to the Main Menu
}

// Credits function
void Credits() {
    system("cls"); // Clear the console screen
    std::cout << "============================\n"; // Print message to screen
    std::cout << "Credits and Acknowledgements\n";  // Print message to screen
    std::cout << "============================\n"; // Print message to screen
    std::cout << "\nThis project wouldn't be possible without the help and support from our XDA community:\n"; // Print message to screen
    std::cout << "\nThe first thanks goes to Persona78, we learned how to build the iso in Oscdimg.exe originally  \n"; // Print message to screen
    std::cout << "\nThanks also goes to james28909, P414DIN, knigge111, Indospot, Xazac, elong7681 and Ro Kappa\n"; // Print message to screen
    std::cout << "a lot of bugs were worked out and features added to MODWIN thanks to their input \n"; // Print message to screen
    std::cout << "\nThanks to PeyTy for xorriso, an open source alternative to oscdimg\n"; // Print message to screen
    std::cout << "https://github.com/PeyTy/xorriso-exe-for-windows\n"; // Print message to screen
    std::cout << "\nDeveloped by Jenneh\n";
    std::cout << "\nPress any key to return to the main menu...\n"; // Print message to screen
    system("pause>nul"); // Pauses so the user has time to read the credits 
    system("cls"); // Clear the console screen
    ShowMenu(); // Takes user back to the Main Menu
}
