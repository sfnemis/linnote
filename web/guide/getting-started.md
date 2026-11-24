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
flatpak remote-add --user --if-not-exists --from linnote https://linnote.app/linnote.flatpakrepo
flatpak install --user linnote app.linnote.LinNote
```

Drop `--user` from both lines to install for every account on the machine,
which asks for your password. The repository is signed and the public key
comes with the `.flatpakrepo` file, so `flatpak update` checks every build
before it installs it. LinNote is not on Flathub.

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
