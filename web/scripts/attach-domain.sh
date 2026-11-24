#!/usr/bin/env bash
# Deploys the Worker into the Cloudflare account that owns linnote.app and
# attaches the custom domain. Run it with a token for THAT account:
#
#   CLOUDFLARE_API_TOKEN=... ./scripts/attach-domain.sh
#
# The Worker and the zone have to live in the same account, which is why
# this deploys into the zone's account rather than moving the zone.
set -euo pipefail
cd "$(dirname "$0")/.."

: "${CLOUDFLARE_API_TOKEN:?set CLOUDFLARE_API_TOKEN to a token for the account that owns linnote.app}"
API=https://api.cloudflare.com/client/v4
auth=(-H "Authorization: Bearer $CLOUDFLARE_API_TOKEN")

echo "1/5  checking the token"
curl -fsS "${auth[@]}" "$API/user/tokens/verify" >/dev/null || { echo "token rejected"; exit 1; }

echo "2/5  finding the linnote.app zone"
zone_json=$(curl -fsS "${auth[@]}" "$API/zones?name=linnote.app")
zone_id=$(printf '%s' "$zone_json" | python3 -c 'import sys,json;r=json.load(sys.stdin)["result"];print(r[0]["id"] if r else "")')
acct_id=$(printf '%s' "$zone_json" | python3 -c 'import sys,json;r=json.load(sys.stdin)["result"];print(r[0]["account"]["id"] if r else "")')
if [ -z "$zone_id" ]; then
  echo "This token cannot see the linnote.app zone either. It needs to be a"
  echo "token for the account the domain actually sits in."
  exit 1
fi
echo "     zone=$zone_id account=$acct_id"

echo "3/5  recording what DNS looks like now, so the rollback is exact"
curl -fsS "${auth[@]}" "$API/zones/$zone_id/dns_records?per_page=100" \
  | python3 -m json.tool > "dns-before-$(date +%Y%m%d-%H%M%S).json"
ls -1 dns-before-*.json | tail -1 | sed 's/^/     saved /'

echo "4/5  deploying the Worker into that account with the routes attached"
python3 - <<'PY'
import re
p='wrangler.jsonc'; s=open(p).read()
if '"routes"' not in s:
    s=s.replace('"workers_dev": true', '''"workers_dev": true,

  "routes": [
    { "pattern": "linnote.app", "custom_domain": true },
    { "pattern": "www.linnote.app", "custom_domain": true }
  ]''',1)
    open(p,'w').write(s)
    print("     routes added to wrangler.jsonc")
else:
    print("     routes already present")
PY
CLOUDFLARE_ACCOUNT_ID="$acct_id" npx --yes wrangler deploy

echo "5/5  probing the live domain"
sleep 10
./scripts/check-public.sh https://linnote.app

cat <<'NOTE'

Done. linnote.app now answers from the Worker.

To roll back: delete the linnote.app and www.linnote.app custom domains from
the Worker in the Cloudflare dashboard, then restore the GitHub Pages records
from the dns-before-*.json saved in step 3. GitHub Pages is untouched and
still builds from docs/ on the main branch, so it starts serving again as
soon as the records point back at it.
NOTE
