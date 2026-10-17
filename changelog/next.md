## New Features

- Added `SystemTrayItem.contextMenu()` to open menus of tray items that draw their own, such as Wine applications.

## Bug Fixes

- Fixed ScriptModel not sending role updates on changed data that compares equal by key or structure.
- Fixed Hyprland workspaces keeping their old id after Hyprland changes it.
- Fixed desktop entries being parsed incorrectly with trailing whitespace or CRLF line endings.
- Fixed `WlSessionLock.locked` changes not being emitted on unlock.
- Fixed tray items disappearing or getting stuck when another tray item with a similar bus name exited.
- Fixed a crash when a tray item sends an icon pixmap smaller than its stated size.
- Fixed tray items with a missing or nonstandard status being reported as passive.
- Fixed tray items with a missing or nonstandard category being reported as hardware.
- Fixed `SystemTray.items` not being consistently sorted by category.
- Fixed log spam when Chromium/Electron and Qt 6 tray items fail reads of properties they do not provide.
- Fixed tray icons not updating when only their icon theme path changed.
- Fixed tray icons briefly failing to load when their icon name and theme path changed together.
- Fixed tray overlay pixmaps being ignored for items with a named icon.
- Fixed icon requests combining a theme path and fallback, including paths containing URL reserved characters.
