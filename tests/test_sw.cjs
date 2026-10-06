const assert = require('node:assert/strict');
const fs = require('node:fs');
const vm = require('node:vm');

function worker(scope, badAsset = false) {
  const handlers = {}, deleted = [], stored = new Map();
  const context = {
    URL, Set, Promise,
    self: { registration: { scope }, addEventListener: (name, fn) => { handlers[name] = fn; },
      skipWaiting: async () => {}, clients: { claim: async () => {} } },
    caches: { open: async () => ({ put: async (url, response) => stored.set(url, response) }),
      keys: async () => ['minigame-v1.1', 'minigame-v1.2-r2', 'other-app'],
      delete: async (key) => deleted.push(key), match: async () => null },
    fetch: async (request) => {
      const url = typeof request === 'string' ? request : request.url;
      const type = url.endsWith('.wasm') ? 'application/wasm' : url.endsWith('.js') ?
        'text/javascript' : url.endsWith('.css') ? 'text/css' :
        url.endsWith('.webmanifest') ? 'application/manifest+json' : 'text/html';
      return { status: 200, type: 'basic', headers: { get: () => badAsset ? 'text/plain' : type },
        clone() { return this; } };
    }
  };
  vm.runInNewContext(fs.readFileSync('web/sw.js', 'utf8'), context);
  return { handlers, deleted, stored };
}

(async () => {
  for (const scope of ['https://example.com/MiniGame/', 'http://localhost:8080/']) {
    const sw = worker(scope);
    let pending;
    sw.handlers.install({ waitUntil: (promise) => { pending = promise; } });
    await pending;
    assert.equal(sw.stored.size, 7);
    sw.handlers.activate({ waitUntil: (promise) => { pending = promise; } });
    await pending;
    assert.deepEqual(sw.deleted, ['minigame-v1.1']);
    for (const url of ['https://elsewhere.com/app.js', new URL('unrelated.js', scope).href]) {
      let intercepted = false;
      sw.handlers.fetch({ request: { method: 'GET', url }, respondWith: () => { intercepted = true; } });
      assert.equal(intercepted, false);
    }
    let response;
    sw.handlers.fetch({ request: { method: 'GET', url: new URL('app.js', scope).href },
      respondWith: (promise) => { response = promise; } });
    assert.equal((await response).status, 200);
  }
  const invalid = worker('https://example.com/MiniGame/', true);
  let pending;
  invalid.handlers.install({ waitUntil: (promise) => { pending = promise; } });
  await assert.rejects(pending, /Invalid MiniGame/);
  assert.equal(invalid.stored.size, 0);
  console.log('Service-worker regressions passed.');
})().catch((error) => { console.error(error); process.exitCode = 1; });
