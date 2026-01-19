# Slash Commands

LinNote supports two ways to execute commands:
1. **Slash commands** - Type `/` followed by the command (e.g., `/list`)
2. **Keywords** - Type just the keyword (e.g., `list`)

Both work the same way!

## Mode Switching

| Command | Keyword | Action |
|---------|---------|--------|
| `/plain` | `plain` | Plain Text mode |
| `/text` | `text` | Plain Text mode |
| `/list` | `list` | Checklist mode |
| `/checklist` | `checklist` | Checklist mode |
| `/code` | `code` | Code mode |
| `/calc` | `calc` | Calculator (Math) mode |
| `/timer` | `timer` | Timer mode |
| `/md` | `md` | Markdown mode |
| `/markdown` | `markdown` | Markdown mode |

## Text Analysis

Analyze numbers in your current note:

| Command | Keyword | Action |
|---------|---------|--------|
| `/sum` | `sum` | Sum all numbers |
| `/avg` | `avg` | Average of numbers |
| `/count` | `count` | Count numbers |

### Examples

```
I bought:
- Apples: 25
- Oranges: 30
- Bananas: 15

sum
```
**Result:** `Sum: 70`

```
Test scores:
85
90
78
92

avg
```
**Result:** `Average: 86.25`

## Timer Commands

| Command | Action |
|---------|--------|
| `timer` | Start stopwatch (if enabled in settings) |
| `timer sw` | Start stopwatch |
| `timer stopwatch` | Start stopwatch |
| `timer 5` | 5 minute countdown |
| `timer 5m` | 5 minute countdown |
| `timer 5s` | 5 second countdown |
| `timer 5:30` | 5 min 30 sec countdown |
| `timer 10 : laundry` | 10 min timer named "laundry" |
| `timer pomo` | Pomodoro (uses settings) |
| `timer 52 17` | Custom Pomodoro: 52min work, 17min break |
| `timer breathe` | Breathing exercise |
| `timer p` | Pause/Resume |
| `timer r` | Reset |
| `timer s` | Stop |

## AutoPaste

Automatically paste clipboard content:

| Command | Action |
|---------|--------|
| `paste` | Paste with newline separator |
| `paste(,)` | Paste with comma separator |
| `paste(;)` | Paste with semicolon separator |
| `paste( )` | Paste with space separator |

### Example Usage

Copy multiple items to clipboard one by one, then:
```
paste(, )
```
**Result:** `item1, item2, item3`

## Utility Commands

| Command | Keyword | Action |
|---------|---------|--------|
| `/settings` | `settings` | Open settings dialog |
| `/ocr` | `ocr` | Screen text capture |

## Custom Aliases

You can create custom aliases for keywords in Settings → Keywords.

Example: Set `l` as alias for `list` → typing `l` switches to Checklist mode.
