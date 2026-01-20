# Slash Commands & Keywords

LinNote uses **keywords** to switch modes and perform actions. Just type the keyword directly - no special prefix needed!

## How It Works

1. Type a keyword (e.g., `list`)
2. Press Enter
3. Action executes

> **Tip:** Typing `/` opens a popup menu showing all available commands. But you can also just type the keyword directly without `/`.

---

## Mode Switching

| Keyword | Action |
|---------|--------|
| `plain` | Switch to Plain Text mode |
| `text` | Switch to Plain Text mode |
| `list` | Switch to Checklist mode |
| `checklist` | Switch to Checklist mode |
| `code` | Switch to Code mode |
| `calc` | Switch to Calculator (Math) mode |
| `timer` | Switch to Timer mode |
| `md` | Switch to Markdown mode |
| `markdown` | Switch to Markdown mode |

### Example

```
list
```
Switches current note to Checklist mode.

---

## Text Analysis

Analyze numbers in your current note:

| Keyword | Action |
|---------|--------|
| `sum` | Sum all numbers in the note |
| `avg` | Calculate average |
| `count` | Count how many numbers |

### Example: Sum

```
Groceries:
Milk 4.50
Bread 2.30
Eggs 6.00

sum
```
**Result:** `Sum: 12.80`

### Example: Average

```
Test scores:
85
90
78
92

avg
```
**Result:** `Average: 86.25`

### Example: Count

```
Items: 5
Price: 10
Tax: 2

count
```
**Result:** `Count: 3`

---

## Timer Commands

| Keyword | Action |
|---------|--------|
| `timer` | Start stopwatch (if enabled) |
| `timer sw` | Start stopwatch |
| `timer 5` | 5 minute countdown |
| `timer 5m` | 5 minute countdown |
| `timer 5s` | 5 second countdown |
| `timer 5:30` | 5 min 30 sec countdown |
| `timer 10 : laundry` | Named timer |
| `timer pomo` | Pomodoro timer |
| `timer 52 17` | Custom Pomodoro (52min work, 17min break) |
| `timer breathe` | Breathing exercise |
| `timer p` | Pause/Resume |
| `timer r` | Reset |
| `timer s` | Stop |

### Example

```
timer 25 : focus session
```
Starts a 25-minute countdown named "focus session".

---

## AutoPaste

| Keyword | Action |
|---------|--------|
| `paste` | Paste clipboard items (newline separator) |
| `paste(,)` | Paste with comma separator |
| `paste(;)` | Paste with semicolon separator |
| `paste( )` | Paste with space separator |

### Example

Copy items to clipboard, then:
```
paste(, )
```
**Result:** `item1, item2, item3`

---

## Utility Commands

| Keyword | Action |
|---------|--------|
| `settings` | Open settings dialog |
| `ocr` | Capture text from screen |

---

## Custom Aliases

Create your own shortcuts in Settings → Keywords.

**Example:** Set `l` as alias for `list`

Now typing just `l` switches to Checklist mode.
