# LinNote

A fast, keyboard-driven scratchpad for Linux. Built for KDE Plasma with Qt6.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux-green.svg)
![Qt](https://img.shields.io/badge/Qt-6-brightgreen.svg)

**[User Guide](docs/USER_GUIDE.md)** | **[Changelog](CHANGELOG.md)**

## Features

- Multi-page notes with tabs
- Smart calculations (type `2+2=` to evaluate), with comma or dot as the decimal separator
- Currency conversion (real-time rates)
- Unit conversion (length, weight, temperature, etc.)
- Pomodoro timer, stopwatch, countdown
- OCR screen capture
- Markdown & code highlighting
- Note encryption with a master password (AES-256-GCM)
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

### Flatpak

LinNote has its own Flatpak repository, so updates arrive the normal way:

```bash
flatpak remote-add --user --if-not-exists --from linnote https://linnote.app/linnote.flatpakrepo
flatpak install --user linnote app.linnote.LinNote
```

Drop `--user` from both lines for a system-wide install, which asks for your
password.

The repository is signed, and the public key travels inside the
`.flatpakrepo` file, so `flatpak update` verifies every build. It is not on
Flathub. A single bundle is still attached to each release if you would
rather not add a repository:

```bash
flatpak install linnote-2.0.1.flatpak
```

OCR does not work inside the flatpak, tesseract is not in the sandbox.

### Universal Install (Debian, Fedora, openSUSE, etc.)

```bash
curl -fsSL https://linnote.app/install.sh | bash
```

A minimal Ubuntu or Debian install has no curl, so use wget there:

```bash
wget -qO- https://linnote.app/install.sh | bash
```

The script itself is `web/public/install.sh`, which is the file linnote.app
serves. There is one copy on purpose: when it lived in two places they drifted.

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
sudo pacman -S qt6-base kf6 cmake openssl tesseract
```

**Debian/Ubuntu:**
```bash
sudo apt install qt6-base-dev libqt6sql6-sqlite \
  libkf6windowsystem-dev libssl-dev cmake build-essential tesseract-ocr
```

**Fedora:**
```bash
sudo dnf install qt6-qtbase-devel \
  kf6-kwindowsystem-devel openssl-devel cmake gcc-c++ tesseract
```

## Tested on

Before 2.0.1 went out I ran it on three machines. These are the exact
systems:

| System | Qt | OpenSSL | Desktop |
|---|---|---|---|
| Fedora Workstation 45 | 6.11.2 | 4.0.3 | GNOME 51 |
| Ubuntu 26.04.1 LTS | 6.10.2 | 3.5.5 | GNOME |
| EndeavourOS (Arch, kernel 7.3.5) | 6.12.0 | 4.1.0 | KDE Plasma 6.8.3 |

All three x86_64. What I checked on each of them, not just on one:

- A clean build from source with no compiler warnings, and all seven test
  suites passing, including 23 tests on the encryption and 39 on the
  calculator.
- A note locked by LinNote 1.0.0 opened with its old password and came back
  with exactly the text that went in, and the database then showed it stored
  in the new format.
- Every mode keyword, calc, list, code, markdown and timer, checked by
  reading the mode back out of the database rather than by looking at the
  screen.
- The calculator with both separators: `2+2=`, `100.5*2=`, `sqrt(16)=`,
  `50 km to miles`, and with the comma on, `1,5+2,3=` and `sum(1; 2; 3,5)=`.
- `install.sh` run end to end from linnote.app, then the uninstaller through
  all four of its paths.

On Fedora and Ubuntu that was the published binary and the installer. On
EndeavourOS it was the AUR package built and installed with `makepkg -si`.
The flatpak was built and run on Fedora.

There is no arm64 build yet. Building from source on arm64 works.

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

Notes and settings are stored in `~/.local/share/LinNote/linnote/notes.db`.
Under Flatpak they are in `~/.var/app/app.linnote.LinNote/data/`.

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
