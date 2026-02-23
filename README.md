# MODWIN

A program to help users to customize a Windows 10 or 11 ISO in any way they want to. 

Click on the **How To Use MODWIN** thumbnail below this sentence for video instructions, or continue reading below for written instructions.

[![Click for Video](https://img.youtube.com/vi/P9A3MBgaSCM/hqdefault.jpg)](https://youtu.be/P9A3MBgaSCM)

## Step 1
A Windows Installation ISO is required to use MODWIN. 

You can obtain an ISO from Microsoft:
https://www.microsoft.com/en-us/software-download/windows11

Double click on your Windows ISO to mount it.

![OPEN ISO](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/1.png?raw=true) 

Take note of what drive Windows mounted the ISO to, then close the ISO. (for me it mounted to drive F)

![NOTE DRIVE](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/2.png?raw=true)


## Step 2
Open MODWIN and select where you would like to install. 

![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/3.png?raw=true)

![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/4.png?raw=true)

![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/5.png?raw=true)

## Step 3
After MODWIN installs, you may select the drive your ISO is mounted to, to extract the WIM.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/6.png?raw=true)

Select the drive you took note of before.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/7.png?raw=true)

After selecting the drive, you may select the version of Windows you want to extract. (Typically Home or Pro)
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/8.png?raw=true)

After selecting the version, you may extract the WIM. MODWIN will also copy the ISO files it needs to build the ISO later.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/9.png?raw=true)

MODWIN's action button will then turn to a "Working" text, and you can see the progress in the console at the bottom.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/10.png?raw=true)

## Step 4

Once the extraction is complete:
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/28.png?raw=true)

You can then select option 2 to mount the WIM.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/13.png?raw=true)

## Step 5

Once your WIM is mounted, you can access the apps, packages, and features installed on it. You may also remove any apps and packages you like, and enable / disable features. MODWIN also now has a built in database that will give you some information about what each item is and what it does when you hover over it. Right click on the item to pin it's knowledge base entry, to get a reference link that will take you to either the Microsoft Store Page for the item, or the Microsoft Learn Page for the iten. 

Please note removing any of these apps, packages, or features may have unintended consequences in the future, so be certain you need to remove them before you do. The items that are green and yellow, as of this current version of Windows, wihen removed the ISO will still boot. The consequences have not been tested, only the ability to boot and open a webpage.

### App Manager 
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/29.png?raw=true)

![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/14.png?raw=true)

You are able to add your own apps (Microsoft Store apps you download from https://store.rg-adguard.net/)
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/15.png?raw=true)

### Package Manager
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/16.png?raw=true)

You are able to add packages (drivers / Updates from https://www.catalog.update.microsoft.com/Home.aspx).
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/17.png?raw=true)

### Feature Manager
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/18.png?raw=true)

### Registry Manager --- WARNING: Only edit items in the OFFLINE Folder. If you don't know what a registry is, don't even use this option :-)
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/19.png?raw=true)

### Add Custom Files
You are able to add any files or folders to your iso, either by selecting them directly using the buttons, or by opening the path folder and dragging and dropping them where you like.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/20.png?raw=true)

A tool has been added to give your right click menu a "Take Ownership" command. Running this will allow you to be able to delete anything you want from the ISO. Please note this could break your image if you delete the wrong items. Remember to press the remove button when you are done using it, as it will not work if you uninstall MODWIN.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/21.png?raw=true)

## Step 6
Once you are done with your edits, you are now ready to build your ISO. There are some optional features you may select if you would like or need, such as the TPM bypass (for older machines / virtual machines - do mot install if you don't need), the skip pressing any key option (which saves you from having to frantically try to press a key when you first run the ISO), and the option to add an unattended file to the ISO that will automatically install Windows for you.

### Build ISO Menu
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/22.png?raw=true)

### Unattended Options -- Warning: Will Wipe the disk of the Computer / VM it is installing to, then it will automatically install Windows.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/23.png?raw=true)

## Step 7
When you are done building your ISO, you should uninstall MODWIN. This way the next time you make an ISO, it doesn't get corrupted by leftover files from a previous ISO.

### Uninstall
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/25.png?raw=true)

## Optional

### Stop Operation
OPTIONAL: If you ever need to stop an operation, the red button can handle that for you. MODWIN may freeze temporarily as it halts DISM services.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/11.png?raw=true)

After the stop operation command completes, you will see it's output in the console, you should be able to resume normally. However, if you were trying to build an ISO prior to running the stop command, you will have to rebuild it twice.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/12.png?raw=true)

### Unmount
If you ever need to unmount your image, you can do that here. You are also able to save your changes if you like, else it will discard it.
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/24.png?raw=true)

## Credits
This project would not have been possible without the help and support from all the lovely people below <3
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/26.png?raw=true)
![Open MODWIN](https://github.com/01101010110/MODWIN/blob/v7/PICTURE_INSTRUCTIONS/27.png?raw=true)














