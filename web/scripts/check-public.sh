#!/usr/bin/env bash
# Post-deploy probe. Private paths must 404, public ones must 200.
# Exits non-zero on either failure, so a bad deploy fails loudly.
set -u
BASE="${1:-https://linnote.app}"
fail=0

# Optional: pin the hostname to an IP, so a broken local resolver cannot be
# mistaken for a broken site. Pass the IP as the second argument.
HOST=$(printf '%s' "$BASE" | sed -E 's#^https?://([^/]+).*#\1#')
RESOLVE=()
if [ "${2:-}" != "" ]; then
  RESOLVE=(--resolve "$HOST:443:$2")
  echo "pinning $HOST to $2"
fi

want() { # want <code> <path>
  local want=$1 path=$2 got
  got=$(curl -s -o /dev/null -w '%{http_code}' --max-time 15 "${RESOLVE[@]}" "$BASE$path?cb=$$$RANDOM")
  if [ "$got" = "$want" ]; then
    printf '  ok   %-3s %s\n' "$got" "$path"
  else
    printf '  FAIL want %s got %s  %s\n' "$want" "$got" "$path"
    fail=1
  fi
}

echo "Probing $BASE"
echo "must be reachable:"
for p in / /install.sh /USER_GUIDE.md /COMPARISON.md /robots.txt /icon.png /logo.png \
         /app.css /app.js /tokens.css /guide/ /screenshots/01-calculator.png /api/stats \
         /screenshots/store/sc-01-calculator.png \
         /linnote.flatpakrepo /linnote.flatpakref \
         /flatpak/config /flatpak/summary /flatpak/summary.sig; do
  want 200 "$p"
done

echo "must NOT be reachable:"
for p in /CLAUDE.md /CLAUDE.local.md /AGENTS.md /.git/config /.gitignore /package.json \
         /README.md /CHANGELOG.md /CMakeLists.txt /main.cpp /core/Settings.cpp \
         /project-docs/feature-requests.md /project-docs/agent-log.md \
         /.serena/project.yml /.claude/settings.json /wrangler.jsonc /src/index.js \
         /guide/package.json /guide/package-lock.json /guide/.vitepress/config.mjs \
         /packaging/aur/PKGBUILD /dist/linnote-arch /.env \
         /flatpak-signing-secret.asc /linnote-signing-secret.asc \
         /flatpak/.gpg /flatpak/tmp/; do
  want 404 "$p"
done

echo "stats payload:"
curl -s --max-time 15 "${RESOLVE[@]}" "$BASE/api/stats" | head -c 300; echo
exit $fail
