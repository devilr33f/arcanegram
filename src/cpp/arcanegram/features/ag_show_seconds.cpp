#include "arcanegram/features/ag_show_seconds.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "core/application.h"
#include "lang/lang_keys.h"
#include "window/window_controller.h"

#include <QtCore/QLocale>
#include <QtWidgets/QWidget>

namespace Arcanegram::Time {
namespace {

rpl::lifetime &Lifetime() {
    static rpl::lifetime value;
    return value;
}

QString WithSecondsFormat() {
    auto base = QLocale().timeFormat(QLocale::ShortFormat);
    const auto pos = base.indexOf(u"mm"_q);
    if (pos < 0) {
        return base;
    }
    base.insert(pos + 2, u":ss"_q);
    return base;
}

void RepaintActiveWindow() {
    if (const auto window = Core::App().activeWindow()) {
        if (const auto widget = window->widget()) {
            widget->update();
        }
    }
}

} // namespace

QString FormatShort(QTime time) {
    if (!Config::ShowSeconds::Enabled.value()) {
        return QLocale().toString(time, QLocale::ShortFormat);
    }
    static thread_local auto cached = WithSecondsFormat();
    return QLocale().toString(time, cached);
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/show_seconds"_q,
        tr::ag_show_seconds_setting(),
        tr::ag_show_seconds_info(),
        Config::ShowSeconds::Enabled);
}

void Init() {
    Config::ShowSeconds::Enabled.changes(
    ) | rpl::on_next([](bool) {
        RepaintActiveWindow();
    }, Lifetime());
}

} // namespace Arcanegram::Time
