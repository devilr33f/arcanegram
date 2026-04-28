---
name: writing-arcanegram-patches
description: Use when adding or editing a stgit patch in this arcanegram repo, modifying any file under worktree/Telegram/SourceFiles/, adding fork code under src/cpp/arcanegram/, or running stgit operations on the arcanegram patch stack.
---

# Writing Arcanegram Patches

Arcanegram is a stgit-managed patchset over upstream tdesktop. This skill captures the workflow and conventions so a fresh agent can write a patch correctly without re-discovering pitfalls.

## Layout — what lives where

- `worktree/` — stgit-managed tdesktop clone. Stock files live under `worktree/Telegram/SourceFiles/`. Every edit here becomes an stgit patch.
- `src/cpp/arcanegram/` — fork code. Junctioned into `worktree/Telegram/SourceFiles/arcanegram/`. **Not** an stgit patch — edits propagate via the directory junction and are committed normally.
- `res/lang/lang_arcanegram.strings` — fork lang strings source.
- `worktree/Telegram/Resources/langs/lang_arcanegram.strings` — worktree copy. **Note:** file symlinks fall back to plain copy on Windows without dev mode, so editing the source does **not** propagate. See "Lang strings sync" below.
- `patches/<group>/<name>.patch` — exported stgit patches (slash form, with `.patch`).
- `series` — exported list of patches in apply order.
- `AGENTS.md`, `BUILD.md` — authoritative project conventions and build docs.

## Stgit naming

Stgit uses **underscore form** internally: `misc__pref-string`, `feature__hidden-users-history`, `hooks__hooks-init`.
The slash form (`misc/pref-string.patch`) only appears in `patches/` and `series` after `pnpm run export`.

`stg pop`/`push`/`sink`/`show`/`new`/`refresh` always take the underscore form.

## Workflow for a new stock patch

1. Pop branding so new patches go below it:
   ```bash
   cd worktree && stg pop misc__branding
   ```
2. Create the empty patch on top:
   ```bash
   cd worktree && stg new <group>__<name> -m '<plain-english subject>'
   ```
3. Edit the **single** stock file in `worktree/Telegram/SourceFiles/...`. Add `#include "arcanegram/features/ag_<feature>.h"` if invoking fork code.
4. Capture the change:
   ```bash
   cd worktree && stg refresh
   stg show <group>__<name> --stat
   ```
5. After all new patches done, push branding back:
   ```bash
   cd worktree && stg push misc__branding
   ```
6. (Optional) Sink mid-stack:
   ```bash
   cd worktree && stg sink --to=<target> <patch>
   ```
7. Build to verify (see "Build" below).
8. Export and commit (see "Export + commit" below).

**One stock file per patch.** Multiple unrelated edits = multiple patches. Single feature touching multiple files = OK to bundle.

**Subjects.** Plain English, no Conventional Commits prefix in the *patch subject*. The wrapping `git commit` later uses Conventional Commits.

## Workflow for fork-only changes

Fork code lives in `src/cpp/arcanegram/`. No stgit involvement. Edit, build, commit at the end with the rest of the work.

Per-feature pattern (mirror `ag_forwarded_header`):
- Header in `features/ag_<feature>.h` exposing `Init`, optional `Setup(SectionBuilder &)`, and any feature-specific entry points.
- Implementation in `features/ag_<feature>.cpp` with anonymous-namespace internals (Set/Stream/Lifetime getters), then public functions.
- Pref entry in `ag_config.h`: `inline auto Foo = BoolItem("ag_foo", false);` under a sub-namespace.
- Wire `Foo::Setup(builder)` into `ui/ag_settings_main.cpp::BuildContent`.
- Wire `Foo::Init()` into `ag_hooks.cpp::init`.
- Add the new sources to `nice_target_sources` in `src/cpp/arcanegram/CMakeLists.txt`.

## Lang strings sync

After editing `res/lang/lang_arcanegram.strings`, **always** run:

```bash
cp res/lang/lang_arcanegram.strings worktree/Telegram/Resources/langs/lang_arcanegram.strings
```

Reason: `linkForkEntry` in `scripts/lib.ts` tries `fs.symlink` first (file type), which on Windows requires dev mode and fails silently into `fs.copyFile`. The two files then diverge until the next `pnpm run setup --force`.

The codegen reads the worktree copy at CMake configure time (see `Telegram/cmake/td_lang.cmake`, patched by `misc__lang-keys`). New keys will fail to compile as `tr::ag_xxx` until the worktree copy is refreshed.

## AGENTS.md essentials

- Prefer data-layer patches over UI-layer patches.
- All keys prefixed `ag_`.
- Comments lowercase if any. Default to no comments unless WHY is non-obvious.
- Never modify `Core::Settings` / `Main::SessionSettings` `QDataStream` serialization — use `Item<T>` with `writePref<T>` / `readPref<T>`.
- One TU per feature; split when ~300 lines.
- Patches add hook calls + tiny stock changes; feature logic stays in `src/cpp/arcanegram/`.
- A patch touching only `src/**` or `res/**` is wrong — those changes live outside patches.
- Don't touch submodules. If a feature requires it, find another way or skip.

## Series order

New patches go between `feature__forwarded-header-date` and `misc__branding`. After creating, use `stg sink --to=<target>` if you need an exact slot.

The `series` file in the repo is regenerated by `pnpm run export` from stgit's internal order, which **may diverge** from what `series` looked like at the start of the session (prior agents reordered patches without exporting). Don't fight it — internal order is the source of truth. Run export and verify the resulting `series`.

## Build

From repo root in **Git Bash**:

```bash
./build-telegram.bat
```

Don't use `cmd //c build-telegram.bat` — cwd inheritance breaks and the script doesn't resolve. Background runs are fine; expect 30s–5min for incremental, 30–45min for clean Debug.

The script source-prepends `vcvars64`, runs configure with debug-info disabled (workaround for libyuv duplicate symbols — see `BUILD.md`), then builds the `Telegram` target.

## Export + commit (final step)

```bash
pnpm run export
git status
```

Stage **only your work**. Specifically:

- New `patches/<group>/<name>.patch` files
- `series` (modified)
- New / modified files under `src/cpp/arcanegram/`
- `res/lang/lang_arcanegram.strings` (if touched)

**Don't commit:**

- Pre-existing user-modified files (initial git status often shows `ag_forwarded_header.{cpp,h}`, `.gitignore`, untracked `BUILD.md`/`INUGRAM_PATCHES.md`/`reconfigure.bat` — these are the user's own in-progress work).
- Existing patch files showing as modified due to `pnpm run export` whitespace/hash drift unless you actually changed them.

Commit message follows Conventional Commits: `feat(patches): ...`, `fix(patches): ...`, `docs: ...`.

**One commit at the end.** Don't make incremental git commits during patch work — stgit owns its own commit chain via `stg refresh`.

## Quick checklist before declaring done

- [ ] All new stock patches show as exported under `patches/<group>/<name>.patch`.
- [ ] `series` reflects the intended order (verify with `cat series`).
- [ ] `lang_arcanegram.strings` source matches the worktree copy.
- [ ] Build succeeds (or you've explicitly skipped per user instruction).
- [ ] `git status` shows only your intended files staged.
- [ ] `misc__branding` is back on top of the stgit stack.

## Cross-references

- `AGENTS.md` — canonical project rules.
- `BUILD.md` — build prerequisites + known issues (libyuv).
- `BROKEN.md` — historical link-time crash notes.
- `docs/INUGRAM_PATCHES.md` — inugram (Android) feature catalogue, used as a port-candidate menu.
