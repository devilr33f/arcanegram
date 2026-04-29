---
name: researching-upstream-feature
description: Use when locating where in upstream tdesktop a UI element, menu, data flow, or rendering surface is generated — for example to find the right hook point before writing a stock patch, or to disambiguate parallel rendering hierarchies (HistoryInner vs HistoryView::ListWidget) in this arcanegram repo.
---

# Researching Upstream Feature

Tdesktop is large (~700K LOC) and has multiple parallel hierarchies for similar UI surfaces. Before writing a stock patch, you need to find the *right* hook point — and verify that hook is reached by the path the user cares about. This skill captures the methodology and the known-mappings reference.

## Methodology

### 1. Identify the surface

What is the user actually looking at? Be specific:
- "Avatar context menu" — clicked which avatar, in which view (chat history, profile, dialog row)?
- "Reactions" — the chip strip on the bubble, the avatar mini-strip, the "who reacted" popup, or the picker?
- "Settings row" — main page, sub-page, sub-sub-page?

A vague surface gives a vague hook. Restate the surface to yourself before grepping.

### 2. Disambiguate parallel hierarchies

The single most important pitfall in tdesktop: the same kind of UI uses different code paths in different contexts. Most importantly:

- **Regular 1:1 / group chats** render via `HistoryInner` walking `History::blocks`. Each block holds `Element` views.
- **Discussion threads / scheduled / saved messages** render via `HistoryView::ListWidget` with `_items` slices.

Both share `Element` view objects, so hooks at the `Element` level (e.g., `Element::isHidden`, `Message::draw`, `Message::resizeContentGetHeight`) cover **both**. Hooks at the list/loop level (`ListWidget::refreshRows`, `HistoryInner::paintEvent`) cover **only one**.

Other parallel hierarchies to watch for:
- `Filler::fillProfileActions` (profile kebab) vs `Filler::fillChatsListActions` (dialogs row RMB).
- `dialogs/ui/dialogs_message_view.cpp::MessageView::prepare` (dialogs row preview) vs `Reply::update` in `history_view_reply.cpp` (reply quote inside a message).
- Stock `Settings::writePref<bool>` vs the fork-added `<QString>` specialization.

If a patch only works in one context but not another, the parallel-hierarchy assumption is the most likely bug.

### 3. Grep with intent

Common keyword sets per surface type:

| Looking for | Grep pattern |
|---|---|
| Menu items | `addAction\|fillMenu\|FillMenu\|context.*menu\|_menu->addAction` |
| Rendering | `::draw\(Painter\|::paintEvent\|view->draw` |
| Sizing / layout | `::resizeGetHeight\|::resizeContentGetHeight\|setHeight\|recountHeight` |
| Data composition for views | `DataFromMessage\|FromMessage\|toPreview\|::prepare\(` |
| Stored prefs | `writePref\|readPref\|writePrefImpl\|readPrefImpl` |
| Notifications | `skipNotification\|System::skip\|notifications_manager` |
| History add/remove | `addItemToBlock\|addNewMessage\|removeMessage\|destroyView` |
| Avatar / userpic | `userpic\|fillSenderUserpicMenu\|linkUserpicPeerId` |
| Reactions | `MessageReactions\|recentReactions\|InlineList\|RecentReaction` |

### 4. Verify each candidate

Read **20–30 lines** around the match before recommending it. Confirm:
- The function actually executes in the path the user cares about.
- The variables you'd hook against are in scope (`item`, `peer`, `view`, etc.).
- The insertion point doesn't break logic (e.g., a `continue` inside a loop that reserves indexes).

### 5. Output a structured candidates table

Even if you found the answer immediately, present candidates in a table so the next step (writing the patch) has the right context:

```
| Surface | File:Line | Function | Why this hook | Grep next if not it |
|---------|-----------|----------|---------------|---------------------|
```

## Seen-this-before reference

Built up from prior sessions. Use as a starting point — verify line numbers haven't drifted (upstream rebases).

| Surface | File | Anchor |
|---|---|---|
| Avatar RMB (chat history) | `history/history_inner_widget.cpp` | inside `if (linkUserpicPeerId)` block, after `Window::AddSenderUserpicModerateAction` (~line 2607) |
| Profile kebab | `window/window_peer_menu.cpp` | `Filler::fillProfileActions` (~line 1782) |
| Reactions strip data | `history/view/reactions/history_view_reactions.cpp` | `InlineListDataFromMessage` (~line 913) |
| Who-reacted popup peers | `api/api_who_reacted.cpp` | resolved-peers filter inside `UpdateUserpics` (~line 425) |
| Dialog row preview | `dialogs/ui/dialogs_message_view.cpp` | `MessageView::prepare` (~line 155) |
| Per-message bubble timestamp | `history/view/history_view_bottom_info.cpp` | `BottomInfo::layoutDateText` (~line 460) |
| Message resize / draw | `history/view/history_view_message.cpp` | `Message::resizeContentGetHeight` (~5106), `Message::draw` (~1063) |
| `Element::isHidden` virtual | `history/view/history_view_element.cpp` | line ~1422 (consumed by ListWidget + Message::resize) |
| Reply preview text + sender name | `history/view/history_view_reply.cpp` | `Reply::update` (~330), `Reply::updateName` (~598) |
| Notification skip chain | `window/notifications_manager.cpp` | `System::skipNotification` OR-chain (~294) |
| History list item filter (threads / scheduled / saved) | `history/view/history_view_list_widget.cpp` | `ListWidget::refreshRows` loop (~673) |
| Settings KV pref impl | `core/core_settings.cpp` | `writePrefImpl<T>` / `readPrefImpl<T>` (~1226–1250) |
| History add view to block | `history/history.cpp` | `History::addItemToBlock` (~1682) |
| `Application::run` init order | `core/application.cpp` | `Hooks::init` MUST come after `startLocalStorage()` (~265–271) |
| Lang codegen wiring | `Telegram/cmake/td_lang.cmake` | `arcanegram_lang_combined` concat (via `misc__lang-keys`) |
| Per-feature settings registration | `src/cpp/arcanegram/ui/ag_settings_main.cpp` | `BuildContent(SectionBuilder &)` |
| Per-feature init | `src/cpp/arcanegram/ag_hooks.cpp` | `init()` |
| SectionBuilder API | `Telegram/SourceFiles/settings/settings_builder.h` | `container()` returns `Ui::VerticalLayout *`; no `addWrap()` |
| `Ui::SettingsButton` constructor | `Telegram/lib_ui/ui/widgets/buttons.h` | `(QWidget *parent, rpl::producer<QString> &&text, const style::SettingsButton & = st::defaultSettingsButton)` |
| Earliest message ingest (nullable return) | `data/data_session.cpp` | `Session::addNewMessage(MTPMessage)` (~3099) and `(MsgId, MTPMessage, ...)` (~3106) — returns `HistoryItem*`, already short-circuits empty/no-peer; safe drop point |
| History-side ingest (NOT droppable) | `history/history.cpp` | `History::addNewMessage` (~555) returns `not_null<HistoryItem*>`; cannot drop here, must filter at Session layer or skip downstream side effects |
| Post-create routing (blocks vs lastMessage) | `history/history.cpp` | `History::addNewItem` (~728) — calls `setLastMessage`/`addNewToBack`/`newItemAdded`; hook here to suppress dialog row + unread without losing the item |
| Unread count bump + notif schedule | `history/history.cpp` | `History::newItemAdded` (~1544) — bumps `setUnreadCount(unreadCount()+1)` at lines ~1569/1586; early-return here to keep the item but freeze unread counter |
| Dialog row anchor (`_lastMessage`) | `history/history.cpp` | `History::setLastMessage` (~2854) — stamps the message that drives the dialogs row preview |
| Dialog row preview computation | `history/history.cpp` | `History::computeChatListMessageFromLast` (~2917) — currently only walks past migration messages; extend to walk past hidden-user messages too |
| Typing indicator ("X is typing...") | `data/data_send_action.cpp` | `SendActionManager::registerFor` (~47) and painter clear at `History::newItemAdded` (~1549) |
| @mention autocomplete | `chat_helpers/field_autocomplete.cpp` | `updateFiltered` populates `mrows` from participants/admins |
| MTProto `initConnection` builder | `mtproto/session_private.cpp` | `SessionPrivate::sendPrepared` (~671) — wraps every layer-bearing request in `MTPInitConnection<>(api_id, device_model, system_version, app_version, lang…)`; TL signature at `mtproto/scheme/api.tl:2154`. **No `app_name` field exists** — display name comes from server-side api_id registration |
| `initConnection` extra JSON params | `mtproto/session_private.cpp` | `SessionPrivate::prepareInitParams` (~549) — only sends `tz_offset`; the only place to inject custom JSON visible to the server |
| `device_model` plumbing | `main/main_account.cpp` (~420, ~567) → `mtp_instance.cpp` (~904 `Private::deviceModel`, ~333 `_deviceModelDefault` init) → `Platform::DeviceModelPretty()` in `lib_base/base/platform/{win,mac,linux}/base_info_*.cpp`; user override via `Settings::customDeviceModel()` (`core/core_settings.cpp:1342`) |
| `app_version` builder | `mtproto/session_private.cpp` | `ComputeAppVersion` (~79) — `AppVersionStr` + arch (` x64`) + store/sandbox suffix (` Mac App Store` / ` Microsoft Store` / ` Flatpak` / ` Snap`) |
| ApiId / ApiHash | `Telegram/SourceFiles/config.h` | line 68 (build-time `TDESKTOP_API_ID` / `TDESKTOP_API_HASH`); line 90 fallback to leaked test pair `17349` / `344583e4…` (guarded by `#error`) |
| Active session display name override | `api/api_authorizations.cpp` | `ParseEntry` (~40) — hardcodes `"Telegram Desktop"` when `apiId ∈ {DesktopApiId=2040, SnapApiId=611335, TestApiId=17349}`, otherwise `qs(data.vapp_name())`. Pure client-side cosmetic; other clients see whatever name the api_id was registered with on my.telegram.org |
| Mini App `requestSimpleWebView` call | `inline_bots/bot_attach_web_view.cpp` | `WebViewInstance::requestSimple` (~1174) — full ctor: `MTPmessages_RequestSimpleWebView(flags, bot:InputUser, url:bytes, start_param:string, theme_params:DataJSON, platform:string)`; `Flag::` enum has `f_url, f_start_param, f_theme_params, f_from_switch_webview, f_from_side_menu, f_compact, f_fullscreen`. Result is unified `MTPWebViewResult`/`MTPDwebViewResultUrl`; URL via `result.data().vurl()` |
| Username resolve via MTProto | `inline_bots/bot_attach_web_view.cpp` | (~2482) — current signature: `MTPcontacts_ResolveUsername(MTP_flags(0), MTP_string(username), MTP_string()/*referrer*/)`. Result `MTPDcontacts_resolvedPeer`; call `processUsers/processChats`, then `peerFromMTP(data.vpeer())` → `session->data().peer(peerId)->asUser()->inputUser` |
| `MTP::Sender` construction | `mtproto/sender.h:17` | ctor takes `MTP::Instance*`. Build with `MTP::Sender(&session->api().instance())`. Pattern: `_api.request(MTPxxx(...)).done([=](const Result &r){}).fail([=](const MTP::Error &e){}).send()` |
| Active-session rpl stream | `main/main_domain.h:64` | `Domain::activeSessionValue()` (`rpl::producer<Main::Session*>`) fires immediately + on changes; `activeSessionChanges()` only on changes. Always null-check (logout fires `nullptr`) |
| `SectionBuilder` text-input/status helpers | `settings/settings_builder.h` | none built-in — use `builder.container()` (returns `Ui::VerticalLayout*`) and add `Ui::InputField`/`Ui::FlatLabel` directly. Example InputField construction at `settings/business/settings_chatbots.cpp:437` |
| `Show Peer IDs in Profile` experimental option | `info/profile/info_profile_actions.cpp` | toggle declared (~116, `kOptionShowPeerIdBelowAbout = "show-peer-id-below-about"`); display+link emitted in `AboutWithAdvancedValue` (~215-225) using raw mtproto `peer->id.value & PeerId::kChatTypeMask`; URL `internal:~peer_id~:copy:<raw>`; RMB context-menu hook for that URL prefix at ~1471-1485; LMB handler `CopyPeerId` at `core/local_url_handlers.cpp:843` (regex `^copy:(.+)$` at ~1779). Registered in *Experimental* settings at `settings/settings_experimental.cpp:213` |
| Bot API peer-id format | n/a (no upstream helper) | User: `raw`; legacy chat: `-raw`; channel/supergroup: `-(raw + 1'000'000'000'000)` (i.e. `-100…` prefix). Peer-type detection via `peer->isUser()` / `peer->isChat()` / `peer->isChannel()` (covers supergroups too) |
| Internal URL handler registry | `core/local_url_handlers.cpp` | `InternalUrlHandlers()` returns `static std::vector<LocalUrlHandler>` (~1764). Each entry is `{regex, Fn<bool(SessionController*, Match*, QVariant)>}`. Matched via `Application::openCustomUrl("internal:", ...)` at `core/application.cpp:1184`. Add fork handlers by patching this list |

## Self-update rule

When research uncovers a mapping not in this table, **append a new row** before completing the task. The reference is meant to grow.

## Common dead ends

- Searching for a "renderer" by class name when tdesktop names them `Element`, `Message`, `Service`, `Reply`. The Element hierarchy underpins everything message-shaped — start there.
- Assuming a `Settings::*` pref helper exists for non-bool — only `bool` (and `QString` after `misc__pref-string`) have specializations.
- Relying on `Core::App().enumerateWindows` — it's private. Use `activeWindow()`.
- Looking for a `pubsub` / signal — tdesktop uses `rpl` everywhere. Look for `rpl::producer<>`, `rpl::event_stream<>`, `rpl::on_next(callback, lifetime)`.
