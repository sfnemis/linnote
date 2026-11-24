#!/usr/bin/env bash
# Capture the screenshots the linnote.app tour section uses.
#
# Run it on a Linux machine that has Xvfb, openbox, xcompmgr, xdotool,
# ImageMagick and a LinNote build on PATH. It does NOT start the display:
# bring one up first, because the app needs a window manager for its frame
# and a compositor for its translucency.
#
#   setsid Xvfb :99 -screen 0 1680x1150x24 >/dev/null 2>&1 &
#   DISPLAY=:99 setsid openbox   >/dev/null 2>&1 &
#   DISPLAY=:99 setsid xcompmgr  >/dev/null 2>&1 &
#   DISPLAY=:99 LINNOTE_NO_SCOPE=1 setsid linnote >~/lin.log 2>&1 &
#
# Then:  tour-shots.sh pages      the five page shots and the code page
#        tour-shots.sh settings   the three settings tabs
#        tour-shots.sh wizard     the two first-run screens
#
# The wizard only appears on a first run, so delete ~/.local/share/LinNote
# before asking for it. Settings live in the `metadata` table of that
# directory's notes.db, not in ~/.config.
#
# Files land in ~/shots. Copy each one off the machine as it is taken: this
# VM gets rebooted from its console, and /tmp does not survive that, which
# is why nothing is written there.
#
# Known pitfalls, each one paid for:
#   - The main window is named after the page it shows, so `xdotool search
#     --name linnote` does not find it, and the ids it does return are not
#     the visible window. Every window is listed and matched instead.
#   - An xdotool call naming a window the app has just destroyed blocks
#     forever, so every call has a timeout.
#   - Ctrl+Home and Ctrl+End do not change page, whatever the app's own
#     shortcut list says. Ctrl+1 to Ctrl+9 do.
#   - Expressions only evaluate on the pages the app seeds on first run.
#     On a page made with Ctrl+N they never do, so these shots use the
#     seeded pages.
set -u
export DISPLAY=${DISPLAY:-:99}
OUT="$HOME/shots"
mkdir -p "$OUT"

x() { timeout 10 xdotool "$@"; }

# The only visible window that is not openbox's own and is wider than 200 px.
win() {
  local w n g best=""
  for w in $(timeout 10 xdotool search --onlyvisible "" 2>/dev/null); do
    n=$(timeout 5 xdotool getwindowname "$w" 2>/dev/null) || continue
    [ "$n" = "Openbox" ] && continue
    [ -z "$n" ] && continue
    g=$(timeout 5 xdotool getwindowgeometry --shell "$w" 2>/dev/null) || continue
    eval "$g"
    [ "${WIDTH:-0}" -gt 200 ] && best=$w
  done
  echo "$best"
}

grab() {   # grab <file> [window]
  # No window given means resolve it now: the main window is recreated while
  # the app settles, so an id read at the start of a run goes stale and
  # every later capture fails with "no window with specified ID exists".
  local W=${2:-$(win)}
  [ -z "$W" ] && { echo "no window for $1"; return 1; }
  sleep 1.2
  timeout 20 import -window "$W" "$OUT/$1" || return 1
  identify "$OUT/$1"
}

case "${1:-}" in
pages)
  W=$(win); [ -z "$W" ] && { echo "no window"; exit 1; }
  x windowsize "$W" 1200 800; sleep 1
  x windowmove "$W" 200 150; sleep 0.6
  x windowactivate "$W"; sleep 0.6

  # Page 3 is the seeded Calculator Example. Its text is replaced so the
  # results are visible, leaving out unit conversion, which answers with
  # its own input (tracked list).
  x key --clearmodifiers ctrl+3; sleep 1.2
  x key --clearmodifiers ctrl+a; sleep 0.5
  x key --clearmodifiers Delete; sleep 0.8
  for L in "calc" "Welcome to Calculator Mode!" "" "Basic math:" \
           "1250 * 18% =" "(240 + 180) / 3 =" "sqrt(144) =" "2^10 =" "" \
           "Currency conversion:" "50 USD to EUR" "100 EUR to TRY"; do
    [ -n "$L" ] && x type --clearmodifiers --delay 16 "$L"
    x key --clearmodifiers Return; sleep 1.0
  done
  sleep 2.5
  grab 01-calculator.png

  # The function pages answer when Return is pressed at the end of the
  # keyword line, which is where the caret already sits.
  for pair in "5 03-average-function.png" "6 avg.png" "7 05-word-count.png"; do
    set -- $pair
    x key --clearmodifiers "ctrl+$1"; sleep 1.3
    x key --clearmodifiers Return; sleep 2.5
    grab "$2"
  done

  for pair in "4 02-checklist.png" "8 08-code.png" "9 07-timer.png"; do
    set -- $pair
    x key --clearmodifiers "ctrl+$1"; sleep 1.5
    grab "$2"
  done
  ;;

settings)
  W=$(win)
  x windowactivate "$W"; sleep 0.5
  # The gear sits at the right end of the toolbar. Ctrl+, does not open
  # Settings, whatever the guide says.
  eval "$(timeout 5 xdotool getwindowgeometry --shell "$W")"
  x mousemove $((X + WIDTH - 21)) $((Y + 18)) click 1; sleep 3
  S=$(win)
  x windowsize "$S" 1000 760; sleep 1
  x windowmove "$S" 300 180; sleep 1
  x windowactivate "$S"; sleep 0.8
  eval "$(timeout 5 xdotool getwindowgeometry --shell "$S")"
  # Sidebar rows, 49 px apart: Visual, Editor, Backup, Security, Shortcuts,
  # Calculator, Timer, OCR, Keywords, About.
  pick() { x mousemove $((X + 69)) $((Y + $1)) click 1; sleep 1.8; grab "$2" "$S"; }
  pick 81  12-settings-editor.png
  pick 228 15-settings-shortcuts.png
  pick 473 20-settings-about.png
  ;;

wizard)
  W=$(win); [ -z "$W" ] && { echo "no window"; exit 1; }
  x windowsize "$W" 900 620; sleep 1.2
  x windowactivate "$W"; sleep 0.8
  eval "$(timeout 5 xdotool getwindowgeometry --shell "$W")"
  next() { x mousemove $((X + 840)) $((Y + 595)) click 1; sleep 2; }
  next                                   # step 1 -> 2
  grab 09-setup-preferences.png "$W"
  next                                   # step 2 -> 3
  timeout 20 import -window "$W" "$OUT/sec-raw.png"
  # The dialog will not shrink below 520 px, and the security step fills
  # only the top 200, so the empty half is cropped away.
  magick "$OUT/sec-raw.png" -crop 900x200+0+0 +repage "$OUT/10-setup-security.png"
  identify "$OUT/10-setup-security.png"
  ;;

*)
  echo "usage: tour-shots.sh pages|settings|wizard"
  exit 1
  ;;
esac
