# 1.2.0
- Updated to `Geode 6.0.0` and `Geometry Dash 2.2091`.
- Migrated to the new `geode::TextInput` (callbacks instead of `TextInputDelegate`).
- Fixed a crash when opening the editor: GD 2.2091 has a new D-pad move menu where `EditButtonBar` does not exist.
- Y-axis flipping is now done by the parser (fixes the macOS build, `flipObjectsY` has no macOS binding in 2.2091).
- Migrated file handling to `asp::fs::Path`.
- The popup now uses the standard `show()` animation and respects the `Fast Menu` option.
- The popup can no longer be closed while a file is being picked or parsed.

# 1.1.2
- Moved the guide to Github Wiki.

# 1.1.1
- Asynchronous file loading.
- Refactored the mod.
- New animation for the Popup.

# 1.0.1
- Linked to `Geode 5.0.0-beta.1`.

# 1.0.0
- Ported to `Geode v5`.
- Added `Help` button inside popup.
