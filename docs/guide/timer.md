# Timer

LinNote includes built-in timers for productivity.

## Stopwatch

Simple elapsed time counter.

**Start:**
```
/timer start
```

**Controls:**
- Click timer to pause/resume
- Reset button to restart

---

## Countdown

Timer that counts down to zero.

**Syntax:**
```
/timer 5m          → 5 minutes
/timer 1h          → 1 hour
/timer 1h30m       → 1 hour 30 minutes
/timer 90s         → 90 seconds
/timer 25m work    → Named timer
```

**When finished:**
- Desktop notification
- Optional sound alert

---

## Pomodoro

Productivity technique with work/break intervals.

**Start default (25/5):**
```
/pomodoro
```

**Custom intervals:**
```
/pomodoro 50 10    → 50 min work, 10 min break
/pomodoro 25 5     → Classic Pomodoro
```

**Cycle:**
1. Work session (25 min default)
2. Short break (5 min)
3. Repeat 4 times
4. Long break (15 min)

**Features:**
- Session counter
- Auto-start next session (optional)
- Desktop notifications

---

## Breathing Exercise

Guided breathing for relaxation.

```
/breathe
```

**Pattern:**
- Inhale: 4 seconds
- Hold: 4 seconds
- Exhale: 4 seconds
- Repeat

---

## Timer Controls

| Key | Action |
|-----|--------|
| `Space` | Pause/Resume |
| `R` | Reset |
| `Escape` | Stop and close |

## Notifications

Timers trigger desktop notifications when complete. Make sure your system allows notifications from LinNote.
