; -- Example1.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

#define AppVersion GetFileVersion("release\orion.exe")

[Setup]
AppName=Orion
AppVersion={#AppVersion}
DefaultDirName={pf}\Orion
DefaultGroupName=Orion
UninstallDisplayIcon={app}\orion.exe
Compression=lzma2
SolidCompression=yes
OutputDir=.
#ifdef x64
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#endif

[Files]
Source: "release\*"; DestDir: "{app}\bin"; Flags: ignoreversion recursesubdirs

[Icons]
Name: "{group}\Orion"; Filename: "{app}\bin\orion.exe"
