# NIAHttpBOT Agent Guide

## Scope

- NIAHttpBOT is a C++20 HTTP service for BDS automation. The native entry point and server wiring live in [src/app/NIAHttpBOT.cpp](src/app/NIAHttpBOT.cpp).
- Native helper headers used by the backend live in [src/ui](src/ui).
- Prefer linking to existing project docs instead of duplicating them. Start with [README.md](README.md).

## Build And Validation

- Use the checked-in CMake presets from [CMakePresets.json](CMakePresets.json):
  - Debug: `cmake --preset vs2022-x64-debug --fresh && cmake --build --preset build-debug`
  - Release: `cmake --preset vs2022-x64-release --fresh && cmake --build --preset build-release`
- There is no maintained automated test suite under [test](test). For backend changes, always at least build the touched target and manually exercise affected HTTP endpoints.

## Codebase Map

- [src/app](src/app): process startup, config load/save, HTTP server startup, module registration.
- [src/api](src/api): HTTP route modules for BDS, file, and game features.
- [src/bot/qq](src/bot/qq): NapCat/QQ integration and QQ-specific HTTP handlers.
- [src/common](src/common): shared config model, HttpV1 helpers, and logger macros.
- [src/ui](src/ui): native helper headers such as [src/ui/Graphics.hpp](src/ui/Graphics.hpp).

## Project Conventions

- Keep request parsing and response formatting on the HttpV1 helpers in [src/common/HttpV1.hpp](src/common/HttpV1.hpp). New endpoints should use `ParseRequestV1`, `RespondSuccess`, and `RespondFail` instead of inventing a new JSON envelope.
- Route modules follow the `init_*_API(httplib::Server&)` pattern and are wired from [src/app/NIAHttpBOT.cpp](src/app/NIAHttpBOT.cpp). If you add a new module, add both the init function and the registration call.
- Keep config schema changes synchronized across [src/common/AppConfig.hpp](src/common/AppConfig.hpp), [src/common/AppConfig.cpp](src/common/AppConfig.cpp), and [NIAHttpBOT.json](NIAHttpBOT.json).
- Prefer the logging and i18n macros from [src/common/Logger.hpp](src/common/Logger.hpp) for user-facing messages instead of raw `std::cout`.
- Preserve the Windows UTF-8 and console setup in [src/app/NIAHttpBOT.cpp](src/app/NIAHttpBOT.cpp) when touching startup, logging, or terminal output.

## Known Pitfalls

- The preset names say `vs2022`, but the generator string in [CMakePresets.json](CMakePresets.json) is `Visual Studio 18 2026`. Reuse the existing preset names and verify generator changes deliberately.
- OpenSSL is wired as vendored Windows libraries from [lib](lib) in [CMakeLists.txt](CMakeLists.txt). Do not switch TLS linkage casually when changing build logic.
- The project communicates over HTTP, not HTTPS. Follow the deployment warning in [README.md](README.md): do not assume it is safe to expose service ports publicly or split the bot and BDS across untrusted hosts.
- Local BDS development has extra prerequisites, including loopback exemption on Windows. Link users to [README.md](README.md) instead of duplicating the setup steps.

## Useful References

- [README.md](README.md): setup, runtime expectations, NapCat/BDS prerequisites, and deployment caveats.
- [CMakeLists.txt](CMakeLists.txt): native dependency wiring, OpenSSL copy step, and compiler flags.
- [CMakePresets.json](CMakePresets.json): supported configure/build presets.
- [NIAHttpBOT.json](NIAHttpBOT.json): sample runtime configuration.