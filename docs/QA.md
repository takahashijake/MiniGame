# v1.2 regression verification

Run from the repository root. Native tests require CMake and a C++17 compiler;
Python 3 enables the CLI subprocess tests. Browser adapter tests require Node.js.

```sh
cmake -S . -B build -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
node --check web/app.js
node --check web/sw.js
node tests/test_web.cjs
node tests/test_sw.cjs
emcmake cmake -S . -B build-web -DMINIGAME_BUILD_TESTS=OFF -DMINIGAME_WARNINGS_AS_ERRORS=ON
cmake --build build-web --target minigame_web
node tests/test_web.cjs build-web/web-dist/minigame.js ./build/minigame_replay
```

On Windows, use the native replay executable path for the build configuration,
for example `./build/Release/minigame_replay.exe`.

## Automated coverage

| Boundary | Regression cases |
| --- | --- |
| Core persistence | 32 seeds with repeated restores; exploring, combat, victory and defeat; invalid loads preserve RNG/state |
| Combat | Exact Shield damage, inclusive Dragon enrage, blocked flee retaliation, healing turn cost, missing Potion, lethal hit, Rogue/Golem behavior |
| Native adapter | Seed limits and malformed input; EOF in exploration/combat/merchant; save/load in combat; reload from disk; corrupt save |
| Browser adapter + WASM | Autosave/Continue, corrupted save, denied storage, invalid seed correction, new-run reset, Dragon presentation |
| Portability | Fixed RNG vector and 40 native/WASM snapshot transitions with resumed RNG parity |
| Service worker | HTTPS/localhost allowlist, response validation, unrelated-cache isolation |

CI runs Linux/macOS/Windows native tests, Linux ASan/UBSan, and the real WASM/native
adapter comparison. Browser adapter tests simulate DOM elements rather than launching
a browser. Service-worker tests simulate worker events and CacheStorage.

## Manual browser smoke check

Serve `build-web/web-dist` over HTTP on localhost. Start seed 42, explore into the
Rogue encounter, attack, reload, and Continue. Verify HP and enemy HP are restored.
Check `A/H/R` combat shortcuts and `W/I/B` exploration shortcuts, merchant affordability,
and the eight-entry event log. Start seed 0 and verify gear/progression and history
reset. Repeat at a mobile viewport. After the service worker has installed, reload
offline and Continue. In a profile denying storage, confirm play works and autosave
reports unavailable. Corrupt the versioned localStorage value and confirm Continue
rejects it without an engine failure.

## October 6 validation

Local GCC 13.3 builds passed with warnings as errors. Both CTest groups (core and CLI)
passed normally and with ASan/UBSan. Leak detection was disabled locally because the
execution environment prevents LeakSanitizer process inspection; CI retains its normal
sanitizer configuration. Emscripten 3.1.6 built the complete bundle, and real WASM
adapter tests, the 40-step native/WASM comparison, JavaScript syntax checks, and worker
regressions passed. A graphical browser was unavailable locally, so manual visual and
real offline-install checks are not claimed by this report.
