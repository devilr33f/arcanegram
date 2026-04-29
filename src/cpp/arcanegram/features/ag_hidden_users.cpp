#include "arcanegram/features/ag_hidden_users.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ag_refresh.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "history/history.h"
#include "history/history_item.h"
#include "lang/lang_keys.h"
#include "main/main_session.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Arcanegram::HiddenUsers {
namespace {

base::flat_set<PeerId> &Set() {
	static base::flat_set<PeerId> value;
	return value;
}

rpl::event_stream<PeerId> &Stream() {
	static rpl::event_stream<PeerId> value;
	return value;
}

rpl::lifetime &Lifetime() {
	static rpl::lifetime value;
	return value;
}

QString Serialize(const base::flat_set<PeerId> &set) {
	auto parts = QStringList();
	parts.reserve(int(set.size()));
	for (const auto &id : set) {
		parts.append(QString::number(id.value));
	}
	return parts.join(QChar(','));
}

void Load() {
	const auto raw = Config::HiddenUsers::Stored.value();
	auto &set = Set();
	set.clear();
	if (raw.isEmpty()) {
		return;
	}
	for (const auto &part : raw.split(QChar(','), Qt::SkipEmptyParts)) {
		auto ok = false;
		const auto v = part.toULongLong(&ok);
		if (ok && v) {
			set.emplace(PeerId(v));
		}
	}
}

void Save() {
	Config::HiddenUsers::Stored.setValue(Serialize(Set()));
}

void RefreshAll(PeerId changed) {
	ForEachLoadedHistoryFor(changed, [](not_null<History*> history) {
		history->updateChatListExistence();
	});
	ForEachLoadedItem([changed](not_null<HistoryItem*> item) {
		auto &owner = item->history()->owner();
		const auto from = item->from();
		if (from && from->id == changed) {
			owner.requestItemViewRefresh(item);
			return;
		}
		const auto reply = item->replyToFullId();
		if (!reply) {
			return;
		}
		const auto target = owner.message(reply);
		if (target && target->from()->id == changed) {
			owner.requestItemViewRefresh(item);
		}
	});
}

} // namespace

void Init() {
	Load();
	Config::HiddenUsers::Stored.changes(
	) | rpl::on_next([](const QString &) {
		Load();
	}, Lifetime());
}

bool IsHidden(PeerId id) {
	return Set().contains(id);
}

bool IsHiddenItem(not_null<const HistoryItem*> item) {
	const auto from = item->from();
	if (!from || !from->isUser()) {
		return false;
	}
	return IsHidden(from->id);
}

void Hide(PeerId id) {
	if (Set().emplace(id).second) {
		Save();
		RefreshAll(id);
		Stream().fire_copy(id);
	}
}

void Unhide(PeerId id) {
	if (Set().remove(id)) {
		Save();
		RefreshAll(id);
		Stream().fire_copy(id);
	}
}

rpl::producer<PeerId> Changes() {
	return Stream().events();
}

const base::flat_set<PeerId> &List() {
	return Set();
}

void FillMenu(AddActionCallback add, PeerId id) {
	const auto hidden = IsHidden(id);
	add(hidden
			? tr::ag_hidden_users_menu_unhide(tr::now)
			: tr::ag_hidden_users_menu_hide(tr::now),
		[=] {
			if (hidden) {
				Unhide(id);
			} else {
				Hide(id);
			}
		});
}

} // namespace Arcanegram::HiddenUsers
