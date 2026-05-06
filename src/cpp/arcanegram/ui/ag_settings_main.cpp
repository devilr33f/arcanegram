#include "arcanegram/ui/ag_settings_main.h"

#include "arcanegram/features/ag_chat_wallpaper.h"
#include "arcanegram/features/ag_fast_messages.h"
#include "arcanegram/features/ag_forwarded_header.h"
#include "arcanegram/features/ag_show_seconds.h"
#include "arcanegram/features/ag_sync_feature.h"
#include "arcanegram/features/screenshot/ag_screenshot.h"
#include "arcanegram/ui/ag_hidden_users_settings.h"
#include "lang/lang_keys.h"
#include "settings/sections/settings_main.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_arcanegram.h"
#include "styles/style_menu_icons.h"
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

class Appearance : public Section<Appearance> {
public:
    Appearance(QWidget *parent, not_null<Window::SessionController*> controller);

    [[nodiscard]] rpl::producer<QString> title() override {
        return tr::ag_settings_appearance();
    }

private:
    void setupContent();
};

void BuildAppearance(SectionBuilder &builder) {
    Arcanegram::ForwardedHeader::Setup(builder);
    Arcanegram::Time::Setup(builder);
    Arcanegram::ChatWallpaper::Setup(builder);
}

const auto kAppearanceMeta = BuildHelper({
    .id = Appearance::Id(),
    .parentId = Main::Id(),
    .title = &tr::ag_settings_appearance,
    .icon = &st::menuIconPalette,
}, [](SectionBuilder &builder) {
    BuildAppearance(builder);
});

const SectionBuildMethod kAppearanceSection = kAppearanceMeta.build;

Appearance::Appearance(
    QWidget *parent,
    not_null<Window::SessionController*> controller)
: Section(parent, controller) {
    setupContent();
}

void Appearance::setupContent() {
    const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
    build(content, kAppearanceSection);
    Ui::ResizeFitChild(this, content);
}

void BuildContent(SectionBuilder &builder) {
    builder.addSectionButton({
        .title = tr::ag_settings_appearance(),
        .targetSection = Appearance::Id(),
        .icon = { &st::menuIconPalette },
    });
    Arcanegram::HiddenUsers::Setup(builder);
    Arcanegram::Screenshot::Setup(builder);
    Arcanegram::FastMessages::Setup(builder);
    Arcanegram::CloudSync::Setup(builder);
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
