#include "arcanegram/features/ag_hidden_users.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ag_refresh.h"
#include "data/data_peer.h"
#include "data/data_peer_id.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/history_item_components.h"
#include "lang/lang_keys.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"
#include "core/application.h"
#include "styles/style_menu_icons.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Arcanegram::HiddenUsers {
namespace {

base::flat_set<PeerId> &Set() {
	static base::flat_set<PeerId> value;
	return value;
}

base::flat_set<UserId> &BotSet() {
	static base::flat_set<UserId> value;
	return value;
}

QStringList &RegexStrings() {
	static QStringList value;
	return value;
}

std::vector<QRegularExpression> &RegexCompiled() {
	static std::vector<QRegularExpression> value;
	return value;
}

rpl::event_stream<PeerId> &Stream() {
	static rpl::event_stream<PeerId> value;
	return value;
}

rpl::event_stream<> &BotStream() {
	static rpl::event_stream<> value;
	return value;
}

rpl::event_stream<> &RegexStream() {
	static rpl::event_stream<> value;
	return value;
}

rpl::lifetime &Lifetime() {
	static rpl::lifetime value;
	return value;
}

QString SerializePeers(const base::flat_set<PeerId> &set) {
	auto parts = QStringList();
	parts.reserve(int(set.size()));
	for (const auto &id : set) {
		parts.append(QString::number(id.value));
	}
	return parts.join(QChar(','));
}

QString SerializeBots(const base::flat_set<UserId> &set) {
	auto parts = QStringList();
	parts.reserve(int(set.size()));
	for (const auto &id : set) {
		parts.append(QString::number(id.bare));
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

void LoadBots() {
	const auto raw = Config::HiddenUsers::Bots.value();
	auto &set = BotSet();
	set.clear();
	if (raw.isEmpty()) {
		return;
	}
	for (const auto &part : raw.split(QChar(','), Qt::SkipEmptyParts)) {
		auto ok = false;
		const auto v = part.toULongLong(&ok);
		if (ok && v) {
			set.emplace(UserId(v));
		}
	}
}

void RebuildRegexCache() {
	auto &compiled = RegexCompiled();
	compiled.clear();
	for (const auto &pattern : RegexStrings()) {
		if (pattern.isEmpty()) {
			continue;
		}
		auto re = QRegularExpression(
			pattern,
			QRegularExpression::CaseInsensitiveOption
				| QRegularExpression::DotMatchesEverythingOption);
		if (re.isValid()) {
			re.optimize();
			compiled.push_back(std::move(re));
		}
	}
}

void LoadRegexes() {
	const auto raw = Config::HiddenUsers::Regexes.value();
	auto &list = RegexStrings();
	list = raw.split(QChar('\n'), Qt::SkipEmptyParts);
	RebuildRegexCache();
}

void SaveUsers() {
	Config::HiddenUsers::Stored.setValue(SerializePeers(Set()));
}

void SaveBots() {
	Config::HiddenUsers::Bots.setValue(SerializeBots(BotSet()));
}

void SaveRegexes() {
	Config::HiddenUsers::Regexes.setValue(RegexStrings().join(QChar('\n')));
}

void RefreshForPeer(PeerId changed) {
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
		const auto originalSender = item->originalSender();
		if (originalSender && originalSender->id == changed) {
			owner.requestItemViewRefresh(item);
			return;
		}
		const auto hiddenInfo = item->originalHiddenSenderInfo();
		if (hiddenInfo && !hiddenInfo->name.isEmpty()) {
			const auto changedPeer = owner.peerLoaded(changed);
			if (changedPeer && changedPeer->name() == hiddenInfo->name) {
				owner.requestItemViewRefresh(item);
				return;
			}
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

UserData *AnyLoadedBot(UserId id) {
	for (const auto &entry : Core::App().domain().accounts()) {
		if (const auto session = entry.account->maybeSession()) {
			const auto peer = session->data().peerLoaded(peerFromUser(id));
			if (peer && peer->isUser()) {
				return peer->asUser();
			}
		}
	}
	return nullptr;
}

} // namespace

void Init() {
	Load();
	LoadBots();
	LoadRegexes();
	Config::HiddenUsers::Stored.changes(
	) | rpl::on_next([](const QString &) {
		Load();
	}, Lifetime());
	Config::HiddenUsers::Bots.changes(
	) | rpl::on_next([](const QString &) {
		LoadBots();
	}, Lifetime());
	Config::HiddenUsers::Regexes.changes(
	) | rpl::on_next([](const QString &) {
		LoadRegexes();
	}, Lifetime());
}

bool IsHidden(PeerId id) {
	return Set().contains(id);
}

bool IsHiddenBot(UserId id) {
	return BotSet().contains(id);
}

bool IsHiddenByRegex(const QString &text) {
	if (RegexCompiled().empty() || text.isEmpty()) {
		return false;
	}
	for (const auto &re : RegexCompiled()) {
		if (re.match(text).hasMatch()) {
			return true;
		}
	}
	return false;
}

bool IsHiddenItem(not_null<const HistoryItem*> item) {
	const auto from = item->from();
	if (from && from->isUser() && IsHidden(from->id)) {
		return true;
	}
	const auto originalSender = item->originalSender();
	if (originalSender && originalSender->isUser() && IsHidden(originalSender->id)) {
		return true;
	}
	const auto hiddenInfo = item->originalHiddenSenderInfo();
	if (hiddenInfo && !hiddenInfo->name.isEmpty()) {
		auto &owner = item->history()->owner();
		for (const auto &id : Set()) {
			const auto peer = owner.peerLoaded(id);
			if (peer && peer->name() == hiddenInfo->name) {
				return true;
			}
		}
	}
	if (const auto bot = item->viaBot()) {
		if (IsHiddenBot(peerToUser(bot->id))) {
			return true;
		}
	}
	if (!RegexCompiled().empty()) {
		if (IsHiddenByRegex(item->originalText().text)) {
			return true;
		}
	}
	return false;
}

void Hide(PeerId id) {
	if (Set().emplace(id).second) {
		SaveUsers();
		RefreshForPeer(id);
		Stream().fire_copy(id);
	}
}

void Unhide(PeerId id) {
	if (Set().remove(id)) {
		SaveUsers();
		RefreshForPeer(id);
		Stream().fire_copy(id);
	}
}

void HideBot(UserId id) {
	if (BotSet().emplace(id).second) {
		SaveBots();
		RefreshAllItems();
		BotStream().fire({});
	}
}

void UnhideBot(UserId id) {
	if (BotSet().remove(id)) {
		SaveBots();
		RefreshAllItems();
		BotStream().fire({});
	}
}

QStringList RegexList() {
	return RegexStrings();
}

void SetRegexList(QStringList patterns) {
	for (auto &p : patterns) {
		p = p.trimmed();
	}
	patterns.removeAll(QString());
	if (RegexStrings() == patterns) {
		return;
	}
	RegexStrings() = std::move(patterns);
	RebuildRegexCache();
	SaveRegexes();
	RefreshAllItems();
	RegexStream().fire({});
}

rpl::producer<PeerId> Changes() {
	return Stream().events();
}

rpl::producer<> BotChanges() {
	return BotStream().events();
}

rpl::producer<> RegexChanges() {
	return RegexStream().events();
}

const base::flat_set<PeerId> &List() {
	return Set();
}

const base::flat_set<UserId> &BotList() {
	return BotSet();
}

QString BotDisplay(UserId id) {
	if (const auto bot = AnyLoadedBot(id)) {
		const auto name = bot->username();
		return name.isEmpty() ? bot->name() : (QChar('@') + name);
	}
	return QString::number(id.bare);
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
		},
		&st::menuIconStealth);
}

void FillBotMenu(AddActionCallback add, not_null<UserData*> bot) {
	const auto id = peerToUser(bot->id);
	const auto hidden = IsHiddenBot(id);
	add(hidden
			? tr::ag_hidden_bots_menu_unhide(tr::now)
			: tr::ag_hidden_bots_menu_hide(tr::now),
		[id, hidden] {
			if (hidden) {
				UnhideBot(id);
			} else {
				HideBot(id);
			}
		},
		&st::menuIconStealth);
}

} // namespace Arcanegram::HiddenUsers
