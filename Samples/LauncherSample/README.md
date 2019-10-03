# Launcher sample

This C++ sample creates a launcher that sets the current working directory for your application entry-point at runtime. This should be considered as a workaround, ideally you should update your main process code to set the CWD. 
See this blog post for more details on how to properly set your current working directory or alternately use the launcher: [Accessing to the files in the installation folder in a Desktop Bridge application](http://blogs.msdn.microsoft.com/appconsult/2017/06/23/accessing-to-the-files-in-the-installation-folder-in-a-desktop-bridge-application/).

The sample also includes the capability to launch an application prior to the launching of the main app. 
A example scenario: You need to set the app license for a Win32 app prior to the app launching. 

This sample must be run in an MSIX or APPX Package. The LauncherPackage project in the sample does this. The ArgEcho project is a sample 'pre-install' app. The MainApp project is a sample main app. These are included to demonstrate how the launcher can be packaged and debugged using the Windows Application Packaging Project (LauncherPackage project).

An optional third and fourth line have been added to the **launcher.cfg** file to support the pre-launch app. The third line is the relative path to the pre-launch EXE. The forth line, if set to 'onetime', will restrict the pre-launch app to launching only once. See below.

Build and Run the sample
-------------------------

 - Build the solution.
 - Copy Launcher.exe and Launcher.cfg from the build path to the root of your Windows package.
 - (Optional) Copy your pre-launch EXE to the root of your Windows Package
 - Update your application entrypoint in your package.appxmanifest to Launcher.exe.
 - Configure Launcher.cfg content: <br />
	Put your win32 application executable name that on the first line. <br />
	Put the current working directory as a relative path. <br />
 <br />
	Content of Launcher.cfg should look like this: (lines 3 and 4 optional) <br />
	MyappEntrypoint.exe <br />
	Win32\MyappDirecotory\ <br/>
    ..\ArgEcho\ArgEcho.exe -online -serial "6RABC WXUJB EHUJH 5MNK0" -silent <br/>
    onetime <br/>
