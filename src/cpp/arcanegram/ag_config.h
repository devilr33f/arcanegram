#pragma once

#include "base/assertion.h"

#include <string_view>
#include <QtCore/QString>
#include <rpl/event_stream.h>
#include <rpl/producer.h>

namespace Arcanegram::Config {

template <typename T>
class Item {
public:
    Item(std::string_view key, T defaultValue);

    [[nodiscard]] T value() const;
    void setValue(T v);
    [[nodiscard]] rpl::producer<T> changes() const;

private:
    std::string_view _key;
    T _default;
    rpl::event_stream<T> _changes;
};

class BoolItem : public Item<bool> {
public:
    using Item::Item;
    void toggle() { setValue(!value()); }
};

extern template class Item<bool>;
extern template class Item<QString>;

namespace ForwardedHeader {

inline auto ShowDate = BoolItem("ag_forwarded_show_date", false);

} // namespace ForwardedHeader

namespace ShowSeconds {

inline auto Enabled = BoolItem("ag_show_seconds", false);

} // namespace ShowSeconds

namespace HiddenUsers {

inline auto Stored = Item<QString>("ag_hidden_users", QString());

} // namespace HiddenUsers

namespace ChatWallpaper {

inline auto Disabled = BoolItem("ag_chat_wallpaper_disabled", false);

} // namespace ChatWallpaper

namespace ScreenshotMode {

inline auto Enabled = BoolItem("ag_streamer_enabled", false);
inline auto AnonymizeBots = BoolItem("ag_streamer_anonymize_bots", false);

} // namespace ScreenshotMode

namespace Sync {

inline auto Enabled = BoolItem("ag_sync_enabled", false);
inline auto Endpoint = Item<QString>(
    "ag_sync_endpoint",
    QString::fromUtf8("https://arcane.femboy.page"));
inline auto Bot = Item<QString>(
    "ag_sync_bot",
    QString::fromUtf8("arcanesync_bot"));

} // namespace Sync

namespace FastMessages {

constexpr int kSlotCount = 10;

inline auto CountStr = Item<QString>("ag_fast_messages_count", u"1"_q);

[[nodiscard]] inline int Count() {
    const auto v = CountStr.value().toInt();
    return (v >= 1 && v <= kSlotCount) ? v : 1;
}
inline void SetCount(int n) {
    CountStr.setValue(QString::number(n));
}

inline auto Slot1 = Item<QString>("ag_fast_message_text_1", QString());
inline auto Slot2 = Item<QString>("ag_fast_message_text_2", QString());
inline auto Slot3 = Item<QString>("ag_fast_message_text_3", QString());
inline auto Slot4 = Item<QString>("ag_fast_message_text_4", QString());
inline auto Slot5 = Item<QString>("ag_fast_message_text_5", QString());
inline auto Slot6 = Item<QString>("ag_fast_message_text_6", QString());
inline auto Slot7 = Item<QString>("ag_fast_message_text_7", QString());
inline auto Slot8 = Item<QString>("ag_fast_message_text_8", QString());
inline auto Slot9 = Item<QString>("ag_fast_message_text_9", QString());
inline auto Slot10 = Item<QString>("ag_fast_message_text_10", QString());

[[nodiscard]] inline Item<QString> &Slot(int index) {
    switch (index) {
    case 0: return Slot1;
    case 1: return Slot2;
    case 2: return Slot3;
    case 3: return Slot4;
    case 4: return Slot5;
    case 5: return Slot6;
    case 6: return Slot7;
    case 7: return Slot8;
    case 8: return Slot9;
    case 9: return Slot10;
    }
    Unexpected("FastMessages::Slot index out of range");
}

inline void RemoveSlot(int index) {
    const auto count = Count();
    for (auto i = index; i < count - 1; ++i) {
        Slot(i).setValue(Slot(i + 1).value());
    }
    Slot(count - 1).setValue(QString());
    SetCount(count - 1);
}

} // namespace FastMessages

namespace QuickAccess {

inline auto SidebarScreenshot = BoolItem("ag_quick_sidebar_screenshot", false);

} // namespace QuickAccess

} // namespace Arcanegram::Config
