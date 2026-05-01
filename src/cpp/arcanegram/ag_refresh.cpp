#include "arcanegram/ag_refresh.h"

#include "core/application.h"
#include "data/data_channel.h"
#include "data/data_chat.h"
#include "data/data_forum.h"
#include "data/data_forum_topic.h"
#include "data/data_peer.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/history_item.h"
#include "history/view/history_view_element.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"

namespace Arcanegram {

void ForEachLoadedHistory(Fn<void(not_null<History*>)> action) {
	for (const auto &entry : Core::App().domain().accounts()) {
		const auto session = entry.account->maybeSession();
		if (!session) {
			continue;
		}
		auto &owner = session->data();
		const auto visit = [&](PeerId id) {
			if (const auto h = owner.historyLoaded(id)) {
				action(h);
			}
		};
		owner.enumerateUsers([&](not_null<UserData*> p) {
			visit(p->id);
		});
		owner.enumerateGroups([&](not_null<PeerData*> p) {
			visit(p->id);
		});
		owner.enumerateBroadcasts([&](not_null<ChannelData*> p) {
			visit(p->id);
		});
	}
}

void ForEachLoadedTopic(Fn<void(not_null<Data::ForumTopic*>)> action) {
	for (const auto &entry : Core::App().domain().accounts()) {
		const auto session = entry.account->maybeSession();
		if (!session) {
			continue;
		}
		auto &owner = session->data();
		owner.enumerateBroadcasts([&](not_null<ChannelData*> p) {
			if (const auto forum = p->forum()) {
				forum->enumerateTopics([&](not_null<Data::ForumTopic*> t) {
					action(t);
				});
			}
		});
		owner.enumerateGroups([&](not_null<PeerData*> p) {
			if (const auto channel = p->asChannel()) {
				if (const auto forum = channel->forum()) {
					forum->enumerateTopics(
						[&](not_null<Data::ForumTopic*> t) {
							action(t);
						});
				}
			}
		});
	}
}

void ForEachLoadedItem(Fn<void(not_null<HistoryItem*>)> action) {
	ForEachLoadedHistory([&](not_null<History*> h) {
		for (const auto &block : h->blocks) {
			for (const auto &element : block->messages) {
				action(element->data());
			}
		}
	});
}

void ForEachLoadedHistoryFor(
		PeerId id,
		Fn<void(not_null<History*>)> action) {
	for (const auto &entry : Core::App().domain().accounts()) {
		const auto session = entry.account->maybeSession();
		if (!session) {
			continue;
		}
		if (const auto h = session->data().historyLoaded(id)) {
			action(h);
		}
	}
}

void ForEachLoadedPeer(Fn<void(not_null<PeerData*>)> action) {
	for (const auto &entry : Core::App().domain().accounts()) {
		const auto session = entry.account->maybeSession();
		if (!session) {
			continue;
		}
		auto &owner = session->data();
		owner.enumerateUsers([&](not_null<UserData*> p) {
			action(p);
		});
		owner.enumerateGroups([&](not_null<PeerData*> p) {
			action(p);
		});
		owner.enumerateBroadcasts([&](not_null<ChannelData*> p) {
			action(p);
		});
	}
}

void RefreshAllItems() {
	ForEachLoadedHistory([](not_null<History*> h) {
		auto &owner = h->owner();
		for (const auto &block : h->blocks) {
			for (const auto &element : block->messages) {
				owner.requestItemViewRefresh(element->data());
			}
		}
	});
}

} // namespace Arcanegram
