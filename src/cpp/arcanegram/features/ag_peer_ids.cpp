#include "arcanegram/features/ag_peer_ids.h"

#include "data/data_peer.h"

namespace Arcanegram::PeerIds {
namespace {

constexpr auto kChannelBotApiOffset = qint64(1'000'000'000'000);

} // namespace

QString FormatBotApi(not_null<PeerData*> peer) {
	const auto raw = qint64(peer->id.value & PeerId::kChatTypeMask);
	if (peer->isUser()) {
		return QString::number(raw);
	} else if (peer->isChannel()) {
		return QString::number(-(raw + kChannelBotApiOffset));
	}
	return QString::number(-raw);
}

QString FormatMtproto(not_null<PeerData*> peer) {
	return QString::number(peer->id.value & PeerId::kChatTypeMask);
}

} // namespace Arcanegram::PeerIds
