# Note Modes

LinNote has 6 specialized note modes. Switch between them by typing the keyword and pressing Enter.

---

## 📝 Plain Text

The default mode for general notes.

**Activate:**
```
plain
```

### Features
- Clean, distraction-free writing
- Auto-save
- Word and character count

### Usage Example

```
Meeting Notes - January 20

Attendees: John, Sarah, Mike

Key Points:
- Q4 targets exceeded by 15%
- New product launch in March
- Budget review next week

Action Items:
- John: Prepare sales report
- Sarah: Contact marketing
- Mike: Update timeline
```

---

## ✅ Checklist

Interactive todo lists with checkboxes.

**Activate:**
```
list
```

### Syntax
```
[ ] Unchecked item
[x] Checked item
```

### Features
- Click to toggle checkboxes
- Progress percentage shown
- Filter completed items

### Usage Example

```
[ ] Morning workout
[x] Check emails
[x] Team standup
[ ] Review PRs
[ ] Prepare presentation
[x] Lunch meeting
[ ] Code review
[ ] Update docs
```

**Progress: 3/8 (37.5%)**

---

## 💻 Code

Syntax-highlighted code editing.

**Activate:**
```
code
```

### Features
- Syntax highlighting (20+ languages)
- Line numbers
- Auto-indentation
- Bracket matching

### Supported Languages
C, C++, Python, JavaScript, TypeScript, Rust, Go, Java, Kotlin, Swift, Ruby, PHP, HTML, CSS, JSON, YAML, SQL, Bash

### Usage Example

```python
def fibonacci(n):
    a, b = 0, 1
    result = []
    while a < n:
        result.append(a)
        a, b = b, a + b
    return result

print(fibonacci(100))
```

---

## 🧮 Calculator

Live mathematical evaluation.

**Activate:**
```
calc
```

### Features
- Real-time calculation
- Variables
- Math functions
- Percentages
- Currency conversion
- Unit conversion

### Usage Example

```
budget = 500
tax_rate = 18

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
```

---

## ⏱️ Timer

Built-in productivity timers.

**Activate:**
```
timer
```

### Timer Types

- **Stopwatch:** `timer sw`
- **Countdown:** `timer 25` (25 minutes)
- **Pomodoro:** `timer pomo`
- **Breathing:** `timer breathe`

### Usage Example

```
timer 25 : deep work
```
Starts 25-minute timer named "deep work".

```
timer pomo
```
Starts Pomodoro session (25min work, 5min break).

```
timer breathe
```
Guided 4-4-4 breathing exercise.

---

## 📑 Markdown

Formatted text with live rendering.

**Activate:**
```
md
```

### Features
- Live preview
- Headings (H1-H6)
- Bold, italic
- Lists
- Code blocks
- Links
- Tables

### Usage Example

```markdown
# Project Title

## Overview
This is a **sample project** for demonstration.

## Features
- Fast performance
- Easy to use
- Fully documented

## Installation
Run the install script.

## Links
- Documentation: example.com/docs
- GitHub: github.com/example
```
