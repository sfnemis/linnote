# linnote.app — Cloudflare Worker

The site that answers https://linnote.app.

## Layout

    web/
      public/     everything that is published, and nothing else
      src/        the Worker: serves public/ and answers /api/stats
      scripts/    post-deploy probe

`public/` is the whole web root. The repository root is never served: an
allowlisted directory cannot leak a file nobody thought about, a denylist
always can.

## /api/stats

The page shows the current version, the total download count across every
release, and the star count. The Worker fetches those from the GitHub API
server side and caches them for ten minutes, so a visitor's browser never
calls GitHub and the unauthenticated rate limit is never the page's problem.
If GitHub is unreachable the endpoint answers with nulls and the chips keep
their dash; the page never fails over a stats call.

## Deploy

    cd web
    CLOUDFLARE_API_TOKEN=$(cat ~/.cloudflare/api-token) npx wrangler deploy
    ./scripts/check-public.sh https://linnote.app

The probe is not optional. It asserts that every public path answers 200 and
that a list of paths that must never be published answers 404. It exits
non-zero on either failure.

## Custom domain

linnote.app sits in a different Cloudflare account than the token above, so
the custom domain is not attached by this config yet. Once the zone is
reachable, put the routes back in `wrangler.jsonc` and redeploy. Until then
the Worker answers on its workers.dev hostname and GitHub Pages still serves
the live domain from `docs/`.
