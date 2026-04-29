#pragma once

#include "base/basic_types.h"

class History;
class HistoryItem;

namespace Arcanegram {

void ForEachLoadedItem(Fn<void(not_null<HistoryItem*>)> action);

void ForEachLoadedHistoryFor(PeerId id, Fn<void(not_null<History*>)> action);

void RefreshAllItems();

} // namespace Arcanegram
