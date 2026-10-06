const CACHE_NAME = "minigame-v1.2-r2";
const SCOPE_URL = new URL(self.registration.scope);

const APP_FILES = [
  "",
  "index.html",
  "styles.css",
  "app.js",
  "minigame.js",
  "minigame.wasm",
  "manifest.webmanifest"
];

const APP_SHELL = APP_FILES.map((path) => new URL(path, SCOPE_URL).href);
const ALLOWED_URLS = new Set(APP_SHELL);
const ALLOWED_PATHS = new Set(APP_SHELL.map((url) => new URL(url).pathname));

function isAllowedAppUrl(rawUrl) {
  const url = new URL(rawUrl);
  return (
    (url.protocol === "https:" ||
      (url.protocol === "http:" && ["localhost", "127.0.0.1", "[::1]"].includes(url.hostname))) &&
    url.origin === SCOPE_URL.origin &&
    ALLOWED_PATHS.has(url.pathname)
  );
}

function hasExpectedContentType(url, response) {
  const contentType = response.headers.get("content-type") || "";
  const pathname = new URL(url).pathname;

  if (pathname.endsWith(".wasm")) return contentType.includes("application/wasm");
  if (pathname.endsWith(".js")) return contentType.includes("javascript");
  if (pathname.endsWith(".css")) return contentType.includes("text/css");
  if (pathname.endsWith(".webmanifest")) {
    return contentType.includes("application/manifest+json") || contentType.includes("application/json");
  }
  return contentType.includes("text/html");
}

self.addEventListener("install", (event) => {
  event.waitUntil(
    (async () => {
      const invalidEntry = APP_SHELL.find(
        (url) => !ALLOWED_URLS.has(url) || !isAllowedAppUrl(url)
      );

      if (invalidEntry) {
        throw new Error(`Refusing to cache unexpected MiniGame asset: ${invalidEntry}`);
      }

      const cache = await caches.open(CACHE_NAME);
      // Validate every response before committing an app shell to the cache.
      const assets = await Promise.all(APP_SHELL.map(async (url) => {
        const response = await fetch(url, { cache: "reload" });
        if (response.status !== 200 || response.type !== "basic" ||
            !hasExpectedContentType(url, response)) {
          throw new Error(`Invalid MiniGame app-shell response: ${url}`);
        }
        return [url, response];
      }));
      await Promise.all(assets.map(([url, response]) => cache.put(url, response)));
      await self.skipWaiting();
    })()
  );
});

self.addEventListener("activate", (event) => {
  event.waitUntil(
    (async () => {
      const keys = await caches.keys();
      await Promise.all(
        keys.filter((key) => key.startsWith("minigame-") && key !== CACHE_NAME).map((key) => caches.delete(key))
      );
      await self.clients.claim();
    })()
  );
});

self.addEventListener("fetch", (event) => {
  if (event.request.method !== "GET" || !isAllowedAppUrl(event.request.url)) {
    return;
  }

  event.respondWith(
    (async () => {
      const cached = await caches.match(event.request);
      if (cached) {
        return cached;
      }

      const response = await fetch(event.request);
      if (
        response.status === 200 &&
        response.type === "basic" &&
        hasExpectedContentType(event.request.url, response)
      ) {
        const cache = await caches.open(CACHE_NAME);
        await cache.put(event.request, response.clone());
      }

      return response;
    })()
  );
});
