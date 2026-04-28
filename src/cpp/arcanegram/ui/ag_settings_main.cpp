#include "arcanegram/ui/ag_settings_main.h"

#include "arcanegram/features/ag_forwarded_header.h"
#include "arcanegram/features/ag_show_seconds.h"
#include "arcanegram/ui/ag_hidden_users_settings.h"
#include "lang/lang_keys.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_arcanegram.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::Settings {
namespace {

using namespace ::Settings;
using namespace ::Settings::Builder;

class Main : public Section<Main> {
public:
    Main(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_settings_title();
    }

private:
    void setupContent();
};

void BuildContent(SectionBuilder &builder) {
    Arcanegram::ForwardedHeader::Setup(builder);
    Arcanegram::Time::Setup(builder);
    Arcanegram::HiddenUsers::Setup(builder);
}

const auto kMeta = BuildHelper({
    .id = Main::Id(),
    .parentId = MainId(),
    .title = &tr::ag_settings_title,
    .icon = &st::menuIconArcanegramSettings,
}, [](SectionBuilder &builder) {
    BuildContent(builder);
});

const SectionBuildMethod kArcanegramSection = kMeta.build;

Main::Main(
    QWidget *parent,
    not_null<Window::SessionController*> controller)
: Section(parent, controller) {
    setupContent();
}

void Main::setupContent() {
    const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
    build(content, kArcanegramSection);
    Ui::ResizeFitChild(this, content);
}

} // namespace

::Settings::Type Id() { return Main::Id(); }

} // namespace Arcanegram::Settings
