# Note Modes

LinNote has 6 specialized note modes. Switch between them using slash commands or keywords.

---

## 📝 Plain Text

The default mode for general notes and quick writing.

**Switch to this mode:**
```
plain
```
or
```
/text
```

### Features
- Clean, distraction-free writing
- Auto-save
- Word and character count in status bar

### Usage Scenario

```
Meeting Notes - January 20

Attendees: John, Sarah, Mike

Key Points:
- Q4 targets exceeded by 15%
- New product launch scheduled for March
- Budget review next week

Action Items:
- John: Prepare sales report
- Sarah: Contact marketing team
- Mike: Update project timeline
```

---

## ✅ Checklist

Interactive todo lists with checkboxes.

**Switch to this mode:**
```
list
```
or
```
/checklist
```

### Syntax
```
[ ] Unchecked item
[x] Checked item
```

### Features
- Click checkbox to toggle
- Progress percentage in status bar
- Filter completed/pending items

### Usage Scenario

```
[ ] Morning workout
[x] Check emails
[x] Team standup meeting
[ ] Review pull requests
[ ] Prepare presentation
[x] Lunch with client
[ ] Code review
[ ] Update documentation
```

**Progress: 3/8 (37.5%)**

---

## 💻 Code

Syntax-highlighted code editing with line numbers.

**Switch to this mode:**
```
code
```
or
```
/code
```

### Features
- Syntax highlighting for 20+ languages
- Line numbers
- Auto-indentation
- Bracket matching
- Tab size: 4 spaces

### Supported Languages
C, C++, Python, JavaScript, TypeScript, Rust, Go, Java, Kotlin, Swift, Ruby, PHP, HTML, CSS, JSON, YAML, XML, SQL, Bash, Markdown

### Usage Scenario

```python
def fibonacci(n):
    """Generate Fibonacci sequence up to n"""
    a, b = 0, 1
    result = []
    while a < n:
        result.append(a)
        a, b = b, a + b
    return result

# Usage
print(fibonacci(100))
# Output: [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89]
```

---

## 🧮 Calculator (Math)

Live mathematical evaluation with variables and functions.

**Switch to this mode:**
```
calc
```
or
```
/calc
```

### Features
- Real-time calculation
- Variables
- Mathematical functions
- Percentages
- Currency conversion
- Unit conversion

### Usage Scenario

```
Shopping Budget Calculation
===========================

budget = 500
tax_rate = 18

Item Prices:
laptop = 350
mouse = 25
keyboard = 45

subtotal = laptop + mouse + keyboard =
// Result: 420

tax = subtotal * tax_rate / 100 =
// Result: 75.6

total = subtotal + tax =
// Result: 495.6

remaining = budget - total =
// Result: 4.4

What is 20% of budget =
// Result: 100

Convert to Euro:
495.6 USD to EUR =
// Result: ~458 EUR
```

---

## ⏱️ Timer

Built-in productivity timers.

**Switch to this mode:**
```
timer
```
or
```
/timer
```

### Timer Types

1. **Stopwatch** - Elapsed time counter
2. **Countdown** - Timer that counts to zero
3. **Pomodoro** - Work/break intervals
4. **Breathing** - Guided breathing exercise

### Usage Scenarios

**Stopwatch:**
```
timer sw
```
Starts counting up. Perfect for tracking how long a task takes.

**Countdown:**
```
timer 25
```
25 minute countdown. Great for meetings or focused work.

```
timer 5:30 : pasta
```
5 minutes 30 seconds timer named "pasta".

**Pomodoro:**
```
timer pomo
```
Uses your settings (default: 25min work, 5min break).

```
timer 52 17
```
Custom Pomodoro: 52 minutes work, 17 minutes break (based on research).

**Breathing:**
```
timer breathe
```
Guided 4-4-4 breathing pattern for relaxation.

---

## 📑 Markdown

Formatted text with live Markdown rendering.

**Switch to this mode:**
```
md
```
or
```
/markdown
```

### Features
- Live preview
- Headings (H1-H6)
- Bold, italic, strikethrough
- Lists (bullet and numbered)
- Code blocks
- Links
- Blockquotes
- Tables

### Usage Scenario

```markdown
# Project README

## Overview
This is a **sample project** for demonstration.

## Features
- Fast performance
- Easy to use
- *Fully documented*

## Installation

```bash
npm install my-package
```

## Usage

| Command | Description |
|---------|-------------|
| `start` | Start server |
| `build` | Build project |

> Note: Requires Node.js 18+

## Links
- [Documentation](https://example.com/docs)
- [GitHub](https://github.com/example)
```
