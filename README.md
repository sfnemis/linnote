# LinNote

A fast, keyboard-driven scratchpad for Linux. Built for KDE Plasma with Qt6.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![Qt](https://img.shields.io/badge/Qt-6-brightgreen.svg)

**[User Guide](docs/USER_GUIDE.md)** | **[Changelog](CHANGELOG.md)**

## Features

- Multi-page notes with tabs
- Smart calculations (type `2+2=` to evaluate)
- Currency conversion (real-time rates)
- Unit conversion (length, weight, temperature, etc.)
- Pomodoro timer, stopwatch, countdown
- OCR screen capture
- Markdown & code highlighting
- Note encryption with master password
- 14+ color themes
- Global hotkey support
- Wayland native

## Installation

### Arch Linux (AUR)

[![AUR version](https://img.shields.io/aur/version/linnote?logo=archlinux&label=AUR)](https://aur.archlinux.org/packages/linnote)

```bash
yay -S linnote
# or
paru -S linnote
```

### Universal Install (Debian, Fedora, openSUSE, etc.)

```bash
curl -fsSL https://linnote.app/install.sh | bash
```

This will:
- Detect your distro (Arch, Debian, Fedora, openSUSE)
- Install dependencies
- Download pre-built binary from GitHub Releases
- Set up desktop entry and icons

### Manual Build

```bash
git clone https://github.com/sfnemis/linnote.git
cd linnote
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Dependencies

**Arch/Manjaro:**
```bash
sudo pacman -S qt6-base qt6-declarative kf6 cmake
```

**Debian/Ubuntu:**
```bash
sudo apt install qt6-base-dev libkf6windowsystem-dev cmake build-essential
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel kf6-kwindowsystem-devel cmake gcc-c++
```

## Usage

Launch from your application menu or run `linnote` from terminal.

**Keyboard shortcuts:**
- `Ctrl+N` - New note
- `Ctrl+Tab` - Next page
- `Ctrl+F` - Find
- `Esc` - Hide window

**Keywords** (type anywhere to switch modes):
- `list` or `checklist` - Checklist mode
- `code` - Code mode
- `calc` - Calculator mode
- `timer` - Timer mode
- `md` or `markdown` - Markdown mode
- `plain` or `text` - Plain text mode
- `settings` - Open settings
- `ocr` - Capture text from screen
- `sum`, `avg`, `count` - Text analysis

## Configuration

Settings are stored in `~/.local/share/linnote/`.

## Contributing

Pull requests welcome. For major changes, open an issue first.

## Support

If you find LinNote useful, consider supporting the project:

☕ [Buy me a coffee (Shopier)](https://www.shopier.com/sfnemis/43380031)  
💜 [GitHub Sponsors](https://github.com/sponsors/sfnemis)

## License

[MIT](LICENSE)

## Author

sfnemis
