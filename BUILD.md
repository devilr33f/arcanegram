# Building Arcanegram

Patchset over [tdesktop](https://github.com/telegramdesktop/tdesktop). This doc covers the **Windows** debug build path that's been verified end-to-end. Other platforms are unverified — see notes at the end.

## Prerequisites

| Tool | Version used | Notes |
| --- | --- | --- |
| Node | 20+ | for the patchset scripts |
| pnpm | 10.x | declared in `package.json` |
| Git | 2.40+ | needs LFS not required |
| stgit | 2.5+ | `pip install stgit` or distro pkg |
| Visual Studio 2022 | 14.44 | any edition (BuildTools/Community/Professional/Enterprise); MSVC 14.44.35207 verified. The .bat helpers locate VS via `vswhere`; override toolset version with `MSVC_VER` env var if needed |
| Windows SDK | 10.0.26100 | comes with VS BuildTools |
| CMake | 3.31+ | bundled with VS BuildTools |
| Python | 3.13+ | for `prepare.py` (tdesktop dep build) |
| Qt | 6.8 | built locally by `prepare.py` (~30 min first time) |

Disk: ~30 GB peak (deps + worktree + intermediates), ~12 GB steady-state after a successful Debug build.

## One-shot setup

From the repo root:

```sh
pnpm install
pnpm run setup        # clones tdesktop into worktree/ + applies all patches via stgit
```

`setup` clones the `upstream-commit`-pinned tdesktop with submodules, applies the full `series` of patches with `stg`, and creates directory junctions so `src/cpp/arcanegram/` is mounted into `worktree/Telegram/SourceFiles/arcanegram/`. First run is ~15 min, ~5 GB. Re-running with `--force` re-imports patches into the existing worktree.

## Build deps (once per upstream bump)

The dep build is the slow part. Wrapped in `prepare-deps.bat`:

```sh
cmd //c "prepare-deps.bat"           # Debug-only deps (default, faster)
cmd //c "prepare-deps.bat both"      # Debug + Release deps
```

Runs `Telegram/build/prepare/win.bat qt6 [skip-release] silent` under `vcvars64`. This compiles Qt, OpenSSL, FFmpeg, openal-soft, libavif, tg_owt, etc. into `worktree/Libraries/win64/`. Allow ~30–60 min for `both`. Skipped on subsequent builds unless you delete `Libraries/`. If you previously ran with `debug` and now need Release, delete `Libraries/win64/` and rerun with `both`.

The script sources vcvars via `setup-vcvars.bat` (which uses `vswhere` to find your VS install) and sets `NoDefaultCurrentDirectoryInExePath=` because tdesktop's `prepare.py` invokes batch helpers without a `.\` prefix.

## Configure + build Telegram

Wrapped in `build-telegram.bat`:

```sh
cmd //c "build-telegram.bat"            # Debug only (default)
cmd //c "build-telegram.bat release"    # Release only
cmd //c "build-telegram.bat both"       # Debug + Release
cmd //c "build-telegram.bat configure"  # cmake re-configure only (no build)
```

What it does:

1. Sources vcvars via `setup-vcvars.bat`.
2. Sets `QT=6.8` and clears `NoDefaultCurrentDirectoryInExePath`.
3. Runs `Telegram/configure.bat qt6 x64 -DTDESKTOP_API_ID=$TDESKTOP_API_ID -DTDESKTOP_API_HASH=$TDESKTOP_API_HASH -DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=`. Defaults to TDesktop's public app id (2040) if env vars are unset; override either var to bake your own creds. Setting `TDESKTOP_API_TEST=1` adds `-DTDESKTOP_API_TEST=ON` (test datacenter).
4. Runs `cmake --build out --config <Debug|Release> --target Telegram` per requested config.

The Windows generator is multi-config (VS), so a single configure produces both Debug and Release in the same `out/` tree — switching between them is just a `--config` flag.

`-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=` (empty) suppresses `/Zi` debug info — matches the forkgram approach. Tdesktop's libavif (with bundled libyuv stripped) and tg_owt would otherwise produce duplicate-symbol warnings on link with PDB, see *Known issues* below. Affects Debug (Release defaults to no PDB anyway).

First clean Debug build: ~30–45 min. First clean Release build: ~30–45 min on top. Incremental relink after touching only fork code: ~30 s–2 min per config.

Result: `worktree/out/Debug/Telegram.exe` (~441 MB Debug binary), `worktree/out/Release/Telegram.exe` (~80–100 MB Release binary).

To re-run only the configure step (e.g. after editing `CMakeLists.txt`), use `cmd //c "build-telegram.bat configure"`.

## Quick sanity check

```sh
cd worktree/out/Debug
rm -f log.txt
./Telegram.exe -test
```

Should produce ~30 KB of `log.txt` with Qt/webrtc init messages. If it crashes immediately with `0xc00000fd`, see *Known issues*.

The `-test` flag points to Telegram's test datacenter so you can sign in without affecting your real account.

## Patch workflow

```sh
cd worktree

stg series                    # show stack
stg top                       # currently-topmost patch
stg show <name>               # diff for a patch

stg new feature__my-thing -m 'short description'   # start new patch
# ... edit worktree/Telegram/* directly ...
stg refresh                   # commit working-tree changes into the topmost patch

stg float feature__existing   # move existing patch to top to edit it
# ... edit ...
stg refresh

stg pop -a                    # unapply everything
stg push -a                   # reapply everything
stg push <name>               # apply one
```

`stg refresh` always lands changes in the topmost patch — make sure the right patch is on top first. Splitting changes across multiple patches: `stg pop` until the target is on top, edit, refresh, push the rest back.

After patch changes, export back to `patches/` + `series`:

```sh
cd ..
pnpm run export
git add patches/ series
git commit -m "feat(patches): short description"
```

## Rebasing onto newer upstream

```sh
pnpm run rebase latest        # or a specific SHA
# resolve conflicts in worktree/ via stg push, stg edit
pnpm run export
```

The pinned upstream is in `./upstream-commit`.

## Known issues

### libyuv duplicate symbols (Windows Debug)

libavif v1.3.0 and tg_owt both statically bundle libyuv. With PDB (`/Zi`) emitted, the link emits `LNK2005` for ~50 libyuv symbols (`ScalePlane`, `CopyPlane`, …). The current workaround is `-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=` (empty) which disables debug info entirely — this matches forkgram's recipe and avoids the duplicates without `/FORCE:MULTIPLE`.

`misc/build-support.patch` also strips libyuv from libavif's CMakeLists so the duplicate is eliminated structurally. If the patch fails to apply after an upstream bump, expect link-time duplicates back.

Stock tdesktop CI typically hides this because libavif is loaded from the GitHub Actions cache (built before the conflict appeared). Cache miss → same problem on stock too.

### Bisecting the original empty-static-lib crash

Earlier history of this repo includes `BROKEN.md` documenting a `STATUS_STACK_OVERFLOW` triggered by adding `arcanegram` to `target_link_libraries(Telegram PRIVATE ...)` even when the lib was empty. The crash is **no longer reproducible** as of the libyuv-dedup fix in `d3d18d…`. If you ever see it return on a fresh clone, the most likely cause is link-order shift around duplicate libyuv symbols re-introduced by an upstream bump.

Useful tools for that case:
- `dumpbin /directives <lib>` to see embedded `/DEFAULTLIB` entries.
- `cdbX64.exe -z <dump> -c "!analyze -v; ~*kn 50; q"` for a stack trace from the crash dump under `%LOCALAPPDATA%\CrashDumps\`.

### `/MTd` runtime everywhere

Stock tdesktop builds with `/MTd` (static debug CRT), not the CMake default `/MDd`. Any new lib added under `init_target` inherits this. Don't manually set `MSVC_RUNTIME_LIBRARY` to `MultiThreadedDebugDLL` — it will diverge and crash on startup.

## Other platforms

Linux / macOS aren't currently exercised. The `setup` / `export` / `rebase` scripts are platform-agnostic; what's missing is the build-driver shell scripts. Adapt by following [tdesktop's build docs](https://github.com/telegramdesktop/tdesktop/tree/dev/docs) — the patchset only changes things inside `worktree/Telegram/` and a few CMake transitive deps; from CMake's perspective it's a normal `Telegram` target.
