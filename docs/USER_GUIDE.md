# LinNote User Guide

Complete documentation for all LinNote features and modes.

## Table of Contents

- [Getting Started](#getting-started)
- [Note Modes](#note-modes)
  - [Plain Text](#plain-text-mode)
  - [Checklist](#checklist-mode)
  - [Code](#code-mode)
  - [Math/Calculator](#math-mode)
  - [Timer](#timer-mode)
  - [Markdown](#markdown-mode)
- [Slash Commands](#slash-commands)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Calculator Features](#calculator-features)
- [Currency Conversion](#currency-conversion)
- [Unit Conversion](#unit-conversion)
- [Timer Features](#timer-features)
- [Security](#security)
- [Settings](#settings)
- [Troubleshooting](#troubleshooting)

---

## Getting Started

### First Launch

When you first launch LinNote, you'll see a welcome wizard that helps you configure:
- Global hotkey for quick access
- Theme preferences (14+ themes available)
- Basic settings

### Creating Notes

- Press `Ctrl+N` to create a new note
- Use `Ctrl+Tab` / `Ctrl+Shift+Tab` to navigate between notes
- Notes are automatically saved

### Quick Mode Switching

Type a slash command at the beginning of a line to switch modes:

```
/list    → Checklist mode
/code    → Code mode
/calc    → Calculator mode
/timer   → Timer mode
/markdown → Markdown mode
```

---

## Note Modes

### Plain Text Mode

The default mode for simple note-taking.

**Features:**
- Unlimited text entry
- Auto-save
- Search & replace (`Ctrl+F`, `Ctrl+H`)

**Usage:**
Just start typing. No special syntax required.

---

### Checklist Mode

Turn your notes into interactive todo lists.

**Activation:**
- Type `/list` or `/checklist`
- Or select "Checklist" from the mode dropdown

**Creating Items:**
```
[ ] Unchecked item
[x] Checked item
```

**Quick Completion:**
Type `/x` at the end of any line to toggle its checkbox.

**Example:**
```
[ ] Buy groceries
[x] Send email
[ ] Call dentist
```

**Tips:**
- Press Enter to create a new checkbox automatically
- Checkboxes are clickable with mouse
- Checked items can be styled differently (strikethrough)

---

### Code Mode

Syntax highlighting for 20+ programming languages.

**Activation:**
- Type `/code`
- Or select "Code" from the mode dropdown

**Supported Languages:**
- JavaScript, TypeScript
- Python
- C, C++, C#
- Java, Kotlin
- Go, Rust
- Ruby, PHP
- HTML, CSS
- SQL
- Shell/Bash
- JSON, YAML, XML
- And more...

**Language Detection:**
LinNote automatically detects the language based on:
1. File extension in the first line (e.g., `// file.js`)
2. Shebang (e.g., `#!/bin/bash`)
3. Syntax patterns

**Manual Language Selection:**
Add a comment with the language name:
```javascript
// javascript
function hello() {
    console.log("Hello, World!");
}
```

---

### Math Mode

Powerful calculator with live evaluation.

**Activation:**
- Type `/calc` or `/math`
- Or select "Math" from the mode dropdown

**Basic Operations:**
```
2 + 3 =          → 5
10 - 4 =         → 6
5 * 6 =          → 30
20 / 4 =         → 5
2 ** 3 =         → 8 (power)
10 % 3 =         → 1 (modulo)
```

**Important:** Add `=` at the end of the expression to evaluate.

**Variables:**
```
x = 10
y = 20
x + y =          → 30
```

**Mathematical Functions:**
```
sqrt(16) =       → 4
sin(0) =         → 0
cos(0) =         → 1
tan(45) =        → 1
log(100) =       → 2
ln(2.718) =      → ~1
abs(-5) =        → 5
round(3.7) =     → 4
floor(3.9) =     → 3
ceil(3.1) =      → 4
```

**Constants:**
```
pi =             → 3.14159...
e =              → 2.71828...
```

**Percentage Calculations:**
```
200 + 10% =      → 220
500 - 20% =      → 400
50% of 80 =      → 40
```

**Text Analysis:**
```
sum               → Sum of all numbers in the note
avg               → Average of all numbers
count             → Count of numbers
min               → Minimum value
max               → Maximum value
```

**Example Session:**
```
Shopping Budget

Groceries: 150
Utilities: 80
Transport: 45
Entertainment: 60

sum =            → 335
avg =            → 83.75
```

---

### Timer Mode

Built-in productivity timers.

**Activation:**
- Type `/timer`
- Or select "Timer" from the mode dropdown

**Timer Types:**

#### Stopwatch
```
/timer stopwatch
```
Counts up from 00:00:00.

Controls:
- Start/Pause button
- Reset button
- Lap functionality

#### Countdown
```
/timer countdown 10m
/timer countdown 1h30m
/timer countdown 90s
```

Time formats:
- `10s` - 10 seconds
- `5m` - 5 minutes
- `2h` - 2 hours
- `1h30m` - 1 hour 30 minutes

#### Pomodoro
```
/timer pomodoro
/timer pomodoro 25 5
```

Default: 25 minutes work, 5 minutes break.
Custom: `/timer pomodoro [work_minutes] [break_minutes]`

**Timer Controls:**
- `/timer pause` or `/timer p` - Pause
- `/timer resume` - Resume
- `/timer reset` or `/timer r` - Reset
- `/timer stop` or `/timer s` - Stop

**Sound Notifications:**
Configurable in Settings → Timer.

---

### Markdown Mode

Full markdown support with live preview.

**Activation:**
- Type `/markdown` or `/md`
- Or select "Markdown" from the mode dropdown

**Supported Syntax:**

#### Headers
```markdown
# Heading 1
## Heading 2
### Heading 3
```

#### Text Formatting
```markdown
**bold text**
*italic text*
~~strikethrough~~
`inline code`
```

#### Lists
```markdown
- Unordered item
- Another item

1. Ordered item
2. Another item
```

#### Links & Images
```markdown
[Link text](https://example.com)
![Alt text](image.png)
```

#### Code Blocks
````markdown
```python
def hello():
    print("Hello!")
```
````

#### Blockquotes
```markdown
> This is a quote
```

#### Tables
```markdown
| Column 1 | Column 2 |
|----------|----------|
| Cell 1   | Cell 2   |
```

#### Horizontal Rule
```markdown
---
```

**Live Preview:**
Enable in Settings → Editor → "Markdown live preview"

---

## Slash Commands

All available slash commands:

| Command | Aliases | Description |
|---------|---------|-------------|
| `/list` | `/checklist` | Switch to checklist mode |
| `/code` | - | Switch to code mode |
| `/calc` | `/math` | Switch to calculator mode |
| `/timer` | - | Switch to timer mode |
| `/markdown` | `/md` | Switch to markdown mode |
| `/plain` | `/text` | Switch to plain text mode |
| `/ocr` | - | Capture text from screen |
| `/settings` | - | Open settings dialog |
| `/x` | - | Toggle checkbox (end of line) |

**Timer Commands:**
| Command | Description |
|---------|-------------|
| `/timer stopwatch` | Start stopwatch |
| `/timer countdown 10m` | Start 10-minute countdown |
| `/timer pomodoro` | Start Pomodoro timer |
| `/timer pause` | Pause timer |
| `/timer reset` | Reset timer |

---

## Keyboard Shortcuts

### Global
| Shortcut | Action |
|----------|--------|
| `Ctrl+Super+N` | Toggle LinNote window (configurable) |

### Notes
| Shortcut | Action |
|----------|--------|
| `Ctrl+N` | New note |
| `Ctrl+D` | Delete current note |
| `Ctrl+Tab` | Next note |
| `Ctrl+Shift+Tab` | Previous note |
| `Ctrl+Home` | First note |
| `Ctrl+End` | Last note |
| `Ctrl+1` to `Ctrl+9` | Jump to note 1-9 |

### Editing
| Shortcut | Action |
|----------|--------|
| `Ctrl+F` | Find |
| `Ctrl+H` | Find & Replace |
| `Ctrl+L` | Lock/Unlock note |
| `Esc` | Hide window |

### Tools
| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+O` | OCR screen capture |
| `Ctrl+Shift+L` | Shorten URL |
| `Ctrl+E` | Export note |
| `Ctrl+T` | Toggle always on top |

---

## Calculator Features

### Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `5 + 3 = 8` |
| `-` | Subtraction | `10 - 4 = 6` |
| `*` | Multiplication | `6 * 7 = 42` |
| `/` | Division | `20 / 4 = 5` |
| `**` | Power | `2 ** 3 = 8` |
| `%` | Modulo | `10 % 3 = 1` |

### Functions

| Function | Description |
|----------|-------------|
| `sqrt(x)` | Square root |
| `abs(x)` | Absolute value |
| `sin(x)` | Sine (degrees) |
| `cos(x)` | Cosine (degrees) |
| `tan(x)` | Tangent (degrees) |
| `log(x)` | Base-10 logarithm |
| `ln(x)` | Natural logarithm |
| `round(x)` | Round to nearest integer |
| `floor(x)` | Round down |
| `ceil(x)` | Round up |
| `min(a,b)` | Minimum of two values |
| `max(a,b)` | Maximum of two values |

---

## Currency Conversion

Convert between 30+ currencies with real-time rates.

### Usage

```
100 USD to EUR =
50 EUR to TRY =
1000 TRY to USD =
```

### Supported Currencies

**Fiat:**
USD, EUR, GBP, JPY, TRY, CAD, AUD, CHF, CNY, NZD, HKD, INR, KRW, BRL, MXN, PLN, SEK, NOK, DKK, SGD, ZAR, THB, AED, SAR, RUB, ALL

**Crypto:**
BTC, ETH, USDT, USDC, XRP, BNB, SOL, ADA, DOGE, TRX

### Configuration

Settings → Calculator:
- Base currency selection
- API provider selection
- Refresh interval

---

## Unit Conversion

Convert between various measurement units.

### Usage

```
100 km to miles =
32 fahrenheit to celsius =
5 kg to pounds =
1 gallon to liters =
```

### Supported Categories

| Category | Units |
|----------|-------|
| Length | km, m, cm, mm, miles, yards, feet, inches |
| Weight | kg, g, mg, pounds, ounces, tons |
| Temperature | celsius, fahrenheit, kelvin |
| Volume | liters, ml, gallons, quarts, pints, cups, fl oz |
| Area | km², m², hectares, acres, sq ft |
| Speed | km/h, mph, m/s, knots |
| Time | hours, minutes, seconds, days, weeks |
| Data | bytes, KB, MB, GB, TB |

---

## Timer Features

### Stopwatch

Counts up from zero. Useful for tracking time spent on tasks.

**Controls:**
- Play/Pause button
- Reset button
- Lap button (records split times)

### Countdown

Set a specific duration and count down to zero.

**Quick Examples:**
```
/timer countdown 5m      → 5 minutes
/timer countdown 1h      → 1 hour
/timer countdown 30s     → 30 seconds
/timer countdown 1h30m   → 1 hour 30 minutes
```

### Pomodoro

Productivity technique: work intervals followed by breaks.

**Default:** 25 min work / 5 min break

**Custom:**
```
/timer pomodoro 45 10    → 45 min work / 10 min break
```

**Features:**
- Automatic work/break switching
- Sound notifications
- Session counter

---

## Security

### Master Password

Protect sensitive notes with encryption.

**Setting Up:**
1. Settings → Security
2. Click "Set Master Password"
3. Enter and confirm your password
4. Save the recovery key!

**Locking Notes:**
- Press `Ctrl+L` on any note
- Or right-click → Lock Note

**Unlocking:**
Enter your master password when prompted.

### Recovery Key

If you forget your password:
1. Settings → Security
2. Click "Use Recovery Key"
3. Enter your recovery key
4. Set a new password

**Important:** Store your recovery key in a safe place!

### Auto-Lock

Configure automatic locking:
- Never
- Until Restart
- 1/5/15/30 minutes

---

## Settings

Access via Settings menu or `/settings` command.

### Visual
- Color scheme (14 themes)
- Dark/Light mode
- Transparency
- Display mode (Tray/Dock/Both)
- Start on boot

### Editor
- Font family and size
- Default note mode
- Note title format
- Auto-paste behavior
- Paste cleanup options

### Backup
- Enable automatic backups
- Backup interval
- Retention count
- Manual backup

### Security
- Master password
- Recovery key
- Auto-lock timer

### Shortcuts
- Global hotkey
- All keyboard shortcuts
- Reset to defaults

### Calculator
- Math evaluation
- Currency conversion
- Base currency
- API provider
- Refresh interval

### Timer
- Pomodoro durations
- Sound settings
- Auto-start options

---

## Troubleshooting

### LinNote won't start

1. Check if another instance is running
2. Try running from terminal: `linnote`
3. Check dependencies are installed

### Global hotkey not working

**KDE Plasma:**
1. System Settings → Shortcuts
2. Look for LinNote entry
3. Reconfigure if needed

**GNOME:**
Global hotkeys require additional setup. See Settings for alternatives.

### Currency rates not updating

1. Check internet connection
2. Settings → Calculator → Test API
3. Try different provider

### Notes not saving

1. Check disk space
2. Check: `~/.local/share/linnote/`
3. Verify SQLite database isn't corrupted

### OCR not working

1. Install Tesseract: `sudo pacman -S tesseract tesseract-data-eng`
2. Check Settings → OCR → Language

---

## Tips & Tricks

1. **Quick math anywhere**: Type `2+2=` in any mode for instant calculation
2. **URL shortening**: Select a URL and press `Ctrl+Shift+L`
3. **Quick mode switching**: `/list`, `/code`, `/calc` work instantly
4. **Pin important notes**: Right-click → Pin Note
5. **Export multiple formats**: `Ctrl+E` → Choose TXT, MD, PDF, CSV, or ZIP
6. **Search all notes**: Use the search bar in the toolbar
7. **Transparency**: Adjust in Settings for overlay-style usage

---

## Getting Help

- GitHub Issues: https://github.com/sfnemis/linnote/issues
- Documentation: https://github.com/sfnemis/linnote/wiki

---

*LinNote - Fast notes for Linux*
