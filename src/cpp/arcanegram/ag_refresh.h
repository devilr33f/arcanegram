#pragma once

#include "base/basic_types.h"

class HistoryItem;

namespace Arcanegram {

void ForEachLoadedItem(Fn<void(not_null<HistoryItem*>)> action);

void RefreshAllItems();

} // namespace Arcanegram
