#pragma once

#include "base/basic_types.h"

#include <QtCore/QString>

class PeerData;

namespace Arcanegram::PeerIds {

[[nodiscard]] QString FormatBotApi(not_null<PeerData*> peer);
[[nodiscard]] QString FormatMtproto(not_null<PeerData*> peer);

} // namespace Arcanegram::PeerIds
