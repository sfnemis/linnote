# Troubleshooting

Common issues and solutions.

## Installation Issues

### "Command not found" after install

The binary may not be in your PATH. Try:

```bash
# Add to path
export PATH="$HOME/.local/bin:$PATH"

# Or run directly
~/.local/bin/linnote
```

### Dependencies not found

Install required packages manually:

**Arch Linux:**
```bash
sudo pacman -S qt6-base kf6-kglobalaccel tesseract
```

**Debian/Ubuntu:**
```bash
sudo apt install qt6-base-dev libkf6globalaccel-dev tesseract-ocr
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel kf6-kglobalaccel-devel tesseract
```

---

## Runtime Issues

### Window doesn't appear

1. Check system tray - LinNote may be minimized
2. Run from terminal to see errors: `linnote`
3. Try resetting window position:
   ```bash
   rm ~/.config/linnote/geometry.conf
   ```

### Global hotkey not working

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
2. Verify permissions: `~/.local/share/linnote/`
3. Check logs: `journalctl -f | grep linnote`

### Lost notes after update

1. Check backup folder: `~/.local/share/linnote/backups/`
2. Notes database: `~/.local/share/linnote/notes.db`

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
