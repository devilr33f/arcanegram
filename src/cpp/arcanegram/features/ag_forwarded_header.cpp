#include "arcanegram/features/ag_forwarded_header.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "base/flat_set.h"
#include "base/unixtime.h"
#include "core/application.h"
#include "data/data_session.h"
#include "history/history_item.h"
#include "lang/lang_keys.h"
#include "main/main_account.h"
#include "main/main_domain.h"
#include "main/main_session.h"

#include <QtCore/QLocale>

namespace Arcanegram::ForwardedHeader {
namespace {

base::flat_set<FullMsgId> &Tracked() {
    static base::flat_set<FullMsgId> value;
    return value;
}

rpl::lifetime &Lifetime() {
    static rpl::lifetime value;
    return value;
}

void RefreshAll() {
    auto &tracked = Tracked();
    if (tracked.empty()) {
        return;
    }
    auto stale = std::vector<FullMsgId>();
    for (const auto &id : tracked) {
        auto refreshed = false;
        for (const auto &entry : Core::App().domain().accounts()) {
            if (const auto session = entry.account->maybeSession()) {
                if (const auto item = session->data().message(id)) {
                    session->data().requestItemViewRefresh(item);
                    refreshed = true;
                }
            }
        }
        if (!refreshed) {
            stale.push_back(id);
        }
    }
    for (const auto &id : stale) {
        tracked.remove(id);
    }
}

} // namespace

void Append(
        TextWithEntities &phrase,
        not_null<const HistoryItem*> item,
        TimeId originalDate) {
    if (!originalDate) {
        return;
    }
    Tracked().emplace(item->fullId());
    if (!Config::ForwardedHeader::ShowDate.value()) {
        return;
    }
    const auto when = QLocale().toString(
        base::unixtime::parse(originalDate),
        QLocale::ShortFormat);
    phrase.text.append(QStringLiteral(". "));
    phrase.text.append(tr::ag_forwarded_date_label(tr::now));
    phrase.text.append(QStringLiteral(": "));
    phrase.text.append(when);
}

void Setup(::Settings::Builder::SectionBuilder &builder) {
    Arcanegram::Settings::AddBoolRow(
        builder,
        u"arcanegram/forwarded_show_date"_q,
        tr::ag_forwarded_show_date_setting(),
        tr::ag_forwarded_show_date_info(),
        Config::ForwardedHeader::ShowDate);
}

void Init() {
    Config::ForwardedHeader::ShowDate.changes(
    ) | rpl::on_next([](bool) {
        RefreshAll();
    }, Lifetime());
}

} // namespace Arcanegram::ForwardedHeader
