#!/usr/bin/env bash
# Hands linnote.app back to GitHub Pages. Deletes the two Worker routes and
# nothing else: the DNS records were never changed, so Pages answers again
# as soon as the routes are gone. Nothing to wait for.
set -euo pipefail
TOKEN=$(cat ~/.cloudflare/linnote-token)
ZONE=4a930a6827b27050aa2fa1c582345ba0
API=https://api.cloudflare.com/client/v4
curl -fsS -H "Authorization: Bearer $TOKEN" "$API/zones/$ZONE/workers/routes" \
 | python3 -c "
import sys,json
for r in json.load(sys.stdin)['result']:
    if r.get('script')=='linnote-site': print(r['id'], r['pattern'])" \
 | while read -r id pattern; do
     curl -fsS -X DELETE "$API/zones/$ZONE/workers/routes/$id" -H "Authorization: Bearer $TOKEN" >/dev/null
     echo "removed route $pattern"
   done
echo "linnote.app is back on GitHub Pages."
