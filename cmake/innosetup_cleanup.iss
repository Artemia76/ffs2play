; Remove files left by the former qmake / Qt5 installers (flat layout in {app})
[InstallDelete]
Type: files; Name: "{app}\ffs2play.exe"
Type: files; Name: "{app}\Qt5*.dll"
Type: files; Name: "{app}\libcrypto-1_1*.dll"
Type: files; Name: "{app}\libssl-1_1*.dll"
Type: files; Name: "{app}\ffs2play.qss"
Type: files; Name: "{app}\vc_redist.x64.exe"
Type: filesandordirs; Name: "{app}\translations\ffs2play_*.qm"
Type: filesandordirs; Name: "{app}\platforms"
Type: filesandordirs; Name: "{app}\sqldrivers"
Type: filesandordirs; Name: "{app}\iconengines"
Type: filesandordirs; Name: "{app}\styles"
Type: filesandordirs; Name: "{app}\imageformats"
Type: filesandordirs; Name: "{app}\bearer"
;
; Update started by FFS2Play (/SILENT) : the [Run] entry of CPack is skipped in silent mode, start the new version
[Run]
Filename: "{app}\bin\ffs2play.exe"; Flags: nowait; Check: WizardSilent
