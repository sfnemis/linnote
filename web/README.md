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

## The guides

The source is `web/guide` (VitePress) and it builds straight into
`web/public/guide`, which is what the Worker serves. Edit the markdown,
run `npm run build` in `web/guide`, then deploy. The workflow in
`.github/workflows/deploy-guide.yml` does the build and commit on push.

## Domain and rollback

linnote.app lives in a different Cloudflare account than the other projects,
so this Worker is deployed there with a token scoped to that account, kept at
`~/.cloudflare/linnote-token`. The hostnames are attached with Worker routes
rather than custom domains: a custom domain hands the hostname's DNS to
Workers, and on the apex that published AAAA with no A, which no IPv4 client
could reach.

GitHub Pages is switched off and `docs/` is gone, so this Worker is the only
thing serving linnote.app. Rolling back means going to a previous Worker
version with `scripts/rollback.sh`; Cloudflare keeps them.
