## New Features

- Added `SystemTrayItem.contextMenu()` to open menus of tray items that draw their own, such as Wine applications.

## Bug Fixes

- Fixed ScriptModel not sending role updates on changed data that compares equal by key or structure.
- Fixed Hyprland workspaces keeping their old id after Hyprland changes it.
- Fixed desktop entries being parsed incorrectly with trailing whitespace or CRLF line endings.
- Fixed `WlSessionLock.locked` changes not being emitted on unlock.
- Fixed a crash when unplugging a screen being captured by ScreencopyView or trying to capture the fallback screen.
- Fixed most broken qml types in qmlls/qmllint for Qt 6.12.
- Fixed tray items disappearing or getting stuck when another tray item with a similar bus name exited.
- Fixed a crash when a tray item sends an icon pixmap smaller than its stated size.
- Fixed tray items with a missing or nonstandard status being reported as passive.
- Fixed tray items with a missing or nonstandard category being reported as hardware.
- Fixed `SystemTray.items` not being consistently sorted by category.
- Fixed log spam when Chromium/Electron and Qt 6 tray items fail reads of properties they do not provide.
