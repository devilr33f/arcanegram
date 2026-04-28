#include "ag_hooks.h"

#include "arcanegram/features/ag_forwarded_header.h"
#include "arcanegram/features/ag_show_seconds.h"
#include "arcanegram/features/ag_hidden_users.h"

namespace Arcanegram::Hooks {

void init() {
    Arcanegram::ForwardedHeader::Init();
    Arcanegram::Time::Init();
    Arcanegram::HiddenUsers::Init();
}

} // namespace Arcanegram::Hooks
