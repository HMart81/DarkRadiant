; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=DarkRadiant
AppVerName=DarkRadiant 2.6.1pre1 x86
AppPublisher=The Dark Mod
AppPublisherURL=http://www.darkradiant.net
AppSupportURL=http://www.darkradiant.net
AppUpdatesURL=http://www.darkradiant.net
DefaultDirName={pf}\DarkRadiant
DefaultGroupName=DarkRadiant 2.6.1pre1 x86
OutputDir=.
OutputBaseFilename=darkradiant-2.6.1pre1-x86
Compression=lzma
SolidCompression=yes
;ArchitecturesAllowed=x64
;ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\install\darkradiant.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\install\*"; Excludes: "*.pdb,*.exp,*.lib,*.in,*.fbp,*.iobj,*.ipdb"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Redist\MSVC\14.12.25810\x86\Microsoft.VC141.CRT\msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Redist\MSVC\14.12.25810\x86\Microsoft.VC141.CRT\vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion

; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\DarkRadiant"; Filename: "{app}\darkradiant.exe";
Name: "{group}\{cm:UninstallProgram,DarkRadiant}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\DarkRadiant"; Filename: "{app}\darkradiant.exe"; Tasks: desktopicon

[InstallDelete]
; Remove all modules that are now integrated in DarkRadiant.exe
Type: files; Name: {app}\modules\shaders.dll;
Type: files; Name: {app}\modules\skins.dll;
Type: files; Name: {app}\modules\sound.dll;
Type: files; Name: {app}\modules\uimanager.dll;
Type: files; Name: {app}\modules\vfspk3.dll;
Type: files; Name: {app}\modules\xmlregistry.dll;
Type: files; Name: {app}\modules\archivezip.dll;
Type: files; Name: {app}\modules\commandsystem.dll;
Type: files; Name: {app}\modules\eclassmgr.dll;
Type: files; Name: {app}\modules\entity.dll;
Type: files; Name: {app}\modules\eventmanager.dll;
Type: files; Name: {app}\modules\filetypes.dll;
Type: files; Name: {app}\modules\filters.dll;
Type: files; Name: {app}\modules\fonts.dll;
Type: files; Name: {app}\modules\image.dll;
Type: files; Name: {app}\modules\mapdoom3.dll;
Type: files; Name: {app}\modules\md5model.dll;
Type: files; Name: {app}\modules\model.dll;
Type: files; Name: {app}\modules\particles.dll;
Type: files; Name: {app}\modules\scenegraph.dll;
Type: files; Name: {app}\modules\script.dll;
; Remove the legacy WaveFront plugin before installation
Type: files; Name: {app}\plugins\wavefront.dll;
; Grid module has been removed
Type: files; Name: {app}\modules\grid.dll;
; eclasstree module has been removed
Type: files; Name: {app}\modules\eclasstree.dll;
; entitylist module has been removed
Type: files; Name: {app}\modules\entitylist.dll;
; undo module has been removed
Type: files; Name: {app}\modules\undo.dll;