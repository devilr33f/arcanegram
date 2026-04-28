#include "arcanegram/ui/ag_settings_widgets.h"

#include "arcanegram/ag_config.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "styles/style_settings.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"

namespace Arcanegram::Settings {

void AddBoolRow(
        ::Settings::Builder::SectionBuilder &builder,
        const QString &id,
        rpl::producer<QString> title,
        rpl::producer<QString> info,
        Config::BoolItem &item,
        bool needsRestart) {
    const auto button = builder.addButton({
        .id = id,
        .title = std::move(title),
        .st = &st::settingsButtonNoIcon,
        .toggled = rpl::single(item.value()) | rpl::then(item.changes()),
    });
    if (button) {
        button->toggledValue(
        ) | rpl::filter([&item](bool checked) {
            return checked != item.value();
        }) | rpl::on_next([&item, needsRestart](bool checked) {
            item.setValue(checked);
            if (needsRestart) {
                Ui::Toast::Show(tr::lng_settings_need_restart(tr::now));
            }
        }, button->lifetime());
    }
    if (info) {
        builder.addDividerText(std::move(info));
    }
}

} // namespace Arcanegram::Settings
