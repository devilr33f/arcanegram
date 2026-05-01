#include "arcanegram/ui/ag_hidden_users_settings.h"

#include "arcanegram/features/ag_hidden_users.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "core/application.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "lang/lang_keys.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::HiddenUsers {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

[[nodiscard]] PeerData *AnyLoadedPeer(PeerId id) {
    for (const auto &entry : Core::App().domain().accounts()) {
        if (const auto session = entry.account->maybeSession()) {
            if (const auto peer = session->data().peerLoaded(id)) {
                return peer;
            }
        }
    }
    return nullptr;
}

void Rebuild(not_null<Ui::VerticalLayout*> container) {
    while (container->count()) {
        delete container->widgetAt(0);
    }
    const auto &set = List();
    if (set.empty()) {
        container->add(
            object_ptr<Ui::FlatLabel>(
                container,
                tr::ag_hidden_users_empty(),
                st::boxDividerLabel),
            st::defaultBoxDividerLabelPadding);
        return;
    }
    for (const auto &id : set) {
        const auto peer = AnyLoadedPeer(id);
        const auto label = peer
            ? peer->name()
            : QString::number(id.value);
        const auto row = container->add(
            object_ptr<Ui::SettingsButton>(
                container,
                rpl::single(label),
                st::settingsButtonNoIcon));
        row->setClickedCallback([id] { Unhide(id); });
    }
}

class Page : public Section<Page> {
public:
    Page(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_hidden_users_title();
    }

private:
    void setupContent();
};

void BuildPage(SectionBuilder &builder) {
    const auto outer = builder.container();
    const auto inner = outer->add(object_ptr<Ui::VerticalLayout>(outer));
    Rebuild(inner);
    Changes(
    ) | rpl::on_next([=](PeerId) {
        Rebuild(inner);
    }, inner->lifetime());
    builder.addDividerText(tr::ag_hidden_users_info());
}

const auto kPageMeta = BuildHelper({
    .id = Page::Id(),
    .parentId = Arcanegram::Settings::Id(),
    .title = &tr::ag_hidden_users_title,
    .icon = &st::menuIconStealth,
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
        .title = tr::ag_hidden_users_title(),
        .targetSection = Page::Id(),
        .icon = { &st::menuIconStealth },
    });
}

} // namespace Arcanegram::HiddenUsers
