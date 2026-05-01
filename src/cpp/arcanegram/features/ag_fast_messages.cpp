#include "arcanegram/features/ag_fast_messages.h"

#include "apiwrap.h"
#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_arcanegram.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/vertical_list.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::FastMessages {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

void AddSlotField(
        not_null<Ui::VerticalLayout*> container,
        int index) {
    auto &item = Config::FastMessages::Slot(index);
    Ui::AddSubsectionTitle(
        container,
        tr::ag_fast_messages_slot(
            lt_index,
            rpl::single(QString::number(index + 1))));
    const auto field = container->add(
        object_ptr<Ui::InputField>(
            container,
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
    const auto container = builder.container();
    for (auto i = 0; i != Config::FastMessages::kSlotCount; ++i) {
        AddSlotField(container, i);
    }
    Ui::AddSkip(container);
    Ui::AddDividerText(container, tr::ag_fast_messages_info());
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
    if (slot < 0 || slot >= Config::FastMessages::kSlotCount) {
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
