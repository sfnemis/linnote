# Timer

LinNote includes built-in timers for productivity.

**Activate:**
```
timer
```

---

## Stopwatch

Elapsed time counter.

**Start:**
```
timer sw
```

**Controls:**
- Click timer to pause/resume
- Press R to reset
- Press Escape to stop

---

## Countdown

Timer that counts down to zero.

| Command | Duration |
|---------|----------|
| `timer 5` | 5 minutes |
| `timer 5m` | 5 minutes |
| `timer 30s` | 30 seconds |
| `timer 5:30` | 5 min 30 sec |
| `timer 90` | 90 minutes |

**Named timer:**
```
timer 10 : laundry
```
10-minute timer named "laundry".

---

## Pomodoro

Work/break intervals.

**Start with default settings:**
```
timer pomo
```

**Custom intervals:**
```
timer 52 17
```
52 minutes work, 17 minutes break.

**Cycle:**
1. Work session
2. Short break
3. Repeat 4 times
4. Long break

---

## Breathing Exercise

Guided relaxation.

```
timer breathe
```

**Pattern:**
- Inhale: 4 seconds
- Hold: 4 seconds
- Exhale: 4 seconds

---

## Timer Controls

| Command | Action |
|---------|--------|
| `timer p` | Pause/Resume |
| `timer r` | Reset |
| `timer s` | Stop |

**Keyboard:**
- Space: Pause/Resume
- R: Reset
- Escape: Stop

---

## Notifications

When countdown completes:
- Desktop notification appears
- Optional sound alert

Configure in Settings → Timer.
