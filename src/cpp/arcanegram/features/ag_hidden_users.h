#pragma once

#include "base/basic_types.h"
#include "base/flat_set.h"
#include <rpl/producer.h>

class HistoryItem;
class PeerData;

namespace Arcanegram::HiddenUsers {

void Init();

[[nodiscard]] bool IsHidden(PeerId id);
[[nodiscard]] bool IsHiddenItem(not_null<const HistoryItem*> item);

void Hide(PeerId id);
void Unhide(PeerId id);

[[nodiscard]] rpl::producer<PeerId> Changes();
[[nodiscard]] const base::flat_set<PeerId> &List();

// generic add-action sink so callers can plug in PopupMenu, PeerMenuCallback, etc.
using AddActionCallback = Fn<void(const QString &label, Fn<void()> action)>;

void FillMenu(AddActionCallback add, PeerId id);

} // namespace Arcanegram::HiddenUsers
