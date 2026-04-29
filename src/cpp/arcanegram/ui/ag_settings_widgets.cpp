#include "arcanegram/ui/ag_settings_widgets.h"

#include "arcanegram/ag_config.h"
#include "lang/lang_keys.h"
#include "settings/settings_builder.h"
#include "styles/style_layers.h"
#include "styles/style_settings.h"
#include "ui/toast/toast.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/vertical_layout.h"

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

void AddTextRow(
        ::Settings::Builder::SectionBuilder &builder,
        rpl::producer<QString> placeholder,
        Config::Item<QString> &item) {
    const auto container = builder.container();
    const auto field = container->add(
        object_ptr<Ui::InputField>(
            container,
            st::defaultInputField,
            std::move(placeholder),
            item.value()),
        st::settingsButtonNoIcon.padding);
    field->changes(
    ) | rpl::on_next([field, &item](auto) {
        const auto v = field->getLastText().trimmed();
        if (v != item.value()) {
            item.setValue(v);
        }
    }, field->lifetime());
}

void AddStatusRow(
        ::Settings::Builder::SectionBuilder &builder,
        rpl::producer<QString> text) {
    const auto container = builder.container();
    container->add(
        object_ptr<Ui::FlatLabel>(
            container,
            std::move(text),
            st::boxLabel),
        st::settingsButtonNoIcon.padding);
}

} // namespace Arcanegram::Settings
