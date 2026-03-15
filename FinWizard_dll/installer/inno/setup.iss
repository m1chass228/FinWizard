#define MyAppName "FinWizard"
#define MyAppVersion "1.0"
#define MyAppExeName "FinWizardGui.exe"
#define MyAppPublisher "FinWizard Creator"

[Setup]
; Базовые настройки
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
; Куда устанавливать по умолчанию (autopf - это Program Files или AppData в зависимости от прав)
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
; Папки откуда брать и куда сохранять передаются снаружи из GitHub Actions
OutputDir={#MyOutDir}
OutputBaseFilename=FinWizard_Setup
Compression=lzma2
SolidCompression=yes
; Позволяет устанавливать программу даже без прав Администратора!
PrivilegesRequired=lowest
; Иконка для удаления программы в панели управления
UninstallDisplayIcon={app}\{#MyAppExeName}

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Берем все файлы из папки релиза и кладем в папку установки
Source: "{#MySourceDir}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#MySourceDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
; Ярлыки в пуске и на рабочем столе
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Запуск программы после успешной установки
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
