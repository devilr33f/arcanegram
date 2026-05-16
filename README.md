# Arcanegram

Personal patchset for [Telegram Desktop](https://github.com/telegramdesktop/tdesktop).

> **No support, no warranty, no promises.** Issues, questions, and feature requests will not be answered. Use at your own risk. See [LICENSE](LICENSE).

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

- **libyuv duplicate symbols (`LNK2005`) on Windows Debug.** libavif and tg_owt both statically bundle `libyuv`, causing `LNK2005` for ~50 symbols (`ScalePlane`, `CopyPlane`, etc.) when linking with PDB (`/Zi`). Worked around by passing `-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=` (empty) in `build-telegram.bat`, which disables debug info and avoids the conflict. The binary builds and runs correctly. If you re-enable `/Zi`, expect duplicate-symbol link errors.

## License

[Unlicense](LICENSE) (public domain) with an explicit no-support clause. Do whatever you want with the code; expect nothing in return.
