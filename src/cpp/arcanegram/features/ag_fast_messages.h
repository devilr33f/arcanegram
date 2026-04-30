#pragma once

#include "api/api_common.h"
#include "settings/settings_builder.h"

class PeerData;

namespace Arcanegram::FastMessages {

void Init();
void Setup(::Settings::Builder::SectionBuilder &builder);

[[nodiscard]] bool Send(
    not_null<PeerData*> peer,
    FullReplyTo replyTo,
    int slot);

} // namespace Arcanegram::FastMessages
