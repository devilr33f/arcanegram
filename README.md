# Arcanegram

Personal patchset for [Telegram Desktop](https://github.com/telegramdesktop/tdesktop). Open-source. Builds binaries. No support promise.

Inspired by [inugram](https://github.com/teidesu/inugram) (Telegram Android patchset).

## Layout

- `patches/` — exported `.patch` files, applied in order from `series`.
- `series` — patch order; export target, do not hand-edit.
- `upstream-commit` — pinned tdesktop SHA.
- `src/cpp/arcanegram/` — fork C++ code, junctioned into the worktree at setup.
- `res/` — fork-owned resources (lang strings, icons).
- `worktree/` — gitignored local tdesktop checkout managed by stgit.

Patch groups: `bugfix | feature | debloat | hooks | misc`.

## Setup

Requirements:
- Node 20+, pnpm
- git, stgit (`stg --version`)
- tdesktop build toolchain — see [tdesktop build docs](https://github.com/telegramdesktop/tdesktop/tree/dev/docs)

```sh
pnpm install
pnpm run setup
```

Clones tdesktop into `worktree/` (with submodules — first run is ~15 min, ~5 GB), applies patches, junctions fork code in. Open `worktree/` in your IDE and build the `Telegram` target.

## Adding / modifying patches

```sh
cd worktree
stg new feature__my-thing -m 'short description'
# ...edit worktree...
stg refresh
cd ..
pnpm run export
git add patches/ series
git commit -m "feat(patches): short description"
```

To edit an existing patch, push it to the top first:

```sh
cd worktree && stg float feature__my-thing
# ...edit...
stg refresh
cd .. && pnpm run export
```

## Rebasing onto newer upstream

```sh
pnpm run rebase latest
# ...resolve any conflicts in worktree, then:
pnpm run export
```

## Windows specifics

- Directory junctions are used instead of symlinks for the fork-source dir, so no admin / dev mode required.
- File-level "symlinks" fall back to copy + `git update-index --skip-worktree` on platforms that refuse `fs.symlink` for files.

## Known issues

- **Windows debug binary crashes on launch (stack overflow, `0xc00000fd`).** libavif v1.3.0 and tg_owt (WebRTC) both statically bundle their own copies of `libyuv`. With `AVIF_LIBYUV=OFF` (what `prepare.py` sets), libavif's CMake explicitly compiles its bundled libyuv source files as a fallback (line ~441 of libavif's `CMakeLists.txt`). At Telegram's final link, both `avif.lib` and `tg_owt.lib` define the same libyuv symbols — `LNK2005` for ~50 symbols including `ScalePlane`, `CopyPlane`, etc. `misc/build-support.patch` adds `/FORCE:MULTIPLE` so the link succeeds, but the resulting binary crashes early due to libyuv version/ABI mismatch.

  Stock tdesktop CI hides this on every successful run because libavif is loaded from the GitHub Actions cache (built fresh long before the conflict appeared). On any cache miss they would hit the same crash. So this is a real upstream issue, not something patches in this repo can solve cleanly. Real fix would require deduplicating libyuv at the dep-build level — modifying `prepare.py` to build libavif against an external libyuv binary, or refactoring `tg_owt` to not bundle its copy.

## License

MIT.
