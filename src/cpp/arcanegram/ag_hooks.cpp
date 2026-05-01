#include "ag_hooks.h"

#include "arcanegram/features/ag_forwarded_header.h"
#include "arcanegram/features/ag_show_seconds.h"
#include "arcanegram/features/ag_hidden_users.h"
#include "arcanegram/features/ag_chat_wallpaper.h"
#include "arcanegram/features/ag_fast_messages.h"
#include "arcanegram/features/streamer/ag_streamer.h"
#include "arcanegram/sync/ag_sync_engine.h"

namespace Arcanegram::Hooks {

void init() {
    Arcanegram::ForwardedHeader::Init();
    Arcanegram::Time::Init();
    Arcanegram::HiddenUsers::Init();
    Arcanegram::ChatWallpaper::Init();
    Arcanegram::Sync::Init();
    Arcanegram::FastMessages::Init();
    Arcanegram::Streamer::Init();
}

} // namespace Arcanegram::Hooks
