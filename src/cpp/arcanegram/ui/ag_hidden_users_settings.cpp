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

not_null<Ui::RpWidget*> MakeRow(
        not_null<Ui::VerticalLayout*> container,
        rpl::producer<QString> label,
        Fn<void()> remove) {
    const auto row = container->add(
        object_ptr<Ui::SettingsButton>(
            container,
            std::move(label),
            st::settingsButtonNoIcon));
    const auto cross = Ui::CreateChild<Ui::IconButton>(
        row,
        st::sessionTerminate);
    cross->setClickedCallback(std::move(remove));
    row->sizeValue() | rpl::on_next([=](QSize size) {
        cross->moveToRight(
            st::settingsButton.padding.right(),
            (size.height() - cross->height()) / 2);
    }, cross->lifetime());
    return row;
}

template <typename Range, typename MakeLabel, typename MakeRemove>
void RebuildSection(
        not_null<Ui::VerticalLayout*> container,
        const Range &items,
        rpl::producer<QString> emptyHint,
        MakeLabel makeLabel,
        MakeRemove makeRemove) {
    while (container->count()) {
        delete container->widgetAt(0);
    }
    if (std::ranges::empty(items)) {
        Ui::AddDividerText(container, std::move(emptyHint));
        return;
    }
    auto i = 0;
    for (const auto &entry : items) {
        MakeRow(
            container,
            rpl::single(makeLabel(entry)),
            makeRemove(entry, i));
        ++i;
    }
}

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
    const auto &set = List();
    RebuildSection(
        container,
        set,
        tr::ag_hidden_users_empty(),
        [](PeerId id) {
            const auto peer = AnyLoadedPeer(id);
            return peer ? peer->name() : QString::number(id.value);
        },
        [](PeerId id, int) -> Fn<void()> {
            return [id] { Unhide(id); };
        });
}

void RebuildBots(not_null<Ui::VerticalLayout*> container) {
    const auto &set = BotList();
    RebuildSection(
        container,
        set,
        tr::ag_hidden_bots_empty(),
        [](UserId id) { return BotDisplay(id); },
        [](UserId id, int) -> Fn<void()> {
            return [id] { UnhideBot(id); };
        });
}

void RebuildRegexes(not_null<Ui::VerticalLayout*> container) {
    const auto patterns = RegexList();
    RebuildSection(
        container,
        patterns,
        tr::ag_hidden_regexes_info(),
        [](const QString &pattern) { return pattern; },
        [](const QString &, int index) -> Fn<void()> {
            return [index] {
                auto list = RegexList();
                if (index >= 0 && index < list.size()) {
                    list.removeAt(index);
                    SetRegexList(std::move(list));
                }
            };
        });
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
