#include "arcanegram/ui/ag_hidden_users_settings.h"

#include "apiwrap.h"
#include "arcanegram/features/ag_hidden_users.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "core/application.h"
#include "data/data_changes.h"
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
#include "styles/style_boxes.h"
#include "styles/style_color_indices.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/controls/userpic_button.h"
#include "ui/empty_userpic.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::HiddenUsers {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

using MakeLeading = Fn<object_ptr<Ui::RpWidget>(not_null<QWidget*>)>;

[[nodiscard]] object_ptr<Ui::RpWidget> MakePlaceholderCircle(
        not_null<QWidget*> parent,
        uint64 colorSeed,
        const QString &glyph) {
    const auto colorIndex = Ui::EmptyUserpic::ColorIndex(colorSeed);
    const auto colors = Ui::EmptyUserpic::UserpicColor(colorIndex);
    auto widget = object_ptr<Ui::RpWidget>(parent);
    const auto raw = widget.data();
    constexpr auto kSize = 32;
    raw->resize(kSize, kSize);
    const auto userpic = raw->lifetime().make_state<Ui::EmptyUserpic>(
        colors,
        glyph);
    raw->paintRequest() | rpl::on_next([=](QRect) {
        auto p = QPainter(raw);
        userpic->paintCircle(p, 0, 0, kSize, kSize);
    }, raw->lifetime());
    return widget;
}

not_null<Ui::RpWidget*> MakeRow(
        not_null<Ui::VerticalLayout*> container,
        MakeLeading makeLeading,
        rpl::producer<QString> label,
        Fn<void()> remove) {
    const auto row = container->add(
        object_ptr<Ui::SettingsButton>(
            container,
            std::move(label),
            st::settingsButton));
    row->setAttribute(Qt::WA_TransparentForMouseEvents);

    const auto leading = makeLeading(row).release();
    leading->setAttribute(Qt::WA_TransparentForMouseEvents);

    const auto cross = Ui::CreateChild<Ui::IconButton>(
        row,
        st::agHiddenRowRemove);
    cross->setClickedCallback(std::move(remove));

    constexpr auto kLeadingSize = 32;
    row->sizeValue() | rpl::on_next([=](QSize size) {
        const auto lx = (st::settingsButton.padding.left() - kLeadingSize) / 2;
        const auto ly = (size.height() - kLeadingSize) / 2;
        leading->move(lx, ly);
        cross->moveToRight(
            st::settingsButton.padding.right(),
            (size.height() - cross->height()) / 2);
    }, cross->lifetime());
    return row;
}

template <
    typename Range,
    typename MakeLabel,
    typename MakeRemove,
    typename MakeLeadingForEntry>
void RebuildSection(
        not_null<Ui::VerticalLayout*> container,
        const Range &items,
        rpl::producer<QString> emptyHint,
        MakeLabel makeLabel,
        MakeRemove makeRemove,
        MakeLeadingForEntry makeLeadingForEntry) {
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
            makeLeadingForEntry(entry),
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
        },
        [](PeerId id) -> MakeLeading {
            if (const auto peer = AnyLoadedPeer(id)) {
                return [peer](not_null<QWidget*> parent)
                        -> object_ptr<Ui::RpWidget> {
                    return object_ptr<Ui::UserpicButton>(
                        parent,
                        peer,
                        st::uploadUserpicButton);
                };
            }
            return [id](not_null<QWidget*> parent) {
                return MakePlaceholderCircle(parent, id.value, u"?"_q);
            };
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
        },
        [](UserId id) -> MakeLeading {
            return [id](not_null<QWidget*> parent) {
                return MakePlaceholderCircle(
                    parent,
                    id.bare,
                    u"@"_q);
            };
        });
}

void RebuildRegexes(not_null<Ui::VerticalLayout*> container) {
    const auto patterns = RegexList();
    RebuildSection(
        container,
        patterns,
        tr::ag_hidden_regexes_info(),
        [](const QString &pattern) { return pattern; },
        [](const QString &pattern, int) -> Fn<void()> {
            return [pattern] {
                auto list = RegexList();
                list.removeAll(pattern);
                SetRegexList(std::move(list));
            };
        },
        [](const QString &pattern) -> MakeLeading {
            const auto seed = uint64(qHash(pattern));
            return [seed](not_null<QWidget*> parent) {
                return MakePlaceholderCircle(parent, seed, u".*"_q);
            };
        });
}

void RequestUnloaded(not_null<Main::Session*> session) {
    QVector<MTPInputUser> inputs;
    for (const auto &id : List()) {
        if (!peerIsUser(id)) continue;
        if (session->data().peerLoaded(id)) continue;
        const auto bare = peerToBareMTPInt(peerToUser(id));
        inputs.push_back(MTP_inputUser(bare, MTPlong()));
    }
    if (inputs.isEmpty()) return;
    session->api().request(MTPusers_GetUsers(
        MTP_vector<MTPInputUser>(std::move(inputs))
    )).done([session](const MTPVector<MTPUser> &result) {
        session->data().processUsers(result);
    }).send();
}

void BuildPage(SectionBuilder &builder) {
    const auto outer = builder.container();

    {
        auto count = rpl::single(rpl::empty_value())
            | rpl::then(Changes() | rpl::to_empty)
            | rpl::map([] { return int(List().size()); });
        auto title = rpl::combine(
            tr::ag_hidden_users_section(),
            std::move(count)
        ) | rpl::map([](const QString &t, int n) {
            return n > 0 ? t + u" · "_q + QString::number(n) : t;
        });
        Ui::AddSubsectionTitle(outer, std::move(title));
    }
    const auto users = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildUsers(users);
    Changes(
    ) | rpl::on_next([=](PeerId) {
        RebuildUsers(users);
    }, users->lifetime());

    const auto session = builder.session();
    auto unresolved = std::make_shared<base::flat_set<PeerId>>();
    for (const auto &id : List()) {
        if (peerIsUser(id) && !session->data().peerLoaded(id)) {
            unresolved->emplace(id);
        }
    }
    if (!unresolved->empty()) {
        RequestUnloaded(session);
        session->changes().peerUpdates(
            Data::PeerUpdate::Flag::Name | Data::PeerUpdate::Flag::Photo
        ) | rpl::on_next([=](const Data::PeerUpdate &update) {
            if (unresolved->remove(update.peer->id)) {
                RebuildUsers(users);
            }
        }, users->lifetime());
    }

    {
        auto count = rpl::single(rpl::empty_value())
            | rpl::then(BotChanges())
            | rpl::map([] { return int(BotList().size()); });
        auto title = rpl::combine(
            tr::ag_hidden_bots_section(),
            std::move(count)
        ) | rpl::map([](const QString &t, int n) {
            return n > 0 ? t + u" · "_q + QString::number(n) : t;
        });
        Ui::AddSubsectionTitle(outer, std::move(title));
    }
    const auto bots = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildBots(bots);
    BotChanges(
    ) | rpl::on_next([=] {
        RebuildBots(bots);
    }, bots->lifetime());

    {
        auto count = rpl::single(rpl::empty_value())
            | rpl::then(RegexChanges())
            | rpl::map([] { return int(RegexList().size()); });
        auto title = rpl::combine(
            tr::ag_hidden_regexes_section(),
            std::move(count)
        ) | rpl::map([](const QString &t, int n) {
            return n > 0 ? t + u" · "_q + QString::number(n) : t;
        });
        Ui::AddSubsectionTitle(outer, std::move(title));
    }
    const auto regexes = outer->add(
        object_ptr<Ui::VerticalLayout>(outer));
    RebuildRegexes(regexes);
    RegexChanges(
    ) | rpl::on_next([=] {
        RebuildRegexes(regexes);
    }, regexes->lifetime());

    const auto adderWrap = outer->add(
        object_ptr<Ui::SlideWrap<Ui::InputField>>(
            outer,
            object_ptr<Ui::InputField>(
                outer,
                st::agFastMessageField,
                tr::ag_hidden_regexes_placeholder(),
                QString()),
            st::settingsBioMargins))->setDuration(150);
    adderWrap->hide(anim::type::instant);

    const auto field = adderWrap->entity();

    const auto addButton = ::Settings::AddButtonWithIcon(
        outer,
        tr::ag_hidden_regexes_add(),
        st::settingsButton,
        { &st::menuIconAdd });

    const auto submit = [=] {
        const auto text = field->getLastText().trimmed();
        if (text.isEmpty()) {
            adderWrap->hide(anim::type::normal);
            return;
        }
        auto list = RegexList();
        if (!list.contains(text)) {
            list.append(text);
            SetRegexList(std::move(list));
        }
        field->setText(QString());
        adderWrap->hide(anim::type::normal);
    };

    addButton->setClickedCallback([=] {
        if (adderWrap->isHidden()) {
            adderWrap->show(anim::type::normal);
            field->setFocus();
        } else {
            submit();
        }
    });

    field->submits(
    ) | rpl::on_next([=](auto) { submit(); }, field->lifetime());

    field->focusedChanges(
    ) | rpl::on_next([=](bool focused) {
        if (!focused && field->getLastText().trimmed().isEmpty()) {
            adderWrap->hide(anim::type::normal);
        }
    }, field->lifetime());

    field->cancelled(
    ) | rpl::on_next([=] {
        field->setText(QString());
        adderWrap->hide(anim::type::normal);
    }, field->lifetime());

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
