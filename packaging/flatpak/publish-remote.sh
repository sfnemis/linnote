#!/usr/bin/env bash
# Rebuild the signed Flatpak repository that linnote.app serves.
#
# Run this on a Linux machine with flatpak, flatpak-builder and the KDE
# 6.11 runtime, NOT on the Mac: signing needs ostree, and ostree needs
# Linux. The script leaves the finished repository in ./repo, and the Mac
# then copies it into web/public/flatpak and deploys the Worker.
#
# Usage:  publish-remote.sh <gpg-homedir> <key-fingerprint> [work-dir]
#
# The signing key lives outside every repository. Its only backup is
# ~/.linnote/flatpak-signing-secret.asc on Sait's Mac, chmod 600. Import it
# with:
#   gpg --homedir <gpg-homedir> --import ~/.linnote/flatpak-signing-secret.asc
#
# Afterwards, on the Mac:
#   rsync -a --delete <work-dir>/repo/ web/public/flatpak/
#   rm -rf web/public/flatpak/tmp
#   cd web && CLOUDFLARE_ACCOUNT_ID=0c140265fee9d14ccf1998f36fbbc883 \
#     CLOUDFLARE_API_TOKEN=$(cat ~/.cloudflare/linnote-token) npx wrangler deploy
#   bash web/scripts/check-public.sh https://linnote.app <ip>

set -euo pipefail

GPG_HOME=${1:?usage: publish-remote.sh <gpg-homedir> <key-fingerprint> [work-dir]}
FPR=${2:?missing key fingerprint}
WORK=${3:-$HOME/linnote-flatpak}

MANIFEST_SRC=$(cd "$(dirname "$0")" && pwd)/app.linnote.LinNote.yml

command -v flatpak-builder >/dev/null || { echo "flatpak-builder is missing"; exit 1; }
gpg --homedir "$GPG_HOME" --list-secret-keys "$FPR" >/dev/null

mkdir -p "$WORK"
cp "$MANIFEST_SRC" "$WORK/"
cd "$WORK"

# The manifest pins the app to a commit. Point it at the release you are
# publishing before running this, or the repository ships an older build.
echo "building from:"
grep -A3 'url: https://github.com/sfnemis/linnote.git' app.linnote.LinNote.yml

rm -rf build repo
flatpak-builder --force-clean --repo=repo --default-branch=stable \
  build app.linnote.LinNote.yml

# The .Debug extension is seven times the size of the app and nobody
# installs it from here, so it does not get published.
if ostree --repo=repo refs | grep -q '\.Debug/'; then
  ostree --repo=repo refs --delete "$(ostree --repo=repo refs | grep '\.Debug/')"
fi

flatpak build-sign  repo --gpg-sign="$FPR" --gpg-homedir="$GPG_HOME"
flatpak build-update-repo repo --prune --prune-depth=0 \
  --gpg-sign="$FPR" --gpg-homedir="$GPG_HOME"

# The scratch directory serves nothing. The empty directories stay:
# deleting refs/remotes makes every later `ostree refs` fail with
# "opendir(refs/remotes): No such file or directory", and HTTP cannot serve
# a directory anyway, so they cost a client nothing.
rm -rf repo/tmp

test -f repo/summary.sig || { echo "the summary is not signed"; exit 1; }

echo
echo "repository ready: $WORK/repo"
ostree --repo=repo refs
du -sh repo
echo "files: $(find repo -type f | wc -l)"
