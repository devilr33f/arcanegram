#include "arcanegram/features/streamer/ag_streamer.h"

#include "arcanegram/features/streamer/streamer_names.h"
#include "arcanegram/ag_config.h"
#include "arcanegram/ag_refresh.h"
#include "arcanegram/ui/ag_settings_main.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "data/data_peer.h"
#include "data/data_user.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "settings/settings_common_session.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/vertical_list.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"
#include "window/window_session_controller.h"

#include <QtCore/QRandomGenerator>

namespace Arcanegram::Streamer {
namespace {

quint64 &Seed() {
	static quint64 value = 0;
	return value;
}

rpl::event_stream<bool> &Stream() {
	static rpl::event_stream<bool> value;
	return value;
}

rpl::lifetime &Lifetime() {
	static rpl::lifetime value;
	return value;
}

QString Capitalize(std::string_view sv) {
	auto s = QString::fromUtf8(sv.data(), int(sv.size()));
	if (!s.isEmpty()) {
		s[0] = s[0].toUpper();
	}
	return s;
}

} // namespace

void Init() {
	Seed() = QRandomGenerator::global()->generate64();

	Config::StreamerMode::Enabled.changes(
	) | rpl::start_with_next([](bool active) {
		Stream().fire_copy(active);
		RefreshAllItems();
	}, Lifetime());

	Config::StreamerMode::AnonymizeBots.changes(
	) | rpl::start_with_next([](bool) {
		if (IsActive()) {
			RefreshAllItems();
		}
	}, Lifetime());
}

bool IsActive() {
	return Config::StreamerMode::Enabled.value();
}

bool ShouldAnonymize(not_null<const PeerData*> peer) {
	if (!IsActive()) {
		return false;
	}
	if (peer->isSelf()) {
		return false;
	}
	if (const auto user = peer->asUser()) {
		if (user->isBot() && !Config::StreamerMode::AnonymizeBots.value()) {
			return false;
		}
		return true;
	}
	return peer->isChannel() || peer->isChat();
}

QString GeneratedName(PeerId id) {
	const auto mix = Seed() ^ quint64(id.value);
	const auto a = mix * 0x9E3779B97F4A7C15ull;
	const auto b = (mix >> 17) * 0xBF58476D1CE4E5B9ull;
	const auto adj = kAdjectives[a % kAdjectives.size()];
	const auto sur = kSurnames[b % kSurnames.size()];
	return Capitalize(adj) + QChar(' ') + Capitalize(sur);
}

QString GeneratedShortName(PeerId id) {
	const auto mix = Seed() ^ quint64(id.value);
	const auto a = mix * 0x9E3779B97F4A7C15ull;
	const auto b = (mix >> 17) * 0xBF58476D1CE4E5B9ull;
	return Capitalize(kAdjectives[(a ^ b) % kAdjectives.size()]);
}

uint8 ForcedColorIndex() {
	return 5;
}

rpl::producer<bool> Changes() {
	return Stream().events();
}

namespace {

void BuildPage(::Settings::Builder::SectionBuilder &builder) {
	const auto container = builder.container();
	Ui::AddSubsectionTitle(container, tr::ag_streamer_title());

	Arcanegram::Settings::AddBoolRow(
		builder,
		u"arcanegram/streamer/enabled"_q,
		tr::ag_streamer_enabled(),
		tr::ag_streamer_enabled_about(),
		Config::StreamerMode::Enabled);

	Arcanegram::Settings::AddBoolRow(
		builder,
		u"arcanegram/streamer/anonymize_bots"_q,
		tr::ag_streamer_anonymize_bots(),
		tr::ag_streamer_anonymize_bots_about(),
		Config::StreamerMode::AnonymizeBots);

	Ui::AddSkip(container);
	Ui::AddDivider(container);
	Ui::AddSkip(container);

	container->add(
		object_ptr<Ui::FlatLabel>(
			container,
			tr::ag_streamer_preview(
				lt_name,
				rpl::single(GeneratedName(PeerId(1234567890ULL)))),
			st::boxDividerLabel),
		st::defaultBoxDividerLabelPadding);

	Ui::AddDividerText(container, tr::ag_streamer_subtitle());
}

class Page : public ::Settings::Section<Page> {
public:
	Page(QWidget *parent, not_null<Window::SessionController*> controller);

	[[nodiscard]] rpl::producer<QString> title() override {
		return tr::ag_streamer_title();
	}

private:
	void setupContent();
};

const auto kPageMeta = ::Settings::Builder::BuildHelper({
	.id = Page::Id(),
	.parentId = Arcanegram::Settings::Id(),
	.title = &tr::ag_streamer_title,
	.icon = &st::menuIconStealthLocked,
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
		.title = tr::ag_streamer_title(),
		.targetSection = Page::Id(),
		.icon = { &st::menuIconStealthLocked },
	});
}

} // namespace Arcanegram::Streamer
