# Keyboard Shortcuts

## General

| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New note |
| `Ctrl+S` | Save note |
| `Ctrl+W` | Close note |
| `Ctrl+Q` | Quit application |
| `Ctrl+,` | Open settings |
| `Escape` | Hide window / Cancel |

## Navigation

| Shortcut | Action |
|----------|--------|
| `Ctrl+Tab` | Next note |
| `Ctrl+Shift+Tab` | Previous note |
| `Ctrl+1-9` | Jump to note 1-9 |
| `Ctrl+Home` | First note |
| `Ctrl+End` | Last note |

## Editing

| Shortcut | Action |
|----------|--------|
| `Ctrl+Z` | Undo |
| `Ctrl+Shift+Z` | Redo |
| `Ctrl+X` | Cut |
| `Ctrl+C` | Copy |
| `Ctrl+V` | Paste |
| `Ctrl+A` | Select all |
| `Ctrl+F` | Find |
| `Ctrl+H` | Find and replace |

## Modes

| Shortcut | Action |
|----------|--------|
| `Ctrl+Alt+T` | Plain text mode |
| `Ctrl+Alt+L` | Checklist mode |
| `Ctrl+Alt+C` | Code mode |
| `Ctrl+Alt+M` | Math mode |
| `Ctrl+Alt+I` | Timer mode |
| `Ctrl+Alt+D` | Markdown mode |

## Notes

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+N` | New note |
| `Ctrl+Shift+D` | Duplicate note |
| `Ctrl+Shift+Delete` | Delete note |
| `Ctrl+Shift+E` | Export note |
| `Ctrl+Shift+P` | Pin/unpin note |

## Timer

| Shortcut | Action |
|----------|--------|
| `Space` | Pause/Resume |
| `R` | Reset timer |
| `Escape` | Stop timer |

## Global

| Shortcut | Action |
|----------|--------|
| `F12` | Show/Hide LinNote (configurable) |

> **Note:** Global hotkey can be configured in Settings → Shortcuts

> ⚠️ **IMPORTANT:** After changing any keyboard shortcut, you must **restart LinNote** for the changes to take effect.

### GNOME Users: Manual Shortcut Setup

GNOME doesn't support automatic global shortcut registration. You must add it manually:

1. Open **Settings → Keyboard → Keyboard Shortcuts**
2. Scroll to **Custom Shortcuts** at the bottom
3. Click **+** to add a new shortcut:
   - **Name:** LinNote Toggle
   - **Command:** 
     ```
     dbus-send --session --type=method_call --dest=org.linnote.LinNote /LinNote org.linnote.LinNote.Toggle
     ```
   - **Shortcut:** Press `F12` (or your preferred key)

After adding, press the shortcut to show/hide LinNote from anywhere.
