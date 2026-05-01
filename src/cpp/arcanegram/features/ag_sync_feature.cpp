#include "arcanegram/features/ag_sync_feature.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/sync/ag_sync_engine.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_menu_icons.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

#include <rpl/map.h>
#include <rpl/producer.h>

namespace Arcanegram::CloudSync {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

QString FormatStatus(const Sync::StatusInfo &info) {
    switch (info.status) {
    case Sync::Status::Disabled:
        return tr::ag_sync_status_disabled(tr::now);
    case Sync::Status::Idle:
        return info.lastSyncedAt
            ? tr::ag_sync_status_synced(tr::now)
            : tr::ag_sync_status_ready(tr::now);
    case Sync::Status::Syncing:
        return tr::ag_sync_status_syncing(tr::now);
    case Sync::Status::Offline: {
        const auto base = info.pendingCount
            ? tr::ag_sync_status_offline_pending(
                tr::now,
                lt_n,
                QString::number(info.pendingCount))
            : tr::ag_sync_status_offline(tr::now);
        return info.message.isEmpty() ? base : (base + u" · "_q + info.message);
    }
    case Sync::Status::Conflict:
        return tr::ag_sync_status_conflict(tr::now);
    }
    return {};
}

class Page : public Section<Page> {
public:
    Page(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_settings_sync_title();
    }

private:
    void setupContent();
};

void BuildPage(SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/sync_enabled"_q,
        tr::ag_sync_enabled_setting(),
        tr::ag_sync_enabled_info(),
        Config::Sync::Enabled);

    Arcanegram::Settings::AddTextRow(
        builder,
        tr::ag_sync_endpoint_placeholder(),
        Config::Sync::Endpoint);

    auto statusText = []() -> rpl::producer<QString> {
        if (auto *e = Sync::Engine::Instance()) {
            return e->status() | rpl::map(&FormatStatus);
        }
        return rpl::single(QString());
    }();
    Arcanegram::Settings::AddStatusRow(builder, std::move(statusText));
}

const auto kPageMeta = BuildHelper({
    .id = Page::Id(),
    .parentId = Arcanegram::Settings::Id(),
    .title = &tr::ag_settings_sync_title,
    .icon = &st::menuIconLink,
}, [](SectionBuilder &builder) {
    BuildPage(builder);
});

const SectionBuildMethod kPageSection = kPageMeta.build;

Page::Page(
    QWidget *parent,
    not_null<Window::SessionController*> controller)
: Section(parent, controller) {
    setupContent();
}

void Page::setupContent() {
    const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
    build(content, kPageSection);
    Ui::ResizeFitChild(this, content);
}

} // namespace

void Setup(::Settings::Builder::SectionBuilder &builder) {
    builder.addSectionButton({
        .title = tr::ag_settings_sync_title(),
        .targetSection = Page::Id(),
        .icon = { &st::menuIconLink },
    });
}

} // namespace Arcanegram::CloudSync
