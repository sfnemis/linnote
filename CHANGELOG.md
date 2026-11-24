# Changelog

All notable changes to this project will be documented in this file.

## [2.0.1] - 2026-11-24

### Fixed

- The uninstaller asks before it runs, lists what goes and what stays, and
  keeps your notes unless you also pass `--purge`. Until now it deleted the
  notes database on the first keystroke with no confirmation and no backup.
  `--yes` is there for scripted removals, and it refuses to guess when
  there is no terminal to ask on.
- Uninstalling no longer corrupts another application's KDE global
  shortcuts. The old cleanup deleted LinNote's section and the header of
  whatever section followed it.
- The Qt SVG package is no longer required. It was added on the assumption
  that the toolbar used the SVG files in the resource bundle; it does not,
  and nothing in the code ever loaded them.
- The README shows a wget command as well, because a minimal Ubuntu or
  Debian install has no curl and so cannot run the documented one.

## [2.0.0] - 2026-11-24

### Security

- Locked notes are now encrypted with AES-256-GCM. The key comes from the
  password through PBKDF2-HMAC-SHA256 at 600000 iterations, the salt and the
  nonce come from the system random generator, and the ciphertext carries an
  authentication tag, so a wrong password or a tampered note is detected
  instead of producing plausible-looking text. Version 1.0.0 used a
  reversible scheme despite describing itself as AES.
- A note locked by 1.0.0 still opens, and is rewritten in the new format the
  first time it is unlocked. Nothing has to be exported or re-entered.
- Password hashes moved to PBKDF2 and are compared in constant time. A hash
  written by 1.0.0 keeps working and is rewritten when the password changes.
- Note contents, clipboard contents and passwords are no longer written to
  the system log, and debug output is compiled out of release builds.
- The FreeCryptoAPI key that shipped inside the application is gone. The
  field starts empty and crypto rates fall back to CoinGecko, which needs no
  key. If you had not set your own key, nothing changes for you.

### Added

- The calculator reads a comma as the decimal point. `1,5 + 2,3` answers
  `3,8`; in 1.0.0 it answered `1`. Settings, Calculator has Automatic, Dot
  and Comma, and Automatic follows the system locale. With a comma active,
  `;` separates function arguments as it does in a spreadsheet, `sum(1, 2)`
  still works, and `1.234,56` reads as one thousand two hundred thirty four
  and a half. (#7)
- Flatpak package, built on the KDE 6.11 runtime, for immutable systems. (#6)
- AppStream metadata, so the app has a proper entry in software centres.

### Fixed

- The global shortcut now binds on GNOME. The shortcuts portal refuses a
  request it cannot tie to an installed desktop file, and LinNote now puts
  itself in the systemd scope that makes that possible, whether it was
  started from the menu, a terminal or an autostart entry. (#4)
- Every icon size is installed system-wide instead of only the 128px one,
  which is why the icon looked blurry. The 512px icon was also a 1024px
  image. (#4)
- Changing a locked note's password no longer overwrites the note when the
  decryption before it failed.
- The installer now installs the Qt SQLite driver. On Debian and Ubuntu it
  is the separate package `libqt6sql6-sqlite`, and without it the app
  could not open its own database on a clean install.
- The installer refuses to install a binary built for another CPU
  architecture instead of reporting success and leaving a program that
  cannot run.
- Each published binary is now offered only on the distribution it was
  built on. 1.0.0 handed the Arch build to Fedora, RHEL and openSUSE too,
  where a different Qt or glibc can stop it from starting. Arch is pointed
  at its AUR package and anything else at the Flatpak or a source build.
- Typing `timer` switches the page to timer mode. It used to start a
  stopwatch without changing the mode, and with stopwatch auto-start
  turned off it did nothing at all.

### Changed

- Building from source now needs OpenSSL development files: `openssl-devel`
  on Fedora, `libssl-dev` on Debian and Ubuntu, `openssl` on Arch.

## [1.0.0] - 2025-01-19

### Added
- Multi-page notes with tabbed interface
- Note modes: Plain, Checklist, Code, Math, Timer, Markdown
- Smart calculations with `=` suffix
- Currency conversion with real-time rates
- Unit conversion (length, weight, temperature, volume, area)
- Pomodoro timer, stopwatch, countdown
- OCR screen capture via Tesseract
- Markdown and code syntax highlighting
- Note encryption with master password
- Recovery key system
- Auto-backup with configurable intervals
- Expiring notes (1h, 24h, 7d)
- 14 color themes (Catppuccin, Dracula, Nord, etc.)
- Global hotkey support (KDE)
- Wayland native support
- Export to TXT, Markdown, PDF, CSV, ZIP
- URL shortening
- Slash commands (/list, /code, /calc, /timer, /markdown, /ocr)
- Check for updates

### Platform
- KDE Plasma 6+ recommended
- Qt6 based
- Works on X11 and Wayland
