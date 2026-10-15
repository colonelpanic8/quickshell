## Bug Fixes

- Fixed ScriptModel not sending role updates on changed data that compares equal by key or structure.
- Fixed Hyprland workspaces keeping their old id after Hyprland changes it.
- Fixed desktop entries being parsed incorrectly with trailing whitespace or CRLF line endings.
- Fixed `WlSessionLock.locked` changes not being emitted on unlock.
- Fixed a crash when unplugging a screen being captured by ScreencopyView or trying to capture the fallback screen.
- Fixed most broken qml types in qmlls/qmllint for Qt 6.12.
- Fixed icons not being found in a theme path given as a file url, with an extension, or as a directory containing or inside an icon theme.
