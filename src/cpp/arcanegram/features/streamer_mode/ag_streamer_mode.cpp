#include "arcanegram/features/streamer_mode/ag_streamer_mode.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/features/streamer_mode/platform/ag_streamer_mode_impl.h"
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

namespace Arcanegram::StreamerMode {
namespace {

rpl::lifetime &Lifetime() {
	static rpl::lifetime value;
	return value;
}

void BuildPage(::Settings::Builder::SectionBuilder &builder) {
	const auto container = builder.container();
	Ui::AddSubsectionTitle(container, tr::ag_streamer_mode_title());

	Arcanegram::Settings::AddBoolRow(
		builder,
		u"arcanegram/streamer_mode/enabled"_q,
		tr::ag_streamer_mode_enabled(),
		tr::ag_streamer_mode_about(),
		Config::StreamerMode::Enabled);
}

class Page : public ::Settings::Section<Page> {
public:
	Page(QWidget *parent, not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override {
		return tr::ag_streamer_mode_title();
	}

private:
	void setupContent();
};

const auto kPageMeta = ::Settings::Builder::BuildHelper({
	.id = Page::Id(),
	.parentId = Arcanegram::Settings::Id(),
	.title = &tr::ag_streamer_mode_title,
	.icon = &st::menuIconStealth,
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

void Init() {
	Config::StreamerMode::Enabled.changes(
	) | rpl::on_next([](bool active) {
		if (active) {
			Impl::EnableHook();
		} else {
			Impl::DisableHook();
		}
	}, Lifetime());

	if (Config::StreamerMode::Enabled.value()) {
		Impl::EnableHook();
	}
}

bool IsActive() {
	return Config::StreamerMode::Enabled.value();
}

void ApplyTo(QWidget *widget) {
	if (IsActive()) {
		Impl::HideWidget(widget);
	} else {
		Impl::ShowWidget(widget);
	}
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
	builder.addSectionButton({
		.title = tr::ag_streamer_mode_title(),
		.targetSection = Page::Id(),
		.icon = { &st::menuIconStealth },
	});
}

} // namespace Arcanegram::StreamerMode
