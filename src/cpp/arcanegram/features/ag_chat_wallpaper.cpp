#include "arcanegram/features/ag_chat_wallpaper.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "lang/lang_keys.h"

namespace Arcanegram::ChatWallpaper {

bool Disabled() {
    return Config::ChatWallpaper::Disabled.value();
}

rpl::producer<bool> DisabledValue() {
    return rpl::single(
        Config::ChatWallpaper::Disabled.value()
    ) | rpl::then(Config::ChatWallpaper::Disabled.changes());
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/chat_wallpaper_disabled"_q,
        tr::ag_chat_wallpaper_disabled_setting(),
        tr::ag_chat_wallpaper_disabled_info(),
        Config::ChatWallpaper::Disabled);
}

void Init() {
}

} // namespace Arcanegram::ChatWallpaper
