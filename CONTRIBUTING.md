# Contributing

## Development loop

1. Configure: `cmake -S . -B build -DMINIGAME_WARNINGS_AS_ERRORS=ON`
2. Build: `cmake --build build --parallel`
3. Test: `ctest --test-dir build --output-on-failure`
4. Format: `make format` when `clang-format` is installed.

Keep terminal I/O in `TerminalUI` or the executable layer. Game rules belong in the core library so they remain deterministic and testable.

## Quality expectations

- Add tests for bug fixes and new game rules.
- Avoid raw owning pointers and hidden global state.
- Preserve cross-platform behavior; CI builds on Linux, macOS, and Windows.
- Treat warnings as errors before opening a pull request.
- Update `CHANGELOG.md` for user-visible behavior changes.
- Do not commit generated build outputs or local save files.

## Commit guidance

Prefer focused commits with imperative subjects such as `Add campaign save validation` or `Fix guard damage calculation`.
