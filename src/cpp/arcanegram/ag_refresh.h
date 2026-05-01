#pragma once

#include "base/basic_types.h"

class History;
class HistoryItem;
class PeerData;

namespace Arcanegram {

void ForEachLoadedItem(Fn<void(not_null<HistoryItem*>)> action);

void ForEachLoadedHistoryFor(PeerId id, Fn<void(not_null<History*>)> action);

void ForEachLoadedPeer(Fn<void(not_null<PeerData*>)> action);

void RefreshAllItems();

} // namespace Arcanegram
