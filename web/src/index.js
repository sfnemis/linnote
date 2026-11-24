/**
 * linnote.app — Cloudflare Worker.
 *
 * Serves the static site from the bound assets (web/public only, never the
 * repository root) and answers /api/stats with the release numbers so the
 * page never calls the GitHub API from the visitor's browser.
 */

const REPO = "sfnemis/linnote";
const UA = "linnote-site-worker";
const STATS_TTL = 600; // seconds

async function gh(path, ctx) {
  const res = await fetch(`https://api.github.com/repos/${REPO}${path}`, {
    headers: { "user-agent": UA, accept: "application/vnd.github+json" },
    cf: { cacheTtl: STATS_TTL, cacheEverything: true },
  });
  if (!res.ok) throw new Error(`github ${path} -> ${res.status}`);
  return res.json();
}

async function buildStats() {
  const [releases, repo] = await Promise.all([
    gh("/releases?per_page=100"),
    gh(""),
  ]);

  let downloads = 0;
  for (const rel of releases) {
    for (const asset of rel.assets || []) {
      downloads += asset.download_count || 0;
    }
  }

  const latest = releases.find((r) => !r.draft && !r.prerelease) || releases[0];
  const version = latest ? String(latest.tag_name || "").replace(/^v/, "") : null;

  return {
    version,
    downloads,
    stars: repo.stargazers_count ?? null,
    releases: releases.length,
    updated: new Date().toISOString(),
  };
}

async function statsResponse(request, ctx) {
  // One cache layer only, on the GitHub fetch in gh(), which Cloudflare
  // expires on its own. A second layer here, caches.default keyed on
  // /api/stats, kept serving the same payload long after its s-maxage had
  // passed, so the version chip on the page still read 2.0.1 hours after
  // 2.0.2 was released and nothing short of a purge could move it.
  let body, status;
  try {
    body = await buildStats();
    status = 200;
  } catch (err) {
    // Never fail the page over a stats call: answer with nulls and a short TTL
    // so the chips simply keep their dash.
    body = { version: null, downloads: null, stars: null, error: "upstream" };
    status = 200;
  }

  const res = new Response(JSON.stringify(body), {
    status,
    headers: {
      "content-type": "application/json; charset=utf-8",
      "cache-control": `public, max-age=60, s-maxage=${body.error ? 60 : STATS_TTL}`,
      "access-control-allow-origin": "https://linnote.app",
    },
  });
  return res;
}

const SECURITY_HEADERS = {
  "x-content-type-options": "nosniff",
  "referrer-policy": "strict-origin-when-cross-origin",
  "x-frame-options": "SAMEORIGIN",
  "strict-transport-security": "max-age=31536000; includeSubDomains",
};

export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);

    if (url.pathname === "/api/stats") {
      return statsResponse(request, ctx);
    }

    // Everything else comes from the bound asset directory.
    const res = await env.ASSETS.fetch(request);
    const out = new Response(res.body, res);
    for (const [k, v] of Object.entries(SECURITY_HEADERS)) out.headers.set(k, v);
    return out;
  },
};
