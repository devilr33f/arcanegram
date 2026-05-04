#include "arcanegram/features/ag_fast_messages.h"

#include "apiwrap.h"
#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "base/invoke_queued.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "styles/style_arcanegram.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::FastMessages {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

void FillSlots(not_null<Ui::VerticalLayout*> wrap, not_null<Ui::VerticalLayout*> outer) {
    const auto count = Config::FastMessages::Count();
    for (auto i = 0; i < count; ++i) {
        auto &item = Config::FastMessages::Slot(i);
        Ui::AddSubsectionTitle(
            wrap,
            tr::ag_fast_messages_slot(
                lt_index,
                rpl::single(QString::number(i + 1))));
        const auto field = wrap->add(
            object_ptr<Ui::InputField>(
                wrap,
                st::agFastMessageField,
                tr::ag_fast_messages_placeholder(),
                item.value()),
            st::settingsBioMargins);
        field->changes(
        ) | rpl::on_next([field, &item](auto) {
            const auto v = field->getLastText();
            if (v != item.value()) {
                item.setValue(v);
            }
        }, field->lifetime());
        const auto idx = i;
        ::Settings::AddButtonWithIcon(
            wrap,
            tr::ag_fast_messages_remove(),
            st::settingsButton,
            { &st::menuIconDelete }
        )->setClickedCallback([outer, idx] {
            Config::FastMessages::RemoveSlot(idx);
            outer->resizeToWidth(outer->width());
        });
        Ui::AddSkip(wrap);
        Ui::AddDivider(wrap);
    }
}

class Page : public Section<Page> {
public:
    Page(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_fast_messages_title();
    }

private:
    void setupContent();
};

void BuildPage(SectionBuilder &builder) {
    const auto outer = builder.container();

    const auto slotWrap = outer->add(object_ptr<Ui::VerticalLayout>(outer));
    FillSlots(slotWrap, outer);

    Config::FastMessages::CountStr.changes(
    ) | rpl::on_next([outer, slotWrap](auto) {
        InvokeQueued(slotWrap, [outer, slotWrap] {
            while (slotWrap->count()) {
                delete slotWrap->widgetAt(0);
            }
            FillSlots(slotWrap, outer);
            slotWrap->resizeToWidth(slotWrap->width());
        });
    }, slotWrap->lifetime());

    Ui::AddSkip(outer);
    ::Settings::AddButtonWithIcon(
        outer,
        tr::ag_fast_messages_add(),
        st::settingsButton,
        { &st::menuIconAdd }
    )->setClickedCallback([outer] {
        const auto count = Config::FastMessages::Count();
        if (count >= Config::FastMessages::kSlotCount) {
            return;
        }
        Config::FastMessages::SetCount(count + 1);
    });

    Ui::AddSkip(outer);
    Ui::AddDividerText(outer, tr::ag_fast_messages_info());
}

const auto kPageMeta = BuildHelper({
    .id = Page::Id(),
    .parentId = Arcanegram::Settings::Id(),
    .title = &tr::ag_fast_messages_title,
    .icon = &st::menuIconReply,
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

void Init() {
}

bool Send(
        not_null<PeerData*> peer,
        FullReplyTo replyTo,
        int slot) {
    if (slot < 0 || slot >= Config::FastMessages::Count()) {
        return false;
    }
    const auto text = Config::FastMessages::Slot(slot).value().trimmed();
    if (text.isEmpty()) {
        return false;
    }
    const auto history = peer->owner().history(peer);
    auto action = Api::SendAction(history);
    action.replyTo = replyTo;
    auto message = Api::MessageToSend(action);
    message.textWithTags = { text, TextWithTags::Tags() };
    peer->session().api().sendMessage(std::move(message));
    return true;
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
    builder.addSectionButton({
        .title = tr::ag_fast_messages_title(),
        .targetSection = Page::Id(),
        .icon = { &st::menuIconReply },
    });
}

} // namespace Arcanegram::FastMessages
