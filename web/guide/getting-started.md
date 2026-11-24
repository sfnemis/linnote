# Getting Started

## Installation

Install LinNote with a single command:

```bash
curl -fsSL https://linnote.app/install.sh | bash
```

### Supported Distributions

- Arch Linux / Manjaro
- Debian / Ubuntu / Linux Mint / Pop!_OS
- Fedora
- openSUSE

### Flatpak

On an immutable system, or if you would rather keep LinNote in a sandbox,
add its repository instead:

```bash
flatpak install --user --from https://linnote.app/linnote.flatpakref
```

Drop `--user` to install for every account on the machine, which asks for
your password.

That single line adds the LinNote repository, installs the app, and pulls
the KDE runtime from Flathub if your machine does not have it yet. The
repository is signed and the public key comes with the file, so every build
is checked before it installs. Afterwards `flatpak update` keeps LinNote
current like anything else. LinNote is not on Flathub.

OCR does not work in the sandbox: it calls Tesseract on the host, and the
sandbox cannot see it.

### Requirements

- Qt6 runtime libraries
- KDE Frameworks 6 (KWindowSystem, KGlobalAccel)
- Tesseract (optional, for OCR)

> **GNOME Users:** For system tray support, install the [AppIndicator extension](https://extensions.gnome.org/extension/615/).

> **Note:** No build tools required - LinNote is distributed as a pre-built binary.

## Uninstallation

To completely remove LinNote and all its data:

```bash
curl -fsSL https://linnote.app/install.sh | bash -s -- --uninstall
```

Or if you have the script locally:
```bash
./install.sh --uninstall
```

This removes:
- Binary (`/usr/local/bin/linnote`)
- Desktop entry
- Icons
- Configuration and data (`~/.config/LinNote`, `~/.local/share/LinNote`)
- KDE global shortcuts

## First Launch

1. Open your application menu
2. Search for "LinNote"
3. Click to launch

Or run from terminal:
```bash
linnote
```

## Basic Usage

- **Type anywhere** - Just start typing
- **Keywords** - Type `list`, `code`, `calc` to switch modes
- **Menu** - Type `/` to open command popup
- **Save** - Notes auto-save, or press `Ctrl+S`

## Mode Switching

Type the keyword and press Enter:

| Keyword | Mode |
|---------|------|
| `plain` | Plain Text |
| `list` | Checklist |
| `code` | Code |
| `calc` | Calculator |
| `timer` | Timer |
| `md` | Markdown |

## Quick Tips

| Action | How |
|--------|-----|
| New note | `Ctrl+N` |
| Save | `Ctrl+S` |
| Search | `Ctrl+F` |
| Settings | `Ctrl+,` |
| Open menu | Type `/` |
