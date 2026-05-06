#pragma once

#include "base/basic_types.h"
#include "base/flat_set.h"
#include <rpl/producer.h>
#include <QtCore/QStringList>

class HistoryItem;
class PeerData;
class UserData;

namespace Arcanegram::HiddenUsers {

void Init();

[[nodiscard]] bool IsHidden(PeerId id);
[[nodiscard]] bool IsHiddenItem(not_null<const HistoryItem*> item);

void Hide(PeerId id);
void Unhide(PeerId id);

[[nodiscard]] rpl::producer<PeerId> Changes();
[[nodiscard]] const base::flat_set<PeerId> &List();

[[nodiscard]] bool IsHiddenBot(UserId id);
void HideBot(UserId id);
void UnhideBot(UserId id);

[[nodiscard]] rpl::producer<> BotChanges();
[[nodiscard]] const base::flat_set<UserId> &BotList();
[[nodiscard]] QString BotDisplay(UserId id);

[[nodiscard]] bool IsHiddenByRegex(const QString &text);
[[nodiscard]] QStringList RegexList();
void SetRegexList(QStringList patterns);
[[nodiscard]] rpl::producer<> RegexChanges();

// generic add-action sink so callers can plug in PopupMenu, PeerMenuCallback, etc.
using AddActionCallback = Fn<void(const QString &label, Fn<void()> action)>;

void FillMenu(AddActionCallback add, PeerId id);
void FillBotMenu(AddActionCallback add, not_null<UserData*> bot);

} // namespace Arcanegram::HiddenUsers
