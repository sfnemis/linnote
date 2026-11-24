# Troubleshooting

Common issues and solutions.

## Installation Issues

### "Command not found" after install

The binary is installed to `/usr/local/bin`. Try:

```bash
# Run directly
/usr/local/bin/linnote

# Or reinstall
curl -fsSL https://linnote.app/install.sh | bash
```

### Dependencies not found

Install required packages manually:

**Arch Linux:**
```bash
sudo pacman -S qt6-base qt6-wayland kf6-kwindowsystem kf6-kglobalaccel layer-shell-qt tesseract
```

**Debian/Ubuntu:**
```bash
sudo apt install libqt6core6 libqt6widgets6 libkf6windowsystem6 libkf6globalaccel6 tesseract-ocr
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase qt6-qtwayland kf6-kwindowsystem kf6-kglobalaccel layer-shell-qt tesseract
```

---

## GNOME-Specific Issues

### System tray icon not visible

GNOME doesn't have native system tray support. Install the AppIndicator extension:

1. Visit: https://extensions.gnome.org/extension/615/
2. Install "AppIndicator and KStatusNotifierItem Support"
3. Restart GNOME Shell (Alt+F2, type `r`, Enter) or log out/in

Without the extension, LinNote runs in "Dock Only" mode.

### Global hotkey not working (GNOME)

GNOME's Portal GlobalShortcuts is not fully implemented. Set up manually:

1. Open **Settings → Keyboard → Keyboard Shortcuts**
2. Scroll to **Custom Shortcuts**
3. Click **+** to add:
   - **Name:** LinNote Toggle
   - **Command:** `dbus-send --session --type=method_call --dest=org.linnote.LinNote /LinNote org.linnote.LinNote.Toggle`
   - **Shortcut:** Press F12 (or your preferred key)

### "Always on Top" not working (GNOME)

GNOME Wayland doesn't support programmatic always-on-top. LinNote provides "Stay Visible" mode instead, which prevents auto-hide when clicking outside but doesn't change window z-order.

---

## Runtime Issues

### Window doesn't appear

1. Check system tray - LinNote may be minimized
2. Run from terminal to see errors: `linnote`
3. Try resetting window position:
   ```bash
   rm -rf ~/.config/LinNote/
   ```

### Global hotkey not working (KDE)

1. KDE Plasma required for global shortcuts
2. Check System Settings → Shortcuts → LinNote
3. Ensure no conflicts with other apps

### OCR not working

1. Install Tesseract:
   ```bash
   # Arch
   sudo pacman -S tesseract tesseract-data-eng
   
   # Ubuntu
   sudo apt install tesseract-ocr tesseract-ocr-eng
   ```
2. Restart LinNote

### Themes not loading

1. Check Qt6 installation
2. Reset theme: Settings → Appearance → Reset

---

## Data Issues

### Notes not saving

1. Check disk space
2. Verify permissions: `~/.local/share/LinNote/`
3. Check logs: `journalctl -f | grep LinNote`

### Lost notes after update

1. Check backup folder: `~/.local/share/LinNote/backups/`
2. Notes database: `~/.local/share/LinNote/notes.db`

---

## Performance

### Slow startup

1. Disable unnecessary plugins
2. Reduce number of notes
3. Check Tesseract isn't running

### High memory usage

1. Close unused notes
2. Disable auto-paste
3. Restart application

---

## Getting Help

1. **GitHub Issues:** [Report a bug](https://github.com/sfnemis/linnote/issues)
2. **Discussions:** [Ask questions](https://github.com/sfnemis/linnote/discussions)

When reporting issues, include:
- LinNote version (`linnote --version`)
- Distribution and version
- Desktop environment
- Error messages from terminal
