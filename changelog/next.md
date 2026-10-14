## Bug Fixes

- Fixed ScriptModel not sending role updates on changed data that compares equal by key or structure.
- Fixed Hyprland workspaces keeping their old id after Hyprland changes it.
- Fixed desktop entries being parsed incorrectly with trailing whitespace or CRLF line endings.
- Fixed `WlSessionLock.locked` changes not being emitted on unlock.
- Fixed notification images with unpadded rows being decoded incorrectly.
- Fixed a possible crash when a notification image is smaller than its stated size.
