#include "arcanegram/features/ag_sync_feature.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/sync/ag_sync_engine.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "settings/settings_builder.h"

#include <rpl/map.h>
#include <rpl/producer.h>

// todo: replace hardcoded strings with tr::ag_sync_* once the misc__lang-keys
// stgit patch is updated to declare them.

namespace Arcanegram::CloudSync {
namespace {

QString FormatStatus(const Sync::StatusInfo &info) {
    switch (info.status) {
    case Sync::Status::Disabled:
        return u"disabled"_q;
    case Sync::Status::Idle:
        return info.lastSyncedAt
            ? u"synced"_q
            : u"ready"_q;
    case Sync::Status::Syncing:
        return u"syncing…"_q;
    case Sync::Status::Offline:
        return info.pendingCount
            ? u"offline · %1 pending"_q.arg(info.pendingCount)
            : (info.message.isEmpty() ? u"offline"_q : info.message);
    case Sync::Status::Conflict:
        return u"resolving conflict…"_q;
    }
    return {};
}

} // namespace

void Setup(::Settings::Builder::SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/sync_enabled"_q,
        rpl::single(u"cloud sync (experimental)"_q),
        rpl::single(u"sync arcanegram-only settings between your devices via the arcanesync backend."_q),
        Config::Sync::Enabled);

    Arcanegram::Settings::AddTextRow(
        builder,
        rpl::single(u"endpoint"_q),
        Config::Sync::Endpoint);

    auto statusText = [] -> rpl::producer<QString> {
        if (auto *e = Sync::Engine::Instance()) {
            return e->status() | rpl::map(&FormatStatus);
        }
        return rpl::single(QString());
    }();
    Arcanegram::Settings::AddStatusRow(builder, std::move(statusText));
}

} // namespace Arcanegram::CloudSync
