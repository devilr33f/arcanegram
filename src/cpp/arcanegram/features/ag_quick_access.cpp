#include "arcanegram/features/ag_quick_access.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/vertical_list.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

namespace Arcanegram::QuickAccess {
namespace {

void BuildPage(::Settings::Builder::SectionBuilder &builder) {
	const auto container = builder.container();

	Ui::AddSubsectionTitle(container, tr::ag_quick_access_sidebar());
	Arcanegram::Settings::AddBoolRow(
		builder,
		u"arcanegram/quick_access/sidebar_screenshot"_q,
		tr::ag_quick_access_screenshot_mode(),
		tr::ag_quick_access_sidebar_screenshot_info(),
		Config::QuickAccess::SidebarScreenshot);
	Arcanegram::Settings::AddBoolRow(
		builder,
		u"arcanegram/quick_access/sidebar_streamer_mode"_q,
		tr::ag_quick_access_streamer_mode(),
		tr::ag_quick_access_sidebar_streamer_mode_info(),
		Config::QuickAccess::SidebarStreamerMode);
}

class Page : public ::Settings::Section<Page> {
public:
	Page(QWidget *parent, not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override {
		return tr::ag_quick_access_title();
	}

private:
	void setupContent();
};

const auto kPageMeta = ::Settings::Builder::BuildHelper({
	.id = Page::Id(),
	.parentId = Arcanegram::Settings::Id(),
	.title = &tr::ag_quick_access_title,
	.icon = &st::menuIconShowInFolder,
}, [](::Settings::Builder::SectionBuilder &builder) {
	BuildPage(builder);
});

const ::Settings::SectionBuildMethod kPageSection = kPageMeta.build;

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
		.title = tr::ag_quick_access_title(),
		.targetSection = Page::Id(),
		.icon = { &st::menuIconShowInFolder },
	});
}

} // namespace Arcanegram::QuickAccess
