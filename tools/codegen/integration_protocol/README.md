# Integration Protocol Registry Codegen

Generates the bot integration protocol registry from `include/fb/game/protocol/**/*.h`.

## Generate

```bash
python tools/codegen/integration_protocol/generate.py
```

Outputs:

- `include/fb/bot/integration/protocol_registry.generated.h`
- `server/bot/src/integration/protocol_registry.generated.cpp`
- `include/fb/bot/integration/protocol_lua.generated.h`
- `server/bot/src/integration/protocol_lua.generated.cpp`

## Visual Studio (`server/bot/bot.vcxproj`)

`server/bot/CMakeLists.txt` picks up new files via `GLOB`, but the local Visual Studio project does not.

When adding a new `.cpp` under `server/bot` (including generated registry files or hand-written sources like `protocol_registry.cpp`):

1. Add a `ClCompile` entry to `server/bot/bot.vcxproj` (absolute path, **alphabetical** order within the `ItemGroup`)
2. Add the matching `ClCompile` + `Filter` entry to `server/bot/bot.vcxproj.filters`
3. Do **not** run `cmake` to regenerate vcxproj — edit those files directly

Example:

```xml
<ClCompile Include="D:\git\fb\server\bot\src\integration\protocol_registry.cpp" />
<ClCompile Include="D:\git\fb\server\bot\src\integration\protocol_registry.generated.cpp" />
```

`bot.vcxproj` and `bot.vcxproj.filters` are gitignored local project files.

## Verify (CI)

```bash
python tools/codegen/integration_protocol/generate.py --check
```

Fails when protocol headers change but generated files were not regenerated.

## Registry contents

Each scanned `game_reqs::*` / `game_resp::*` type gets:

| Field | Response | Request |
|-------|----------|---------|
| `ensure_registered` | `ensure_handler_registered<T>()` | no-op |
| `clone` | copy from received header | `nullptr` |
| `create` | `nullptr` | `make_shared<T>()` |
| `marshal_lua` | field codegen | stub |

`protocol_registry::register_all()` registers response `clone` handlers by type key at `game_bot_controller` startup.
