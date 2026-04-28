# Inugram patch reference

Snapshot of [teidesu/inugram](https://github.com/teidesu/inugram) — Telegram **Android** patchset that arcanegram is modelled on. Use this as a brainstorm catalogue when looking for features to port to tdesktop.

**Caveat.** Inugram patches are Kotlin/Android. Their *implementations* don't transfer; only the user-facing capability does. Mark column meanings:

- 🖥 — concept maps cleanly to tdesktop (UI/behaviour also exists on desktop, port is mostly fresh code)
- ❓ — uncertain (mobile-specific UX or unknown desktop equivalent — investigate before promising)
- 📱 — android-only (predictive back, IME, RecyclerListView, attach drawer mechanics, etc. — skip)

Total: 95 patches at upstream `009e9735…` (cloned 2026-04-28). Keep this file in sync only when consciously porting; we don't need to track inugram day-to-day.

## misc — 6

| Patch | Mark | Summary |
| --- | --- | --- |
| `misc/build-support` | 📱 | Kotlin + min-SDK + versioning bootstrap. Arcanegram has its own equivalent. |
| `misc/branding` | 🖥 | App name change. Arcanegram already has this. |
| `misc/remove-play` | 📱 | Remove Google Play Services bits. N/A on desktop. |
| `misc/settings-ui` | 🖥 | Settings entry point and section. Arcanegram already has this. |
| `misc/defaults` | 🖥 | Opinionated default settings. Easy to add per-toggle. |
| `misc/database` | ❓ | Fork-specific db migration hook. Desktop equivalent would be `Core::Settings` migrations. |

## hooks — 6

Thin call-site stubs so feature code has somewhere to attach. Each desktop port needs its own equivalent inside `Arcanegram::Hooks::*`.

| Patch | Mark | Summary |
| --- | --- | --- |
| `hooks/app-loader` | 📱 | Custom `ApplicationLoaderImpl` — Android startup. |
| `hooks/photo-viewer-menu` | 🖥 | Inject items into the photo-viewer context menu. |
| `hooks/chat-menu-items` | 🖥 | Inject items into the message long-press / context menu. |
| `hooks/internal-web-app` | ❓ | Internal web-app infra (ported from Nekogram). Desktop has its own webview pipeline. |
| `hooks/profile-menu` | 🖥 | Inject items into the profile / "more" menu. |
| `hooks/admin-logs` | ❓ | Hooks inside the admin-logs activity. Desktop has admin log too. |

## feature — 37

| Patch | Mark | Summary |
| --- | --- | --- |
| `feature/show-seconds` | 🖥 | Show seconds in timestamps. |
| `feature/all-recent-stickers` | 🖥 | Show all recent stickers (no truncation). |
| `feature/bottom-tabs` | 📱 | Compact / hide / remove bottom-tab bar — phone navigation only. |
| `feature/minimize-sticker-creator` | 🖥 | Toggle the sticker-creator button in the recents tab. |
| `feature/sticker-size` | 🖥 | Customisable sticker render size. |
| `feature/multi-account` | 🖥 | Raise account limit beyond stock cap. |
| `feature/autofill-hints` | 📱 | Password-manager autofill hints — Android `View` flags. |
| `feature/reset-pending-login` | 🖥 | Menu to reset a pending login session. |
| `feature/default-delete-for-both` | 🖥 | Configurable default for the "delete for everyone" checkbox. |
| `feature/hide-keyboard-on-scroll` | 📱 | IME-specific. Desktop has no on-screen keyboard. |
| `feature/call-confirmation` | 🖥 | Confirmation dialog before placing calls. |
| `feature/double-tap-delay` | ❓ | Double-tap timing tuning — likely desktop has its own threshold. |
| `feature/mutual-contact-icon` | 🖥 | Marker on rows for mutual contacts. |
| `feature/web-preview-replacements` | 🖥 | Rewrite/strip patterns in link previews. |
| `feature/jump-to-discussion` | 🖥 | Jump from comments to the discussion group root. |
| `feature/double-tap-action` | 🖥 | Configurable double-tap-on-message action. |
| `feature/id-in-profile` | 🖥 | Show numeric peer id in profile. |
| `feature/hide-spoiler-pending` | 🖥 | Toggle "hide with spoiler" inside the photo viewer. |
| `feature/reactions-menu` | ❓ | Visual tweaks for reactions bar in the message menu. |
| `feature/folders-display-mode` | 🖥 | Folder/tab layout customisations. |
| `feature/remember-all-replies` | 🖥 | Stack the "go-back to reply" button across multiple replies. |
| `feature/max-chat-input-lines` | 🖥 | Customise the max-line count for the composer. |
| `feature/monet-theme` | 📱 | Material You / Monet — Android system colour pull. |
| `feature/ios-menu-gesture` | 📱 | iOS-style press-and-release gesture for menus. |
| `feature/old-mention-indicator` | ❓ | Restore the pre-12.6 mention/reaction indicator visuals. |
| `feature/dialogs-fab` | 📱 | Floating-action-button tweaks — phone idiom. |
| `feature/non-island-ui` | 📱 | Remove the "island" / floating containers — phone layout. |
| `feature/customize-text-classifier` | 📱 | Android `TextClassifier` tuning. |
| `feature/seen-view-below` | 🖥 | Move "seen by" entry to the bottom of the menu. |
| `feature/custom-formatting-ui` | 🖥 | Improved rich-text formatting popup. |
| `feature/long-press-reply-in-replies` | 🖥 | In the "Replies" chat, long-press on the reply panel jumps to the discussion thread root. |
| `feature/animation-multiplier` | 🖥 | Global animation-speed multiplier. |
| `feature/search-media-type-filter` | 🖥 | Media-type filter + "matches only" toggle in chat search. |
| `feature/no-join-discuss` | 🖥 | Send to discussion group without forced join. |
| `feature/voice-recorder` | 📱 | Move voice-record button into the attach drawer. |
| `feature/hide-pinned-panel` | 🖥 | Per-chat toggle to hide the pinned-message panel. |
| `feature/hide-my-phone` | 🖥 | Hide the current user's phone number from the UI. |

## debloat — 28

Each is a setting toggle that suppresses stock behaviour. Desktop port = wire the toggle, then guard the existing tdesktop code path.

| Patch | Mark | Summary |
| --- | --- | --- |
| `debloat/hide-trending-stickers` | 🖥 | Hide trending stickers / emoji packs in the picker. |
| `debloat/disable-predictive-back` | 📱 | Android 13+ predictive-back gesture. |
| `debloat/disable-instant-camera` | 📱 | Camera entry inside attach view. |
| `debloat/disable-rounding` | 🖥 | Disable "1.2K"-style number rounding in counts. |
| `debloat/hide-ai-features` | 🖥 | Hide AI / assistant entry points. |
| `debloat/hide-sticker-time` | 🖥 | Hide the time overlay drawn on stickers. |
| `debloat/disable-pull-to-next` | 📱 | Pull-to-next-channel gesture on phone. |
| `debloat/disable-sensitive` | 🖥 | Disable client-side sensitive-content filter. |
| `debloat/disable-swipe-to-unarchive` | 📱 | Phone swipe gesture on dialogs list. |
| `debloat/hide-paid-reaction-upsell` | 🖥 | Suppress paid-reaction promo (particles, gradient, empty-state). |
| `debloat/always-show-go-to-down` | 🖥 | Always render the "scroll to bottom" button. |
| `debloat/hide-voice-hint` | 🖥 | Show the voice/video toggle hint only once. |
| `debloat/hide-bot-elements` | 🖥 | Hide bot commands/webview buttons in chat header. |
| `debloat/intro-sticker` | ❓ | Greeting / intro-sticker tweaks — partially desktop-relevant. |
| `debloat/hide-fade-views` | ❓ | Fade overlays — needs investigation if tdesktop has same effect. |
| `debloat/hide-stories` | 🖥 | Hide the stories section. |
| `debloat/hide-reactions-entry` | 🖥 | Suppress the "send reaction" animation entry. |
| `debloat/simple-attachalert-anim` | 📱 | Strip bouncy animation from `ChatAttachAlert`. |
| `debloat/hide-suggestions` | 🖥 | Suppress server-pushed suggestion strips throughout the app. |
| `debloat/disable-chat-title-phone` | 🖥 | Don't substitute phone number for display name in chat title. |
| `debloat/disable-chat-themes` | 🖥 | Disable per-chat backgrounds / themes. |
| `debloat/recyclerview-instant-tap` | 📱 | Android-specific tap latency setting. |
| `debloat/opt-in-motion-photos` | 📱 | "Motion photos" in attach picker — Android camera feature. |
| `debloat/chat-hide-bottom-bar` | 🖥 | Hide channel mute/join bottom bar. |
| `debloat/disable-bg-parallax` | 📱 | Gyro-driven wallpaper parallax — phone sensor. |
| `debloat/cloud-drafts` | 🖥 | Don't upload drafts to cloud. |
| `debloat/tap-to-open-preview` | ❓ | Prevent expanding chat preview on tap. Need to find desktop equivalent. |
| `debloat/notification-bubbles` | 📱 | Android notification-bubble feature. |

## bugfix — 18

Almost all of these are fixing Android-specific bugs (RecyclerListView, IME jump, attach panel, photo-viewer LiteMode, face-detect, chromecast). Only port if you actually observe the same bug on desktop.

| Patch | Mark | Summary |
| --- | --- | --- |
| `bugfix/keyboard-image-paste` | 📱 | Gboard image paste bypassing PhotoViewer. |
| `bugfix/recyclerlist-double-tap` | 📱 | RecyclerListView double-tap dispatch. |
| `bugfix/dialogs-list-scrolling` | 📱 | Pull-to-reveal-archive scroll glitches. |
| `bugfix/shared-media-player` | 🖥 | Visual glitches in the shared-media player — possibly relevant to tdesktop's media viewer. |
| `bugfix/paid-reaction-animation` | 📱 | Disabled via Android `LiteMode` flag. |
| `bugfix/attach-panel-perf` | 📱 | Attach-sheet performance. |
| `bugfix/attach-panel-close` | 📱 | Stuck-on-close while half-open. |
| `bugfix/clickable-behind-bottom-bar` | 📱 | Click-through area beside bottom bar. |
| `bugfix/reaction-counter-jump` | ❓ | Reaction counter shifting during long-press menu. |
| `bugfix/message-send-ime-jump` | 📱 | IME height change during send animation. |
| `bugfix/formatting-regular` | 🖥 | "Regular" option in formatting popup — partial-selection fix. Likely also affects tdesktop. |
| `bugfix/photo-viewer-blur` | 📱 | Photo viewer LiteMode blur respect. |
| `bugfix/lazy-face-detect` | 📱 | Face detect should run on demand, not eagerly. |
| `bugfix/lazy-chromecast` | 📱 | Lazy Chromecast init in photo viewer. |
| `bugfix/missing-replace-emoji` | ❓ | `Emoji.replaceEmoji` calls missing in spots — desktop has its own emoji pipeline. |
| `bugfix/media-loading-perf` | 📱 | Experimental Android media-loading tweak. |
| `bugfix/photo-spoiler-power-saving` | 📱 | Power-saving + animated spoiler interaction. |
| `bugfix/shared-media-spoiler` | ❓ | Spoiler positioning in shared-media tab. |

## High-leverage candidates

If picking a first batch to port for arcanegram, sort by user-visible impact and implementation simplicity:

- **Easy + high-impact:** `show-seconds`, `disable-rounding`, `id-in-profile`, `hide-stories`, `hide-suggestions`, `hide-ai-features`, `disable-sensitive`, `cloud-drafts`, `default-delete-for-both`, `hide-my-phone`.
- **Useful but more invasive:** `multi-account` (raise stock cap), `web-preview-replacements`, `search-media-type-filter`, `custom-formatting-ui`, `remember-all-replies`.
- **Hooks-first (enable later patches):** `hooks/chat-menu-items`, `hooks/photo-viewer-menu`, `hooks/profile-menu`.
