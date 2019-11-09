; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

[Setup]
AppName=PageEdit
AppVerName=PageEdit ${PAGEEDIT_FULL_VERSION}
VersionInfoVersion=${PAGEEDIT_FULL_VERSION}
DefaultDirName={pf}\PageEdit
DisableDirPage=no
DefaultGroupName=PageEdit
UninstallDisplayIcon={app}\PageEdit.exe
AppPublisher=Sigil-Ebook
AppPublisherURL=https://github.com/Sigil-Ebook/
WizardImageFile=compiler:wizmodernimage-IS.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-IS.bmp
Compression=lzma2/ultra
SolidCompression=yes
OutputDir=${OUTPUT_PACKAGE_DIR}
LicenseFile=${LICENSE_LOCATION}
; Win 7sp1 is the lowest supported version
MinVersion=0,6.1.7601
PrivilegesRequired=admin
OutputBaseFilename=PageEdit-${PAGEEDIT_FULL_VERSION}-Windows${ISS_SETUP_FILENAME_PLATFORM}-Setup
;ChangesAssociations=yes

; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
; The ${ISS_ARCH} var is substituted with "x64" or an empty string
ArchitecturesAllowed="${ISS_ARCH}"
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; The ${ISS_ARCH} var is substituted with "x64" or an empty string
ArchitecturesInstallIn64BitMode="${ISS_ARCH}"

[Files]
Source: "PageEdit\*"; DestDir: "{app}"; Flags: createallsubdirs recursesubdirs ignoreversion
Source: vendor\vcredist.exe; DestDir: {tmp}

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "PageEdit"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom
Name: dicon\common; Description: "For all users"; Types: full custom; Flags: exclusive
Name: dicon\user; Description: "For the current user only"; Flags: exclusive

;[Registry]
; Add PageEdit as a global file handler for (X)HTML.
;Root: HKLM; Subkey: "Software\Classes\.html\OpenWithList\PageEdit.exe"; Flags: uninsdeletekey
;Root: HKLM; Subkey: "Software\Classes\.xhtml\OpenWithList\PageEdit.exe"; Flags: uninsdeletekey

[Icons]
Name: "{group}\PageEdit"; Filename: "{app}\PageEdit.exe"
Name: "{group}\Uninstall PageEdit"; Filename: "{uninstallexe}"
; Optional desktop icon.
Components: dicon\common; Name: "{commondesktop}\PageEdit"; Filename: "{app}\PageEdit.exe"
Components: dicon\user; Name: "{userdesktop}\PageEdit"; Filename: "{app}\PageEdit.exe"

;[InstallDelete]
;Type: filesandordirs; Name: "{app}\python3"
; Might be moving to precompiled python files in future.
; and to keep install directory clean for future enhancement possibilities


[Run]
; The following command detects whether or not the c++ runtime need to be installed.
Filename: {tmp}\vcredist.exe; Check: NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; StatusMsg: Checking for VC++ RunTime ...

[Code]

function CompareVersion(V1, V2: string): Integer;
// Compare version strings
// Returns 0, if the versions are equal.
// Returns -1, if the V1 is older than the V2.
// Returns 1, if the V1 is newer than the V2.
var
  P, N1, N2: Integer;
begin
  Result := 0;
  while (Result = 0) and ((V1 <> '') or (V2 <> '')) do
  begin
    P := Pos('.', V1);
    if P > 0 then
    begin
      N1 := StrToInt(Copy(V1, 1, P - 1));
      Delete(V1, 1, P);
    end
      else
    if V1 <> '' then
    begin
      N1 := StrToInt(V1);
      V1 := '';
    end
      else
    begin
      N1 := 0;
    end;

    P := Pos('.', V2);
    if P > 0 then
    begin
      N2 := StrToInt(Copy(V2, 1, P - 1));
      Delete(V2, 1, P);
    end
      else
    if V2 <> '' then
    begin
      N2 := StrToInt(V2);
      V2 := '';
    end
      else
    begin
      N2 := 0;
    end;

    if N1 < N2 then Result := -1
      else
    if N1 > N2 then Result := 1;
  end;
end;


function NeedsVCRedistInstall: Boolean;
// Return True if VC++ redist included with PageEdit Installer needs to be installed.
var
  reg_key, installed_ver, min_ver: String;
  R: Integer;
begin
  Result := True;
  // Mimimum version of the VC++ Redistributable needed (currently VS2017 and later).
  min_ver := '14.10.00000';
  if IsWin64 and not Is64BitInstallMode then
    // 32-bit version being installed on 64-bit machine
    reg_key := 'SOFTWARE\WoW6432Node\Microsoft\DevDiv\vc\servicing\14.0\RuntimeMinimum'
  else
    reg_key := 'SOFTWARE\Microsoft\DevDiv\vc\servicing\14.0\RuntimeMinimum';

  // If there's a compatible version of the 14.XX runtime already installed; use it.
  if RegQueryStringValue(HKEY_LOCAL_MACHINE, reg_key, 'Version', installed_ver) then
    begin
      //MsgBox('Registry key: ' + reg_key, mbInformation, MB_OK);
      //MsgBox('Installed version: ' + installed_ver, mbInformation, MB_OK);
      //MsgBox('Minimum version: ' + min_ver, mbInformation, MB_OK);
      R := CompareVersion(installed_ver, min_ver);
      // If installed VC++ runtime version is equal or newer than
      // the minimum version specified, then don't run
      // the bundled VC++ redistributable installer
      if R >= 0 then
        Result := False;
    end
 end;
