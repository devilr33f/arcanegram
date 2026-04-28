#include "ag_hooks.h"

#include "arcanegram/features/ag_forwarded_header.h"
#include "arcanegram/features/ag_show_seconds.h"
#include "arcanegram/features/ag_hidden_users.h"
#include "arcanegram/features/ag_chat_wallpaper.h"

namespace Arcanegram::Hooks {

void init() {
    Arcanegram::ForwardedHeader::Init();
    Arcanegram::Time::Init();
    Arcanegram::HiddenUsers::Init();
    Arcanegram::ChatWallpaper::Init();
}

} // namespace Arcanegram::Hooks
