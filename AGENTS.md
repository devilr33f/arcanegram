# Arcanegram Agent Guide

## Rules

- Fork logic lives in `src/cpp/arcanegram/`, fork-owned resources in `res/`.
- Stock patches: minimal. No renames. Prefer one-line hooks into `Arcanegram::*`.
- Never add fork files to stock patches.
- `patches/` and `series` are export targets, not source of truth. Do not hand-edit.
- Patch commit subject = plain human-readable explanation.
- Don't run stgit yourself unless explicitly asked.
- "topmost" = what `stg top` returns; read via `stg show`.
- If a requested feature is already in stock (even partially), notify the user first.
- Prefer data-layer patches over UI-layer patches.
- Edit the worktree directly — never touch patch files.
- Never touch a submodule. If a feature requires it, find another way or skip.

When you'd add fields/methods to stock classes: first try exposing `private:` → `public:` in a tiny patch. If a method must be added, prefix `ag_`.

When stock logic needs mode-dependent behavior, use `if (Arcanegram::Config::Foo()) { ... } else { /* stock unchanged */ }` with no reindent of the stock branch — keeps rebase noise minimal.

## Patch naming

Format: `group__name` → `patches/<group>/<name>.patch`.

| group | when |
| --- | --- |
| `bugfix` | fixes a stock bug |
| `feature` | adds user-facing capability (qol, ui tweak, customization) |
| `debloat` | hides/disables stock behavior behind a setting toggle |
| `hooks` | thin hook into stock so fork code has a call site |
| `misc` | build, branding, infra, settings entry points |

Example: `stg new feature__double-tap-to-edit -m 'Allow editing by double tapping a message'`.

## Patch scope

- Patches add hook calls, wiring, and tiny stock changes.
- Feature logic goes in `src/cpp/arcanegram/`.
- A patch touching only `src/**` or `res/**` is wrong — those changes live outside patches.

## Storage

- Never modify `Core::Settings` / `Main::SessionSettings` `QDataStream` serialization.
- Use `Core::Settings::writePref<T>` / `readPref<T>` (generic KV) instead.
- All keys prefixed `ag_`.

## Project landmarks

- `Arcanegram::Hooks::init()` — central init bus, called from `Application::run` via `hooks/hooks-init.patch`.
- `Arcanegram::Config::*` — pref registry. New pref = one `inline auto Foo = BoolItem("ag_foo", false);` line in `ag_config.h`.
- `Arcanegram::Settings::Id()` — top-level fork settings page.
- Each feature exposes a `Setup(::Settings::SectionBuilder &)` registered in `ag_settings_main.cpp`.

## Strings

- All keys in `res/lang/lang_arcanegram.strings`, prefixed `ag_`.
- Title/subtitle pattern: `<key>` + `<key>_info`.
- Access: `tr::ag_xxx(tr::now)` or `tr::ag_xxx()` for reactive.

## Code style

- Follow upstream `AGENTS.md` style for fork C++ code: no descriptive single-line comments, `auto` for type deduction, `tr::lng_…` for strings, `.style` for sizes.
- All fork code in `Arcanegram::` namespace.
- One translation unit per feature; split when a TU passes ~300 lines.

## Project skills

Auto-loaded from `.claude/skills/`. Use them when their triggers match.

- `writing-arcanegram-patches` — workflow + conventions for any patch work.
- `researching-upstream-feature` — find upstream tdesktop hooks; carries a seen-this-before reference table.
- `debugging-arcanegram-build` — symptom→cause→fix catalog for build/runtime failures from prior sessions.
