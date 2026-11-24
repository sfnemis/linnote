#!/usr/bin/env bash
# Rolls the site back to the previous Worker version.
#
# This used to delete the two Worker routes and let GitHub Pages answer.
# That fallback is gone: docs/ was deleted and Pages is switched off, so
# deleting the routes now leaves linnote.app with nothing behind it.
# Cloudflare keeps previous Worker versions instead, which is a better
# rollback anyway: it is instant and needs no DNS propagation.
set -euo pipefail
cd "$(dirname "$0")/.."
export CLOUDFLARE_API_TOKEN=$(cat ~/.cloudflare/linnote-token)
export CLOUDFLARE_ACCOUNT_ID=0c140265fee9d14ccf1998f36fbbc883

echo "Versions Cloudflare still holds:"
npx --yes wrangler versions list

echo
echo "Rolling back to the previous one. Pass a version id as the first"
echo "argument to pick a different one."
npx --yes wrangler rollback ${1:+"$1"}

echo
echo "Check it landed:"
echo "  ./scripts/check-public.sh https://linnote.app \$(dig +short linnote.app | head -1)"
