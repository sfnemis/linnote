# Timer

LinNote includes built-in timers for productivity. Access via the `timer` command.

## Stopwatch

Elapsed time counter.

### Start Stopwatch

```
timer sw
```
or
```
timer stopwatch
```

If enabled in settings, just `timer` also starts a stopwatch.

### Controls
- **Click timer** - Pause/Resume
- **R key** - Reset
- **Escape** - Stop and exit

### Usage Scenario

Track how long you spend on a task:
```
timer sw
```
Start working, pause when taking breaks. Great for time tracking.

---

## Countdown

Timer that counts down to zero.

### Formats

| Command | Duration |
|---------|----------|
| `timer 5` | 5 minutes |
| `timer 5m` | 5 minutes |
| `timer 30s` | 30 seconds |
| `timer 5:30` | 5 min 30 sec |
| `timer 90` | 90 minutes |

### Named Timers

Add a name after `:`:
```
timer 10 : laundry
timer 3 : eggs
timer 25 : focus session
```

### Usage Scenario

**Meeting reminder:**
```
timer 55 : meeting in 5 mins
```

**Cooking:**
```
timer 8 : pasta
timer 45s : wait for boil
```

---

## Pomodoro

Work/break intervals for focused productivity.

### Start Pomodoro

```
timer pomo
```

Uses your settings (default: 25 min work, 5 min break).

### Custom Pomodoro

```
timer 52 17
```
52 minutes work, 17 minutes break.

```
timer 25 5
```
Classic Pomodoro: 25 min work, 5 min break.

### Cycle
1. Work session (25 min)
2. Short break (5 min)
3. Repeat 4 times
4. Long break (15 min)

### Usage Scenario

**Deep work session:**
```
timer 52 17
```
Based on research: 52 minutes focus, 17 minutes break is optimal.

**Quick task:**
```
timer 15 3
```
15 minutes work, 3 minute break for smaller tasks.

---

## Breathing Exercise

Guided breathing for relaxation and focus.

```
timer breathe
```

### Pattern
- **Inhale:** 4 seconds
- **Hold:** 4 seconds
- **Exhale:** 4 seconds
- **Repeat**

### Usage Scenario

Before an important meeting or when feeling stressed:
```
timer breathe
```
Follow the visual guide. 5-10 cycles reduces anxiety.

---

## Timer Controls

Control active timers:

| Command | Action |
|---------|--------|
| `timer p` | Pause/Resume |
| `timer pause` | Pause/Resume |
| `timer r` | Reset |
| `timer reset` | Reset |
| `timer s` | Stop |
| `timer stop` | Stop |

### Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Space` | Pause/Resume |
| `R` | Reset |
| `Escape` | Stop and close |

---

## Notifications

When a timer completes:
- Desktop notification appears
- Optional sound alert (configure in settings)

Make sure notifications are enabled for LinNote in your system settings.

---

## Settings

Configure timer behavior in Settings → Timer:

- **Auto-start stopwatch** - Plain `timer` command starts stopwatch
- **Pomodoro work minutes** - Default work duration
- **Pomodoro break minutes** - Default break duration
- **Sound on completion** - Play sound when timer ends
- **Auto-start next session** - For Pomodoro, auto-start break after work
