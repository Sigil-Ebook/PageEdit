; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

#define AppName "PageEdit"

[Setup]
AppName={#AppName}
AppVerName={#AppName} ${PAGEEDIT_FULL_VERSION}
VersionInfoVersion=${PAGEEDIT_FULL_VERSION}
DefaultDirName={autopf}\{#AppName}
AllowNoIcons=yes
DisableDirPage=no
DefaultGroupName={#AppName}
UninstallDisplayIcon={app}\{#AppName}.exe
AppPublisher=Sigil-Ebook
AppPublisherURL=https://github.com/Sigil-Ebook/
; WizardImageFile=compiler:wizmodernimage-IS.bmp
; WizardSmallImageFile=compiler:wizmodernsmallimage-IS.bmp
Compression=lzma2/ultra
SolidCompression=yes
OutputDir=${OUTPUT_PACKAGE_DIR}
LicenseFile=${LICENSE_LOCATION}
; Win 7sp1 is the lowest supported version
MinVersion=0,6.1.7601
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline dialog
OutputBaseFilename={#AppName}-${PAGEEDIT_FULL_VERSION}-Windows${ISS_SETUP_FILENAME_PLATFORM}-Setup
;ChangesAssociations=yes
;SetupLogging=yes

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
Source: "{#AppName}\*"; DestDir: "{app}"; Flags: createallsubdirs recursesubdirs ignoreversion
Source: vendor\vcredist.exe; DestDir: {tmp}

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "{#AppName}"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom

; Cancel runtime install if desired.
Name: vcruntimeadmin; Description: "Check if bundled VS runtime install is necessary? (admin required)"; Types: full custom; Check: IsAdminInstallMode
Name: vcruntimeuser; Description: "Check if bundled VS runtime install is necessary? (admin required)"; Types: full custom; Check: not IsAdminInstallMode

;[Registry]
; Add {#AppName} as a global file handler for (X)HTML.
;Root: HKLM; Subkey: "Software\Classes\.html\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey
;Root: HKLM; Subkey: "Software\Classes\.xhtml\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

; Optional desktop icon.
; commondesktop if admin, userdesktop if not
Components: dicon; Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"


[Run]
; The following command detects whether or not the c++ runtime need to be installed.
Components: vcruntimeadmin; Filename: {tmp}\vcredist.exe; Check: IsAdminInstallMode and NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; StatusMsg: Checking for VC++ RunTime ...
Components: vcruntimeuser; Filename: {tmp}\vcredist.exe; Check: (not IsAdminInstallMode) and NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; Flags: runasoriginaluser; StatusMsg: Checking for VC++ RunTime ...

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

function VCinstalled(const regKey: string): Boolean;
{ Function for Inno Setup Compiler }
{ Returns True if same or later Microsoft Visual C++ 2015 Redistributable is installed, otherwise False. }
var
  major: Cardinal;
  minor: Cardinal;
  bld: Cardinal;
  rbld: Cardinal;
  installed_ver, min_ver: String;

begin
  Result := False;
  { Mimimum version of the VC++ Redistributable needed (currently 14.26.28 and later) }
  min_ver := '14.26.28000.0';
  Log('Minimum VC++ 2015-2019 Redist version is: ' + min_ver);

  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Major', major) then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Minor', minor) then begin
      if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Bld', bld) then begin
        if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'RBld', rbld) then begin
            installed_ver := IntToStr(major) + '.' + IntToStr(minor) + '.' + IntToStr(bld) + '.' + IntToStr(rbld);
            Log('VC++ 2015-2019 Redist version is: ' + installed_ver);
            { Version info was found. Return true if later or equal to our min_ver redistributable }
            // Note brackets required because of weird operator precendence
            //Result := (major >= 14) and (minor >= 23) and (bld >= 27820) and (rbld >= 0)
            //Log('Installed Version ' + installed_ver + ' >= Minimum Version ' + min_ver + Format(': %d', [IntToStr((CompareVersion(installed_ver, min_ver) >= 0))]));
            Result := (CompareVersion(installed_ver, min_ver) >= 0)
        end;
      end;
    end;
  end;
end;

function NeedsVCRedistInstall: Boolean;
begin
  if NOT IsWin64 then
    { 32-bit OS, 32-bit installer }
    Result := not (VCinstalled('SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X86'))
  else if Is64BitInstallMode then
    { 64-bit OS, 64-bit installer }
    Result := not (VCinstalled('SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64'))
  else
    { 64-bit OS, 32-bit installer }
    Result := not (VCinstalled('SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86'));  
end;

(* function NeedsVCRedistInstall: Boolean;
// Return True if VC++ redist included with PageEdit Installer should be installed.
var
  reg_key, installed_ver, min_ver: String;
  R: Integer;
begin
  Result := True;
  // Mimimum version of the VC++ Redistributable needed (currently VS2017 and later).
  min_ver := '14.10.00000';
  //MsgBox('Did runtime check', mbInformation, MB_OK);
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
 end; *)

function CmdLineParamExists(const Value: string): Boolean;
var
  I: Integer;
begin
  Result := False;
  for I := 1 to ParamCount do
    if CompareText(Uppercase(ParamStr(I)), Value) = 0 then
    begin
      Result := True;
      Exit;
    end;
end;

// Disable ability to install VS runtime in "for current user only" mode
procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectComponents then
    if (not IsAdminInstallMode) and ((not CmdLineParamExists('/SILENT')) and (not CmdLineParamExists('/VERYSILENT'))) then
    begin
      WizardForm.ComponentsList.Checked[2] := False;
      //WizardForm.ComponentsList.ItemEnabled[2] := False;
    end;
end;

// Warn "for current user only" installers that someone will need to
// make sure a compatible version of the VS runtime is installed.
function NextButtonClick(CurPageID: Integer): Boolean;
var
  msg: String;
begin
  Result := True;
   msg := 'The option to check/install the VS' + #13#10 +
          'runtime is unchecked. Please make sure a' + #13#10 +
          'compatible version of the Visual Studio' + #13#10 +
          'VC++ runtime is already installed (by you' + #13#10 +
          'or an admin), or click "No" and check' + #13#10 +
          'the box before proceeding.' + #13#10 + #13#10 +
          'You will need admin privileges to' + #13#10 +
          'to install the runtime.' + #13#10 + #13#10 +
          'Do you wish to proceed as is?';
  if CurPageID = wpSelectComponents then begin
    if IsAdminInstallMode then begin
      if (not WizardIsComponentSelected('vcruntimeadmin')) then
        Result := SuppressibleMsgBox(msg, mbInformation, MB_YESNO, IDYES) = IDYES
    end else
      if (not WizardIsComponentSelected('vcruntimeuser')) then
        Result := SuppressibleMsgBox(msg, mbInformation, MB_YESNO, IDYES) = IDYES;
  end;
end;
