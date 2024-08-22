; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

#define AppName "PageEdit"

[Setup]
AppName={#AppName}
AppVerName={#AppName} ${PAGEEDIT_FULL_VERSION}
AppVersion=${PAGEEDIT_FULL_VERSION}
VersionInfoVersion=${PAGEEDIT_FULL_VERSION}
DefaultDirName={autopf}\{#AppName}
AllowNoIcons=yes
DisableDirPage=no
DefaultGroupName={#AppName}
UninstallDisplayIcon={app}\{#AppName}.exe
AppPublisher=Sigil-Ebook
AppPublisherURL=https://github.com/Sigil-Ebook/
Compression=${INNO_COMP}
OutputDir=${OUTPUT_PACKAGE_DIR}
LicenseFile=${LICENSE_LOCATION}
; lowest supported Windows version
MinVersion=${WIN_MIN_VERSION}
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

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "{#AppName}"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"

; Optional desktop icon.
; commondesktop if admin, userdesktop if not
Components: dicon; Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
