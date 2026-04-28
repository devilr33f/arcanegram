#include "arcanegram/features/ag_forwarded_header.h"

#include "arcanegram/ag_config.h"
#include "arcanegram/ag_refresh.h"
#include "arcanegram/ui/ag_settings_widgets.h"
#include "base/unixtime.h"
#include "lang/lang_keys.h"

#include <QtCore/QLocale>

namespace Arcanegram::ForwardedHeader {
namespace {

rpl::lifetime &Lifetime() {
    static rpl::lifetime value;
    return value;
}

} // namespace

void Append(
        TextWithEntities &phrase,
        not_null<const HistoryItem*> item,
        TimeId originalDate) {
    if (!originalDate || !Config::ForwardedHeader::ShowDate.value()) {
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
        RefreshAllItems();
    }, Lifetime());
}

} // namespace Arcanegram::ForwardedHeader
