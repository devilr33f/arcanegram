#pragma once

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

} // namespace Arcanegram::Config
