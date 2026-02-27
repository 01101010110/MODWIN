The artifact I chose to use for all three of my enhancements is my program called MODWIN. MODWIN was created in 2022 (initially only as a few batch files) and it is an open-source tool that will help users to modify their Windows ISOs in any way they want to (removing bloatware, adding custom files, custom registry entries, optional auto-installer, etc.). 

I chose to include MODWIN in my ePortfolio because this project is the reason I came to college. I wanted to learn how to build a frontend for my program and I couldn’t understand how to do it myself before college. I set out at the beginning of this term with the goal of improving my backend skills and learning frontend skills, and thanks to my Professor I was allowed to work on this project the entire term to achieve these goals. 

I learned so incredibly much in improving my program, it’s hard to even know where to begin. The first thing I learned was how to modularize my code so I could make my work more collaborative. I then learned how to add imgui to the code for the frontend and started to learn how to build out the menu and all the different buttons and placements. I learned how to add the option for the users to select a file or folder for various scenarios. I learned how to build an uninstall script to delete a program that is running. I learned how to add a right click context menu item that will allow users to take ownership of any file or folder they want to and be able to delete it. I learned how to build a database for the apps, packages, and features and have it instantly load an entry when an item is hovered over. 

I faced many challenges throughout the entire process, all of these updates were new to me. The most annoying trouble was trying to find all the links for the database. The most troublesome problem that held me behind was trying to get the unattended logic to succeed. By succeed, the outcome is to have a Windows Installation ISO automatically install itself when ran. In order to do this, you have to structure  the xml template in a particular way, alphabetically and with the right options, and I kept making tiny mistakes that would cost me hours in testing, as I had to constantly build the ISO, test, fail, uninstall MODWIN, reinstall MODWIN, rinse repeat.

The enhancements improved the artifact by making it easier for anyone to use my program. Prior to the enhancements, MODWIN was only a console based program, which could be daunting for normal people. Adding a database improved the project by giving the user a quick run down of what the item they select does, and can shoot them over to Microsoft’s official documentation to help them learn further. The algrothym requirement of this class help me to improve how the apps, packages, and features displayed to the users, alphabetically and with a filter now, instead of whatever order DISM spit out. 

Lastly, of the five course outcomes,

Course Outcomes:

•	Employ strategies for building collaborative environments that enable diverse audiences to support organizational decision making in the field of computer science

•	Design, develop, and deliver professional-quality oral, written, and visual communications that are coherent, technically sound, and appropriately adapted to specific audiences and contexts

•	Design and evaluate computing solutions that solve a given problem using algorithmic principles and computer science practices and standards appropriate to its solution, while managing the trade-offs involved in design choices

•	Demonstrate an ability to use well-founded and innovative techniques, skills, and tools in computing practices for the purpose of implementing computer solutions that deliver value and accomplish industry-specific goals

•	Develop a security mindset that anticipates adversarial exploits in software architecture and designs to expose potential vulnerabilities, mitigate design flaws, and ensure privacy and enhanced security of data and resources

I was able to achieve all of these by modularizing the code, adding a GUI, adding a database to the program, adding guardrails for safety like with the uninstall options , STOP OPERATION button, and  red warnings on potentially dangerous options. The apps packages and features displaying knowledge of their items helps users to make a more safe and informed decision about whether or not to keep the item. The items also are now algorithmically sorted and may be filtered if the user likes.
