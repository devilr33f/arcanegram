#pragma once

#include "base/basic_types.h"

#include <rpl/producer.h>

class PeerData;

namespace Settings::Builder { class SectionBuilder; }

namespace Arcanegram::Streamer {

void Init();

[[nodiscard]] bool IsActive();
[[nodiscard]] bool ShouldAnonymize(not_null<const PeerData*> peer);

[[nodiscard]] QString GeneratedName(PeerId id);
[[nodiscard]] QString GeneratedShortName(PeerId id);

[[nodiscard]] uint8 ForcedColorIndex();

[[nodiscard]] rpl::producer<bool> Changes();

void Setup(::Settings::Builder::SectionBuilder &builder);

} // namespace Arcanegram::Streamer
