#include "arcanegram/ui/ag_hidden_users_settings.h"

#include "arcanegram/features/ag_hidden_users.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "core/application.h"
#include "data/data_peer.h"
#include "data/data_peer_id.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "lang/lang_keys.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "styles/style_arcanegram.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
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

void RebuildUsers(not_null<Ui::VerticalLayout*> container) {
    while (container->count()) {
        delete container->widgetAt(0);
    }
    const auto &set = List();
    if (set.empty()) {
        Ui::AddDividerText(container, tr::ag_hidden_users_empty());
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

void RebuildBots(not_null<Ui::VerticalLayout*> container) {
    while (container->count()) {
        delete container->widgetAt(0);
    }
    const auto &set = BotList();
    if (set.empty()) {
        Ui::AddDividerText(container, tr::ag_hidden_bots_empty());
        return;
    }
    for (const auto &id : set) {
        const auto label = BotDisplay(id);
        const auto row = container->add(
            object_ptr<Ui::SettingsButton>(
                container,
                rpl::single(label),
                st::settingsButtonNoIcon));
        row->setClickedCallback([id] { UnhideBot(id); });
    }
}

void RebuildRegexes(not_null<Ui::VerticalLayout*> container) {
    while (container->count()) {
        delete container->widgetAt(0);
    }
    const auto patterns = RegexList();
    if (patterns.isEmpty()) {
        Ui::AddDividerText(container, tr::ag_hidden_regexes_info());
        return;
    }
    for (auto i = 0; i != patterns.size(); ++i) {
        const auto pattern = patterns[i];
        const auto row = container->add(
            object_ptr<Ui::SettingsButton>(
                container,
                rpl::single(pattern),
                st::settingsButtonNoIcon));
        row->setClickedCallback([i] {
            auto list = RegexList();
            if (i >= 0 && i < list.size()) {
                list.removeAt(i);
                SetRegexList(std::move(list));
            }
        });
    }
}

void BuildPage(SectionBuilder &builder) {
    const auto outer = builder.container();

    Ui::AddSubsectionTitle(outer, tr::ag_hidden_users_section());
    const auto users = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildUsers(users);
    Changes(
    ) | rpl::on_next([=](PeerId) {
        RebuildUsers(users);
    }, users->lifetime());
    Ui::AddDividerText(outer, tr::ag_hidden_users_info());

    Ui::AddSubsectionTitle(outer, tr::ag_hidden_bots_section());
    const auto bots = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildBots(bots);
    BotChanges(
    ) | rpl::on_next([=] {
        RebuildBots(bots);
    }, bots->lifetime());
    Ui::AddDividerText(outer, tr::ag_hidden_bots_info());

    Ui::AddSubsectionTitle(outer, tr::ag_hidden_regexes_section());
    const auto regexes = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildRegexes(regexes);
    RegexChanges(
    ) | rpl::on_next([=] {
        RebuildRegexes(regexes);
    }, regexes->lifetime());

    const auto field = outer->add(
        object_ptr<Ui::InputField>(
            outer,
            st::agFastMessageField,
            tr::ag_hidden_regexes_placeholder(),
            QString()),
        st::settingsBioMargins);
    field->submits(
    ) | rpl::on_next([field](auto) {
        const auto text = field->getLastText().trimmed();
        if (text.isEmpty()) {
            return;
        }
        auto list = RegexList();
        if (!list.contains(text)) {
            list.append(text);
            SetRegexList(std::move(list));
        }
        field->setText(QString());
    }, field->lifetime());

    ::Settings::AddButtonWithIcon(
        outer,
        tr::ag_hidden_regexes_add(),
        st::settingsButton,
        { &st::menuIconAdd }
    )->setClickedCallback([field] {
        const auto text = field->getLastText().trimmed();
        if (text.isEmpty()) {
            return;
        }
        auto list = RegexList();
        if (!list.contains(text)) {
            list.append(text);
            SetRegexList(std::move(list));
        }
        field->setText(QString());
    });

    Ui::AddSkip(outer);
    Ui::AddDividerText(outer, tr::ag_hidden_content_info());
}

class Page : public Section<Page> {
public:
    Page(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_hidden_content_title();
    }

private:
    void setupContent();
};

const auto kPageMeta = BuildHelper({
    .id = Page::Id(),
    .parentId = Arcanegram::Settings::Id(),
    .title = &tr::ag_hidden_content_title,
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
        .title = tr::ag_hidden_content_title(),
        .targetSection = Page::Id(),
        .icon = { &st::menuIconStealth },
    });
}

} // namespace Arcanegram::HiddenUsers
