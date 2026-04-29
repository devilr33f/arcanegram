#include "arcanegram/sync/ag_sync_keys.h"

#include "arcanegram/ag_config.h"

namespace Arcanegram::Sync {
namespace {

thread_local bool t_applyingFromCloud = false;

struct ApplyingScope {
    ApplyingScope() { t_applyingFromCloud = true; }
    ~ApplyingScope() { t_applyingFromCloud = false; }
};

template <typename T>
bool ApplyTyped(Config::Item<T> &item, const QJsonValue &value);

template <>
bool ApplyTyped<bool>(Config::Item<bool> &item, const QJsonValue &value) {
    if (!value.isBool()) return false;
    item.setValue(value.toBool());
    return true;
}

template <>
bool ApplyTyped<QString>(Config::Item<QString> &item, const QJsonValue &value) {
    if (!value.isString()) return false;
    item.setValue(value.toString());
    return true;
}

} // namespace

bool ApplyToLocal(const QString &key, const QJsonValue &value) {
    ApplyingScope scope;
    if (key == u"ag_forwarded_show_date"_q)
        return ApplyTyped(Config::ForwardedHeader::ShowDate, value);
    if (key == u"ag_show_seconds"_q)
        return ApplyTyped(Config::ShowSeconds::Enabled, value);
    if (key == u"ag_hidden_users"_q)
        return ApplyTyped(Config::HiddenUsers::Stored, value);
    if (key == u"ag_chat_wallpaper_disabled"_q)
        return ApplyTyped(Config::ChatWallpaper::Disabled, value);
    return false;
}

QJsonObject SnapshotLocal() {
    return {
        {u"ag_forwarded_show_date"_q, Config::ForwardedHeader::ShowDate.value()},
        {u"ag_show_seconds"_q, Config::ShowSeconds::Enabled.value()},
        {u"ag_hidden_users"_q, Config::HiddenUsers::Stored.value()},
        {u"ag_chat_wallpaper_disabled"_q, Config::ChatWallpaper::Disabled.value()},
    };
}

bool IsApplyingFromCloud() { return t_applyingFromCloud; }

} // namespace Arcanegram::Sync
