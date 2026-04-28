---
name: debugging-arcanegram-build
description: Use when a build error appears in this arcanegram repo, when link-time symbol mismatches surface, when chat rendering crashes at runtime, or when a hide / setting feature behaves incorrectly after restart.
---

# Debugging Arcanegram Build

A `symptom → cause → fix` catalog. Search this file by error text first before debugging from scratch — many failures are repeats from prior sessions.

When you encounter a new symptom and resolve it, **append a new row** to the relevant table before completing the task.

## Compile errors

| Symptom (compiler text) | Cause | Fix |
|---|---|---|
| `'enumerateWindows': cannot access private member declared in class 'Core::Application'` | `Core::Application::enumerateWindows` is private | Use `Core::App().activeWindow()->widget()->update()` (the public path). |
| `'start_with_next' is not a member of 'rpl'` | Wrong rpl pattern — this codebase doesn't expose start_with_next | `producer \| rpl::on_next([](auto v) { ... }, Lifetime())`. `Lifetime()` is a static getter in your TU's anonymous namespace (mirror `ag_forwarded_header.cpp`). |
| `'lifetime' is not a member of 'Core::Application'` | No public `lifetime()` on App | Same fix as above — define a static `Lifetime()` getter in your anonymous namespace, don't reach into App. |
| `_qs` literal not declared / `error C2065: '_qs'` | Wrong Qt string-literal suffix in this codebase | Use `_q` (e.g. `u"foo"_q`, not `u"foo"_qs`). |
| `'settingsDividerLabel' is not a member of 'st'` | Wrong style name | Use `st::boxDividerLabel`. |
| `'settingsDividerLabelPadding' is not a member of 'st'` | Wrong style name | Use `st::defaultBoxDividerLabelPadding`. |
| `'ag_xxx' is not a member of 'tr'` after appending to `lang_arcanegram.strings` | Worktree copy of the strings file is stale (file symlink fell back to plain copy on Windows without dev mode) | `cp res/lang/lang_arcanegram.strings worktree/Telegram/Resources/langs/lang_arcanegram.strings` then re-build. |
| `_addAction` overload mismatch in `Window::Filler::*` (window_peer_menu.cpp) | `PeerMenuCallback::operator()` requires `(text, cb, const style::icon*)` — three args, not two | Pass `nullptr` as the icon argument. |
| `Ui::VerticalLayout::add: no matching overloaded function found` | Likely passing `style::margins` by name when the API wants a different type, or wrong overload | Check `wrap/vertical_layout.h` — overloads are `(object_ptr, style::align)` and `(object_ptr, const style::margins &, style::align)`. |
| libyuv duplicate symbols (LNK2005, ~50 symbols like `ScalePlane`, `CopyPlane`) | libavif and tg_owt both bundle libyuv with `/Zi` debug info | See `BUILD.md` "Known issues" — `-DCMAKE_MSVC_DEBUG_INFORMATION_FORMAT=` (empty) in `build-telegram.bat` suppresses /Zi entirely, dodging the duplicate. `misc/build-support.patch` also strips libyuv from libavif structurally. |

## Runtime / behavior bugs

| Symptom | Cause | Fix |
|---|---|---|
| Hidden-users list resets to empty after every restart, even though saving worked in the same session | `Hooks::init()` ran before `startLocalStorage()`, so `_prefs` was empty when `Load()` read it from disk | `hooks__hooks-init` patch must place `Arcanegram::Hooks::init()` **after** `startLocalStorage()` in `Application::run` (Telegram/SourceFiles/core/application.cpp ~line 271). |
| Hide-user works in scheduled / discussion / saved-messages threads but messages still show in regular chats | Patched `HistoryView::ListWidget::refreshRows` only — that class isn't used by `HistoryInner` (regular chat view) | Override `Element::isHidden()` (consulted by `Message::resizeContentGetHeight`) and add early-returns in `Message::draw` and `Message::resizeContentGetHeight` for the user-hide case. |
| Hidden user's messages collapse but leave a margin-sized gap | `Message::resizeContentGetHeight` returns `marginTop()+marginBottom()` for `isHidden()` (which is what group-hide wants) | Add an explicit user-hide branch returning `0` *before* the existing `isHidden()` margin branch. Also early-return at the top of `Message::draw`. |
| Reactions from hidden users still show on the bubble strip | `InlineListDataFromMessage` (history_view_reactions.cpp ~913) builds `result.recent` and `result.reactions` from raw item data | Filter `result.recent[id]` by `Arcanegram::HiddenUsers::IsHidden`; subtract from matching `result.reactions[i].count`; drop reaction entirely if count drops to ≤ 0 (except paid). |
| Reactions still show in "who reacted" popup despite filtering on the strip | `api/api_who_reacted.cpp` resolves peers separately for the popup | Add `IsHidden` check to the resolved-peers filter alongside the existing `nullptr` check (~line 443). |
| `STATUS_STACK_OVERFLOW` on Telegram launch | Historical link-order issue tied to libyuv duplicates being re-introduced after an upstream rebase | See `BROKEN.md` for the bisect notes. Check `dumpbin /directives <lib>` for embedded `/DEFAULTLIB` entries; verify `misc/build-support.patch` still strips libyuv from libavif. |
| Settings toggle persists in-memory but doesn't survive restart | Either the Hooks::init init-order issue (see above) — most common cause — or the `_saveSettingsTimer` hadn't been initialized yet when `setValue` was called | Verify `Hooks::init()` is called after `startLocalStorage()`; `setValue` triggers `Core::App().saveSettingsDelayed()` which only fires once `_saveSettingsTimer` exists (created in `Application::startLocalStorage`). |

## Stgit gotchas

| Symptom | Fix |
|---|---|
| `stg pop misc/branding.patch` errors with `invalid patch range` | Stgit uses underscore form internally — `stg pop misc__branding`. |
| `stg sink` puts patch in unexpected slot | Use `stg sink --to=<target>` to land *immediately before* the target in apply order. Always verify with `stg series` after. |
| `series` file diverges from stgit internal order across sessions | Internal stgit order is the source of truth; the `series` file in repo is just an export snapshot. Run `pnpm run export` to rewrite `series` to match current state. |
| `stg refresh` captured the wrong patch | Make sure the right patch is on top first (`stg top` to verify, `stg float <name>` to bring one up, then `stg refresh`). |
| Adding a new patch and old patches show as modified after `pnpm run export` | Whitespace / commit-hash drift from re-export. Don't stage them unless you actually changed the patch content — they'll re-export consistently next time. |

## Build invocation

If `build-telegram.bat` fails with `'build-telegram.bat' is not recognized`, you're running it through `cmd //c` from Git Bash — cwd inheritance breaks. Run it directly:

```bash
cd <repo root> && ./build-telegram.bat
```

Background execution is fine. Expect 30s–5min for incremental rebuilds, 30–45min for clean Debug.

## When the symptom isn't here

Debug normally — read the actual error, locate the source line, identify the assumption that's broken, fix it. Then **append a new row to this catalog** so the next session benefits.
